/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/Pool.cc
 *
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extern "C"
{
#include <solv/pool.h>
#include <solv/repo.h>
#include <solv/solvable.h>
}

#include <iostream>
#include <fstream>

#include "zypp/base/Easy.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Exception.h"

#include "zypp/AutoDispose.h"

#include "zypp/sat/detail/PoolImpl.h"
#include "zypp/sat/Pool.h"
#include "zypp/sat/LookupAttr.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace sat
  { /////////////////////////////////////////////////////////////////

    const std::string & Pool::systemRepoAlias()
    { return detail::PoolImpl::systemRepoAlias(); }

    ::_Pool * Pool::get() const
    { return myPool().getPool(); }

    Pool::size_type Pool::capacity() const
    { return myPool()->nsolvables; }

    const SerialNumber & Pool::serial() const
    { return myPool().serial(); }

    void Pool::prepare() const
    { return myPool().prepare(); }

    void Pool::prepareForSolving() const
    { return myPool().prepareForSolving(); }

    Pathname Pool::rootDir() const
    { return myPool().rootDir(); }

    void Pool::rootDir( const Pathname & root_r )
    { return myPool().rootDir( root_r ); }

    bool Pool::reposEmpty() const
    { return ! myPool()->urepos; }

    Pool::size_type Pool::reposSize() const
    { return myPool()->urepos; }

    Pool::RepositoryIterator Pool::reposBegin() const
    {
      if ( myPool()->urepos )
      { // repos[0] == NULL
	for_( it, myPool()->repos+1, myPool()->repos+myPool()->nrepos )
	  if ( *it )
	    return RepositoryIterator( it );
      }
      return reposEnd();
    }

    Pool::RepositoryIterator Pool::reposEnd() const
    { return RepositoryIterator( myPool()->repos+myPool()->nrepos ); }

    bool Pool::solvablesEmpty() const
    {
      // return myPool()->nsolvables;
      // nsolvables is the array size including
      // invalid Solvables.
      for_( it, reposBegin(), reposEnd() )
      {
        if ( ! it->solvablesEmpty() )
          return false;
      }
      return true;
    }

    Pool::size_type Pool::solvablesSize() const
    {
      // Do not return myPool()->nsolvables;
      // nsolvables is the array size including
      // invalid Solvables.
      size_type ret = 0;
      for_( it, reposBegin(), reposEnd() )
      {
        ret += it->solvablesSize();
      }
      return ret;
    }

    Pool::SolvableIterator Pool::solvablesBegin() const
    { return SolvableIterator( myPool().getFirstId() ); }

    Pool::SolvableIterator Pool::solvablesEnd() const
    { return SolvableIterator(); }

    Repository Pool::reposInsert( const std::string & alias_r )
    {
      Repository ret( reposFind( alias_r ) );
      if ( ret )
        return ret;

      ret = Repository( myPool()._createRepo( alias_r ) );
      if ( ret.isSystemRepo() )
      {
        // autoprovide (dummy) RepoInfo
        RepoInfo info;
        info.setAlias( alias_r );
        info.setName( alias_r );
        info.setAutorefresh( true );
        info.setEnabled( true );
        ret.setInfo( info );
      }
      return ret;
    }

    Repository Pool::reposFind( const std::string & alias_r ) const
    {
      for_( it, reposBegin(), reposEnd() )
      {
        if ( alias_r == it->alias() )
          return *it;
      }
      return Repository();
    }

    Repository Pool::findSystemRepo() const
    {
      return Repository( myPool().systemRepo() );
    }

    Repository Pool::systemRepo()
    {
      if ( myPool().systemRepo() )
        return Repository( myPool().systemRepo() );
      return reposInsert( systemRepoAlias() );
    }

    Repository Pool::addRepoSolv( const Pathname & file_r, const std::string & alias_r )
    {
      // Using a temporay repo! (The additional parenthesis are required.)
      AutoDispose<Repository> tmprepo( (Repository::EraseFromPool()) );
      *tmprepo = reposInsert( alias_r );
      tmprepo->addSolv( file_r );

      // no exceptions so we keep it:
      tmprepo.resetDispose();
      return tmprepo;
    }

    Repository Pool::addRepoSolv( const Pathname & file_r )
    { return addRepoSolv( file_r, file_r.basename() ); }

    Repository Pool::addRepoSolv( const Pathname & file_r, const RepoInfo & info_r )
    {
      Repository ret( addRepoSolv( file_r, info_r.alias() ) );
      ret.setInfo( info_r );
      return ret;
    }

    /////////////////////////////////////////////////////////////////

    Repository Pool::addRepoHelix( const Pathname & file_r, const std::string & alias_r )
    {
      // Using a temporay repo! (The additional parenthesis are required.)
      AutoDispose<Repository> tmprepo( (Repository::EraseFromPool()) );
      *tmprepo = reposInsert( alias_r );
      tmprepo->addHelix( file_r );

      // no exceptions so we keep it:
      tmprepo.resetDispose();
      return tmprepo;
    }

    Repository Pool::addRepoHelix( const Pathname & file_r )
    { return addRepoHelix( file_r, file_r.basename() ); }

    Repository Pool::addRepoHelix( const Pathname & file_r, const RepoInfo & info_r )
    {
      Repository ret( addRepoHelix( file_r, info_r.alias() ) );
      ret.setInfo( info_r );
      return ret;
    }

   /////////////////////////////////////////////////////////////////

    void Pool::setTextLocale( const Locale & locale_r )
    { myPool().setTextLocale( locale_r ); }

    void Pool::setRequestedLocales( const LocaleSet & locales_r )
    { myPool().setRequestedLocales( locales_r ); }

    bool Pool::addRequestedLocale( const Locale & locale_r )
    { return myPool().addRequestedLocale( locale_r ); }

    bool Pool::eraseRequestedLocale( const Locale & locale_r )
    { return myPool().eraseRequestedLocale( locale_r ); }

    const LocaleSet & Pool::getRequestedLocales() const
    { return myPool().getRequestedLocales(); }

    bool Pool::isRequestedLocale( const Locale & locale_r ) const
    { return myPool().isRequestedLocale( locale_r ); }

    const LocaleSet & Pool::getAvailableLocales() const
    {  return myPool().getAvailableLocales(); }

    bool Pool::isAvailableLocale( const Locale & locale_r ) const
    { return myPool().isAvailableLocale( locale_r ); }

    bool Pool::multiversionEmpty() const			{ return myPool().multiversionList().empty(); }
    size_t Pool::multiversionSize() const			{ return myPool().multiversionList().size(); }
    Pool::MultiversionIterator Pool::multiversionBegin() const	{ return myPool().multiversionList().begin(); }
    Pool::MultiversionIterator Pool::multiversionEnd() const	{ return myPool().multiversionList().end(); }
    bool Pool::isMultiversion( IdString ident_r ) const		{ return myPool().isMultiversion( ident_r ); }

    Queue Pool::autoInstalled() const				{ return myPool().autoInstalled(); }
    void Pool::setAutoInstalled( const Queue & autoInstalled_r ){ myPool().setAutoInstalled( autoInstalled_r ); }

   /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const Pool & obj )
    {
      return str << "sat::pool(" << obj.serial() << ")["
          << obj.capacity() << "]{"
          << obj.reposSize() << "repos|"
	  << obj.solvablesSize() << "slov}";
    }

    /////////////////////////////////////////////////////////////////
    #undef ZYPP_BASE_LOGGER_LOGGROUP
    #define ZYPP_BASE_LOGGER_LOGGROUP "solvidx"

    void updateSolvFileIndex( const Pathname & solvfile_r )
    {
      AutoDispose<FILE*> solv( ::fopen( solvfile_r.c_str(), "re" ), ::fclose );
      if ( solv == NULL )
      {
	solv.resetDispose();
	ERR << "Can't open solv-file: " << solv << endl;
	return;
      }

      std::string solvidxfile( solvfile_r.extend(".idx").asString() );
      if ( ::unlink( solvidxfile.c_str() ) == -1 && errno != ENOENT )
      {
	ERR << "Can't unlink solv-idx: " << Errno() << endl;
	return;
      }
      {
	int fd = ::open( solvidxfile.c_str(), O_CREAT|O_EXCL|O_WRONLY|O_TRUNC, 0644 );
	if ( fd == -1 )
	{
	  ERR << "Can't create solv-idx: " << Errno() << endl;
	  return;
	}
	::close( fd );
      }
      std::ofstream idx( solvidxfile.c_str() );


      ::_Pool * _pool = ::pool_create();
      ::_Repo * _repo = ::repo_create( _pool, "" );
      if ( ::repo_add_solv( _repo, solv, 0 ) == 0 )
      {
	int _id = 0;
	::_Solvable * _solv = nullptr;
	FOR_REPO_SOLVABLES( _repo, _id, _solv )
	{
	  if ( _solv )
	  {
#define SEP '\t'
#define	idstr(V) pool_id2str( _pool, _solv->V )
	    if ( _solv->arch == ARCH_SRC || _solv->arch == ARCH_NOSRC )
	      idx << "srcpackage:" << idstr(name) << SEP << idstr(evr) << SEP << "noarch" << endl;
	    else
	      idx << idstr(name) << SEP << idstr(evr) << SEP << idstr(arch) << endl;
	  }
	}
      }
      else
      {
	ERR << "Can't read solv-file: " << ::pool_errstr( _pool ) << endl;
      }
      ::repo_free( _repo, 0 );
      ::pool_free( _pool );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace sat
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
