/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/target/TargetImpl.commitFindFileConflicts.cc
 */
extern "C"
{
#include <solv/pool.h>
#include <solv/repo.h>
#include <solv/solvable.h>
#include <solv/poolarch.h>
#include <solv/repo_solv.h>
#include <solv/repo_rpmdb.h>
#include <solv/pool_fileconflicts.h>
}
#include <iostream>
#include <unordered_set>
#include <string>

#include "zypp/base/LogTools.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Exception.h"
#include "zypp/base/UserRequestException.h"

#include "zypp/sat/Queue.h"
#include "zypp/sat/Pool.h"

#include "zypp/target/TargetImpl.h"
#include "zypp/target/CommitPackageCache.h"

#include "zypp/ZYppCallbacks.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace target
  {
    ///////////////////////////////////////////////////////////////////
    namespace
    {
      /** libsolv::pool_findfileconflicts callback providing package header. */
      struct FileConflictsCB
      {
	FileConflictsCB( ::_Pool * pool_r, ProgressData & progress_r )
	: _progress( progress_r )
	, _state( ::rpm_state_create( pool_r, ::pool_get_rootdir(pool_r) ), ::rpm_state_free )
	{}

	void * operator()( ::_Pool * pool_r, sat::detail::IdType id_r )
	{
	  void * ret = lookup( id_r );

	  if ( _visited.find( id_r ) == _visited.end() )
	  {
	    //DBG << "FCCB: " << sat::Solvable( id_r ) << " " << ret << endl;
	    _visited.insert( id_r );
	    if ( ! ret )
	      _noFilelist.insert( id_r );
	    // report on 1st visit only (though there may be up to 3 visits)
	    _progress.incr();
	  }
	  else
	  {
	    _progress.tick();
	  }
	  return ret;
	}

	unsigned noFilelist() const
	{ return _noFilelist.size(); }

	static void * invoke( ::_Pool * pool_r, sat::detail::IdType id_r, void * cbdata_r )
	{ return (*reinterpret_cast<FileConflictsCB*>(cbdata_r))( pool_r, id_r ); }

      private:
	void * lookup( sat::detail::IdType id_r )
	{
	  sat::Solvable solv( id_r );
	  if ( solv.isSystem() )
	  {
	    Solvable * s = solv.get();
	    if ( ! s->repo->rpmdbid )
	      return nullptr;
	    sat::detail::IdType rpmdbid = s->repo->rpmdbid[id_r - s->repo->start];
	    if ( ! rpmdbid )
	      return nullptr;
	    return ::rpm_byrpmdbid( _state, rpmdbid );
	  }
	  else
	  {
	    Package::Ptr pkg( make<Package>( solv ) );
	    if ( ! pkg )
	      return nullptr;
	    Pathname localfile( pkg->cachedLocation() );
	    if ( localfile.empty() )
	      return nullptr;
	    AutoDispose<FILE*> fp( ::fopen( localfile.c_str(), "re" ), ::fclose );
	    return ::rpm_byfp( _state, fp, localfile.c_str() );
	  }
	}

      private:
	ProgressData & _progress;
	AutoDispose<void*> _state;
	std::unordered_set<sat::detail::IdType> _visited;
	std::unordered_set<sat::detail::IdType> _noFilelist;
      };

    } // namespace
    ///////////////////////////////////////////////////////////////////

    void TargetImpl::commitFindFileConflicts( const ZYppCommitPolicy & policy_r, ZYppCommitResult & result_r )
    try {
      MIL << "Checking for file conflicts..." << endl;
      sat::Queue todo;
      sat::Queue conflicts;
      int cutoff = result_r.transaction().installedResult( todo );

      callback::SendReport<FindFileConflictstReport> report;
      ProgressData progress( todo.size() );

      if ( ! report->start( progress ) )
	ZYPP_THROW( AbortRequestException() );

      FileConflictsCB cb( sat::Pool::instance().get(), progress );
      // lambda receives progress trigger and translates into report
      auto sendProgress = [&]( const ProgressData & progress_r ) {
	if ( ! report->progress( progress_r, cb.noFilelist() ) )
	  ZYPP_THROW( AbortRequestException() );
	return true;
      };
      progress.sendTo( sendProgress );

      unsigned count = ::pool_findfileconflicts( sat::Pool::instance().get(),
						 todo,
						 cutoff,
						 conflicts,
						 FINDFILECONFLICTS_USE_SOLVABLEFILELIST | FINDFILECONFLICTS_CHECK_DIRALIASING | FINDFILECONFLICTS_USE_ROOTDIR,
						 &FileConflictsCB::invoke,
						 &cb );

      progress.toMax();
      progress.noSend();

      (count?WAR:MIL) << "Found " << count << " file conflicts." << cutoff << ' ' << todo.size() << endl;
      if ( ! report->result( progress, cb.noFilelist(), count ) )
	ZYPP_THROW( AbortRequestException() );
    }
    catch ( const AbortRequestException & e )
    {
      TargetAbortedException excpt( N_("Installation has been aborted as directed.") );
      excpt.remember( e );
      ZYPP_THROW( excpt );
    }


  } // namespace target
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
