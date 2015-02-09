/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/RpmPostTransCollector.cc
 */
#include <iostream>
#include <fstream>
#include "zypp/base/LogTools.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/target/RpmPostTransCollector.h"

#include "zypp/TmpPath.h"
#include "zypp/PathInfo.h"
#include "zypp/HistoryLog.h"
#include "zypp/ZYppCallbacks.h"
#include "zypp/ExternalProgram.h"
#include "zypp/target/rpm/RpmHeader.h"


using std::endl;
#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::posttrans"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace target
  {

    ///////////////////////////////////////////////////////////////////
    /// \class RpmPostTransCollector::Impl
    /// \brief RpmPostTransCollector implementation.
    ///////////////////////////////////////////////////////////////////
    class RpmPostTransCollector::Impl : private base::NonCopyable
    {
      friend std::ostream & operator<<( std::ostream & str, const Impl & obj );
      friend std::ostream & dumpOn( std::ostream & str, const Impl & obj );
      public:
	Impl( const Pathname & root_r )
	: _root( root_r )
	{}

	~Impl()
	{ if ( !_scripts.empty() ) discardScripts(); }

	/** Extract and remember a packages %posttrans script for later execution. */
	bool collectScriptFromPackage( ManagedFile rpmPackage_r )
	{
	  rpm::RpmHeader::constPtr pkg( rpm::RpmHeader::readPackage( rpmPackage_r, rpm::RpmHeader::NOVERIFY ) );

	  std::string prog( pkg->tag_posttransprog() );
	  if ( prog.empty() || prog == "<lua>" )	// by now leave lua to rpm
	    return false;

	  filesystem::TmpFile script( tmpDir(), rpmPackage_r->basename() );
	  filesystem::addmod( script.path(), 0500 );
	  script.autoCleanup( false );	// no autodelete; within a tmpdir
	  {
	    std::ofstream out( script.path().c_str() );
	    out << "#! " << pkg->tag_posttransprog() << endl
	        << pkg->tag_posttrans() << endl;
	  }
	  _scripts.push_back( script.path().basename() );
	  MIL << "COLLECT posttrans: " << PathInfo( script.path() ) << endl;
	  //DBG << "PROG:  " << pkg->tag_posttransprog() << endl;
	  //DBG << "SCRPT: " << pkg->tag_posttrans() << endl;
	  return true;
	}

	/** Execute te remembered scripts. */
	void executeScripts()
	{
	  if ( _scripts.empty() )
	    return;

	  HistoryLog historylog;

	  Pathname noRootScriptDir( ZConfig::instance().update_scriptsPath() / tmpDir().basename() );

	  for ( auto && script : _scripts )
	  {
	    MIL << "EXECUTE posttrans: " << script << endl;
            ExternalProgram prog( (noRootScriptDir/script).asString(), ExternalProgram::Stderr_To_Stdout, false, -1, true, _root );

	    str::Str collect;
	    for( std::string line = prog.receiveLine(); ! line.empty(); line = prog.receiveLine() )
	    {
	      DBG << line;
	      collect << "    " << line;
	    }
	    int ret = prog.close();
	    const std::string & scriptmsg( collect );

	    if ( ret != 0 || ! scriptmsg.empty() )
	    {
	      const std::string & pkgident( script.substr( 0, script.size()-6 ) );	// strip tmp file suffix

	      if ( ! scriptmsg.empty() )
	      {
		str::Str msg;
		msg << "Output of " << pkgident << " %posttrans script:\n" << scriptmsg;
		historylog.comment( msg, true /*timestamp*/);
		JobReport::info( msg );
	      }

	      if ( ret != 0 )
	      {
		str::Str msg;
		msg << pkgident << " %posttrans script failed (returned " << ret << ")";
		WAR << msg << endl;
		historylog.comment( msg, true /*timestamp*/);
		JobReport::warning( msg );
	      }
	    }
	  }
	  _scripts.clear();
	}

	/** Discard all remembered scrips. */
	void discardScripts()
	{
	  if ( _scripts.empty() )
	    return;

	  HistoryLog historylog;

	  str::Str msg;
	  msg << "%posttrans scripts skipped while aborting:\n";
	  for ( auto && script : _scripts )
	  {
	    const std::string & pkgident( script.substr( 0, script.size()-6 ) );	// strip tmp file suffix
	    WAR << "UNEXECUTED posttrans: " << script << endl;
	    msg << "    " << pkgident << "\n";
	  }

	  historylog.comment( msg, true /*timestamp*/);
	  JobReport::warning( msg );

	  _scripts.clear();
	}

      private:
	/** Lazy create tmpdir on demand. */
	Pathname tmpDir()
	{
	  if ( !_ptrTmpdir ) _ptrTmpdir.reset( new filesystem::TmpDir( _root / ZConfig::instance().update_scriptsPath(), "posttrans" ) );
	  DBG << _ptrTmpdir->path() << endl;
	  return _ptrTmpdir->path();
	}

      private:
	Pathname _root;
	std::list<std::string> _scripts;
	boost::scoped_ptr<filesystem::TmpDir> _ptrTmpdir;
    };

    /** \relates RpmPostTransCollector::Impl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const RpmPostTransCollector::Impl & obj )
    { return str << "RpmPostTransCollector::Impl"; }

    /** \relates RpmPostTransCollector::Impl Verbose stream output */
    inline std::ostream & dumpOn( std::ostream & str, const RpmPostTransCollector::Impl & obj )
    { return str << obj; }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : RpmPostTransCollector
    //
    ///////////////////////////////////////////////////////////////////

    RpmPostTransCollector::RpmPostTransCollector( const Pathname & root_r )
      : _pimpl( new Impl( root_r ) )
    {}

    RpmPostTransCollector::~RpmPostTransCollector()
    {}

    bool RpmPostTransCollector::collectScriptFromPackage( ManagedFile rpmPackage_r )
    { return _pimpl->collectScriptFromPackage( rpmPackage_r ); }

    void RpmPostTransCollector::executeScripts()
    { return _pimpl->executeScripts(); }

    void RpmPostTransCollector::discardScripts()
    { return _pimpl->discardScripts(); }

    std::ostream & operator<<( std::ostream & str, const RpmPostTransCollector & obj )
    { return str << *obj._pimpl; }

    std::ostream & dumpOn( std::ostream & str, const RpmPostTransCollector & obj )
    { return dumpOn( str, *obj._pimpl ); }

  } // namespace target
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
