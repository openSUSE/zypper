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
#include <zypp/base/LogTools.h>
#include <zypp/base/NonCopyable.h>
#include <zypp/base/Gettext.h>
#include <zypp/target/RpmPostTransCollector.h>

#include <zypp/TmpPath.h>
#include <zypp/PathInfo.h>
#include <zypp/HistoryLog.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/ExternalProgram.h>
#include <zypp/target/rpm/RpmHeader.h>
#include <zypp/target/rpm/librpmDb.h>
#include <zypp/ZConfig.h>
#include <zypp/ZYppCallbacks.h>

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
          if ( ! pkg )
          {
            WAR << "Unexpectedly this is no package: " << rpmPackage_r << endl;
            return false;
          }

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
          _scripts.push_back( std::make_pair( script.path().basename(), pkg->tag_name() ) );
          MIL << "COLLECT posttrans: '" << PathInfo( script.path() ) << "' for package: '" << pkg->tag_name() << "'" << endl;
          //DBG << "PROG:  " << pkg->tag_posttransprog() << endl;
          //DBG << "SCRPT: " << pkg->tag_posttrans() << endl;
          return true;
        }

        /** Execute the remembered scripts. */
        bool executeScripts()
        {
          if ( _scripts.empty() )
            return true;

          HistoryLog historylog;

          Pathname noRootScriptDir( ZConfig::instance().update_scriptsPath() / tmpDir().basename() );

          ProgressData scriptProgress( static_cast<ProgressData::value_type>(_scripts.size()) );
          callback::SendReport<ProgressReport> report;
          scriptProgress.sendTo( ProgressReportAdaptor( ProgressData::ReceiverFnc(), report ) );

          bool firstScript = true;
          while ( ! _scripts.empty() )
          {
            const auto &scriptPair = _scripts.front();
            const std::string & script = scriptPair.first;
            const std::string & pkgident( script.substr( 0, script.size()-6 ) );	// strip tmp file suffix

            scriptProgress.name( str::Format(_("Executing %%posttrans script '%1%'")) % pkgident );

            bool canContinue = true;
            if (firstScript)  {
              firstScript = false;
              canContinue = scriptProgress.toMin();
            } else {
              canContinue = scriptProgress.incr();
            }

            if (!canContinue) {
              str::Str msg;
              msg << "Execution of %posttrans scripts cancelled";
              WAR << msg << endl;
              historylog.comment( msg, true /*timestamp*/);
              JobReport::warning( msg );
              return false;
            }

            int npkgs = 0;
            rpm::librpmDb::db_const_iterator it;
            for ( it.findByName( scriptPair.second ); *it; ++it )
              npkgs++;

            MIL << "EXECUTE posttrans: " << script << " with argument: " << npkgs << endl;
            ExternalProgram::Arguments cmd {
              "/bin/sh",
              (noRootScriptDir/script).asString(),
              str::numstring( npkgs )
            };
            ExternalProgram prog( cmd, ExternalProgram::Stderr_To_Stdout, false, -1, true, _root );

            str::Str collect;
            for( std::string line = prog.receiveLine(); ! line.empty(); line = prog.receiveLine() )
            {
              DBG << line;
              collect << "    " << line;
            }

            //script was executed, remove it from the list
            _scripts.pop_front();

            int ret = prog.close();
            const std::string & scriptmsg( collect );

            if ( ret != 0 || ! scriptmsg.empty() )
            {
              if ( ! scriptmsg.empty() )
              {
                str::Str msg;
                msg << "Output of " << pkgident << " %posttrans script:\n" << scriptmsg;
                historylog.comment( msg, true /*timestamp*/);
                JobReport::UserData userData( "cmdout", "%posttrans" );
                JobReport::info( msg, userData );
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

          //show a final message
          scriptProgress.name( _("Executing %posttrans scripts") );
          scriptProgress.toMax();
          _scripts.clear();
          return true;
        }

        /** Discard all remembered scrips. */
        void discardScripts()
        {
          if ( _scripts.empty() )
            return;

          HistoryLog historylog;

          str::Str msg;
          msg << "%posttrans scripts skipped while aborting:\n";
          for ( const auto & script : _scripts )
          {
            const std::string & pkgident( script.first.substr( 0, script.first.size()-6 ) );	// strip tmp file suffix
            WAR << "UNEXECUTED posttrans: " << script.first << endl;
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
        std::list< std::pair< std::string, std::string > > _scripts;
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

    bool RpmPostTransCollector::executeScripts()
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
