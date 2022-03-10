/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/TargetImpl.cc
 *
*/
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <set>

#include <sys/types.h>
#include <dirent.h>

#include <zypp/base/LogTools.h>
#include <zypp/base/Exception.h>
#include <zypp/base/Iterator.h>
#include <zypp/base/Gettext.h>
#include <zypp/base/IOStream.h>
#include <zypp/base/Functional.h>
#include <zypp-core/base/UserRequestException>
#include <zypp/base/Json.h>

#include <zypp/ZConfig.h>
#include <zypp/ZYppFactory.h>
#include <zypp/PathInfo.h>

#include <zypp/PoolItem.h>
#include <zypp/ResObjects.h>
#include <zypp/Url.h>
#include <zypp/TmpPath.h>
#include <zypp/RepoStatus.h>
#include <zypp/ExternalProgram.h>
#include <zypp/Repository.h>
#include <zypp/ShutdownLock_p.h>

#include <zypp/ResFilters.h>
#include <zypp/HistoryLog.h>
#include <zypp/target/TargetImpl.h>
#include <zypp/target/TargetCallbackReceiver.h>
#include <zypp/target/rpm/librpmDb.h>
#include <zypp/target/CommitPackageCache.h>
#include <zypp/target/RpmPostTransCollector.h>

#include <zypp/parser/ProductFileReader.h>
#include <zypp/repo/SrcPackageProvider.h>

#include <zypp/sat/Pool.h>
#include <zypp/sat/detail/PoolImpl.h>
#include <zypp/sat/SolvableSpec.h>
#include <zypp/sat/Transaction.h>

#include <zypp-core/base/String.h>
#include <zypp-core/base/StringV.h>
#include <zypp-core/zyppng/base/EventLoop>
#include <zypp-core/zyppng/io/AsyncDataSource>
#include <zypp-core/zyppng/io/Process>
#include <zypp-core/base/IOTools.h>
#include <zypp-core/zyppng/rpc/rpc.h>
#include <zypp-core/zyppng/base/private/linuxhelpers_p.h>
#include <zypp-core/zyppng/base/EventDispatcher>
#include <zypp-proto/commit.pb.h>
#include <zypp-proto/envelope.pb.h>
#include <zypp-core/zyppng/rpc/zerocopystreams.h>

#include <zypp/target/rpm/RpmException.h>

#include <zypp/PluginExecutor.h>

// include the error codes from zypp-rpm
#include "tools/zypp-rpm/errorcodes.h"
#include <rpm/rpmlog.h>

#include <optional>

using std::endl;

///////////////////////////////////////////////////////////////////
extern "C"
{
#include <solv/repo_rpmdb.h>
#include <solv/chksum.h>
}
namespace zypp
{
  namespace target
  {
    inline std::string rpmDbStateHash( const Pathname & root_r )
    {
      std::string ret;
      AutoDispose<void*> state { ::rpm_state_create( sat::Pool::instance().get(), root_r.c_str() ), ::rpm_state_free };
      AutoDispose<Chksum*> chk { ::solv_chksum_create( REPOKEY_TYPE_SHA1 ), []( Chksum *chk ) -> void {
        ::solv_chksum_free( chk, nullptr );
      } };
      if ( ::rpm_hash_database_state( state, chk ) == 0 )
      {
        int md5l;
        const unsigned char * md5 = ::solv_chksum_get( chk, &md5l );
        ret = ::pool_bin2hex( sat::Pool::instance().get(), md5, md5l );
      }
      else
        WAR << "rpm_hash_database_state failed" << endl;
      return ret;
    }

    inline RepoStatus rpmDbRepoStatus( const Pathname & root_r )
    { return RepoStatus( rpmDbStateHash( root_r ), Date() ); }

  } // namespace target
} // namespace
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace zypp
{
  /////////////////////////////////////////////////////////////////
  namespace
  {
    // HACK for bnc#906096: let pool re-evaluate multiversion spec
    // if target root changes. ZConfig returns data sensitive to
    // current target root.
    inline void sigMultiversionSpecChanged()
    {
      sat::detail::PoolMember::myPool().multiversionSpecChanged();
    }
  } //namespace
  /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace json
  {
    // Lazy via template specialisation / should switch to overloading

    template<>
    inline std::string toJSON( const ZYppCommitResult::TransactionStepList & steps_r )
    {
      using sat::Transaction;
      json::Array ret;

      for ( const Transaction::Step & step : steps_r )
        // ignore implicit deletes due to obsoletes and non-package actions
        if ( step.stepType() != Transaction::TRANSACTION_IGNORE )
          ret.add( step );

      return ret.asJSON();
    }

    /** See \ref commitbegin on page \ref plugin-commit for the specs. */
    template<>
    inline std::string toJSON( const sat::Transaction::Step & step_r )
    {
      static const std::string strType( "type" );
      static const std::string strStage( "stage" );
      static const std::string strSolvable( "solvable" );

      static const std::string strTypeDel( "-" );
      static const std::string strTypeIns( "+" );
      static const std::string strTypeMul( "M" );

      static const std::string strStageDone( "ok" );
      static const std::string strStageFailed( "err" );

      static const std::string strSolvableN( "n" );
      static const std::string strSolvableE( "e" );
      static const std::string strSolvableV( "v" );
      static const std::string strSolvableR( "r" );
      static const std::string strSolvableA( "a" );

      using sat::Transaction;
      json::Object ret;

      switch ( step_r.stepType() )
      {
        case Transaction::TRANSACTION_IGNORE:	/*empty*/ break;
        case Transaction::TRANSACTION_ERASE:	ret.add( strType, strTypeDel ); break;
        case Transaction::TRANSACTION_INSTALL:	ret.add( strType, strTypeIns ); break;
        case Transaction::TRANSACTION_MULTIINSTALL: ret.add( strType, strTypeMul ); break;
      }

      switch ( step_r.stepStage() )
      {
        case Transaction::STEP_TODO:		/*empty*/ break;
        case Transaction::STEP_DONE:		ret.add( strStage, strStageDone ); break;
        case Transaction::STEP_ERROR:		ret.add( strStage, strStageFailed ); break;
      }

      {
        IdString ident;
        Edition ed;
        Arch arch;
        if ( sat::Solvable solv = step_r.satSolvable() )
        {
          ident	= solv.ident();
          ed	= solv.edition();
          arch	= solv.arch();
        }
        else
        {
          // deleted package; post mortem data stored in Transaction::Step
          ident	= step_r.ident();
          ed	= step_r.edition();
          arch	= step_r.arch();
        }

        json::Object s {
          { strSolvableN, ident.asString() },
          { strSolvableV, ed.version() },
          { strSolvableR, ed.release() },
          { strSolvableA, arch.asString() }
        };
        if ( Edition::epoch_t epoch = ed.epoch() )
          s.add( strSolvableE, epoch );

        ret.add( strSolvable, s );
      }

      return ret.asJSON();
    }
  } // namespace json
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace target
  {
    ///////////////////////////////////////////////////////////////////
    namespace
    {
      /// \brief Try to provide /proc fs if not present.
      /// bsc#1181328: Some systemd tools require /proc to be mounted
      class AssertProcMounted
      {
        NON_COPYABLE(AssertProcMounted);
        NON_MOVABLE(AssertProcMounted);
      public:

        AssertProcMounted( Pathname root_r )
        {
          root_r /= "/proc";
          if ( ! PathInfo(root_r/"self").isDir() ) {
            MIL << "Try to make sure proc is mounted at" << _mountpoint << endl;
            if ( filesystem::assert_dir(root_r) == 0
              && execute({ "mount", "-t", "proc", "proc", root_r.asString() }) == 0 ) {
              _mountpoint = std::move(root_r);	// so we'll later unmount it
            }
            else {
              WAR << "Mounting proc at " << _mountpoint << " failed" << endl;
            }
          }
        }

        ~AssertProcMounted( )
        {
          if ( ! _mountpoint.empty() ) {
            // we mounted it so we unmount...
            MIL << "We mounted " << _mountpoint << " so we unmount it" << endl;
            execute({ "umount", "-l", _mountpoint.asString() });
          }
        }

      private:
        int execute( ExternalProgram::Arguments && cmd_r ) const
        {
          ExternalProgram prog( cmd_r, ExternalProgram::Stderr_To_Stdout );
          for( std::string line = prog.receiveLine(); ! line.empty(); line = prog.receiveLine() )
          { DBG << line; }
          return prog.close();
        }

      private:
        Pathname _mountpoint;
      };
    } // namespace
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace
    {
      SolvIdentFile::Data getUserInstalledFromHistory( const Pathname & historyFile_r )
      {
        SolvIdentFile::Data onSystemByUserList;
        // go and parse it: 'who' must constain an '@', then it was installed by user request.
        // 2009-09-29 07:25:19|install|lirc-remotes|0.8.5-3.2|x86_64|root@opensuse|InstallationImage|a204211eb0...
        std::ifstream infile( historyFile_r.c_str() );
        for( iostr::EachLine in( infile ); in; in.next() )
        {
          const char * ch( (*in).c_str() );
          // start with year
          if ( *ch < '1' || '9' < *ch )
            continue;
          const char * sep1 = ::strchr( ch, '|' );	// | after date
          if ( !sep1 )
            continue;
          ++sep1;
          // if logs an install or delete
          bool installs = true;
          if ( ::strncmp( sep1, "install|", 8 ) )
          {
            if ( ::strncmp( sep1, "remove |", 8 ) )
              continue; // no install and no remove
              else
                installs = false; // remove
          }
          sep1 += 8;					// | after what
          // get the package name
          const char * sep2 = ::strchr( sep1, '|' );	// | after name
          if ( !sep2 || sep1 == sep2 )
            continue;
          (*in)[sep2-ch] = '\0';
          IdString pkg( sep1 );
          // we're done, if a delete
          if ( !installs )
          {
            onSystemByUserList.erase( pkg );
            continue;
          }
          // now guess whether user installed or not (3rd next field contains 'user@host')
          if ( (sep1 = ::strchr( sep2+1, '|' ))		// | after version
            && (sep1 = ::strchr( sep1+1, '|' ))		// | after arch
            && (sep2 = ::strchr( sep1+1, '|' )) )	// | after who
          {
            (*in)[sep2-ch] = '\0';
            if ( ::strchr( sep1+1, '@' ) )
            {
              // by user
              onSystemByUserList.insert( pkg );
              continue;
            }
          }
        }
        MIL << "onSystemByUserList found: " << onSystemByUserList.size() << endl;
        return onSystemByUserList;
      }
    } // namespace
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace
    {
      inline PluginFrame transactionPluginFrame( const std::string & command_r, ZYppCommitResult::TransactionStepList & steps_r )
      {
        return PluginFrame( command_r, json::Object {
          { "TransactionStepList", steps_r }
        }.asJSON() );
      }
    } // namespace
    ///////////////////////////////////////////////////////////////////

    /** \internal Manage writing a new testcase when doing an upgrade. */
    void writeUpgradeTestcase()
    {
      unsigned toKeep( ZConfig::instance().solver_upgradeTestcasesToKeep() );
      MIL << "Testcases to keep: " << toKeep << endl;
      if ( !toKeep )
        return;
      Target_Ptr target( getZYpp()->getTarget() );
      if ( ! target )
      {
        WAR << "No Target no Testcase!" << endl;
        return;
      }

      std::string stem( "updateTestcase" );
      Pathname dir( target->assertRootPrefix("/var/log/") );
      Pathname next( dir / Date::now().form( stem+"-%Y-%m-%d-%H-%M-%S" ) );

      {
        std::list<std::string> content;
        filesystem::readdir( content, dir, /*dots*/false );
        std::set<std::string> cases;
        for_( c, content.begin(), content.end() )
        {
          if ( str::startsWith( *c, stem ) )
            cases.insert( *c );
        }
        if ( cases.size() >= toKeep )
        {
          unsigned toDel = cases.size() - toKeep + 1; // +1 for the new one
          for_( c, cases.begin(), cases.end() )
          {
            filesystem::recursive_rmdir( dir/(*c) );
            if ( ! --toDel )
              break;
          }
        }
      }

      MIL << "Write new testcase " << next << endl;
      getZYpp()->resolver()->createSolverTestcase( next.asString(), false/*no solving*/ );
    }

    ///////////////////////////////////////////////////////////////////
    namespace
    { /////////////////////////////////////////////////////////////////

      /** Execute script and report against report_r.
       * Return \c std::pair<bool,PatchScriptReport::Action> to indicate if
       * execution was successful (<tt>first = true</tt>), or the desired
       * \c PatchScriptReport::Action in case execution failed
       * (<tt>first = false</tt>).
       *
       * \note The packager is responsible for setting the correct permissions
       * of the script. If the script is not executable it is reported as an
       * error. We must not modify the permessions.
       */
      std::pair<bool,PatchScriptReport::Action> doExecuteScript( const Pathname & root_r,
                                                                 const Pathname & script_r,
                                                                 callback::SendReport<PatchScriptReport> & report_r )
      {
        MIL << "Execute script " << PathInfo(Pathname::assertprefix( root_r,script_r)) << endl;

        HistoryLog historylog;
        historylog.comment(script_r.asString() + _(" executed"), /*timestamp*/true);
        ExternalProgram prog( script_r.asString(), ExternalProgram::Stderr_To_Stdout, false, -1, true, root_r );

        for ( std::string output = prog.receiveLine(); output.length(); output = prog.receiveLine() )
        {
          historylog.comment(output);
          if ( ! report_r->progress( PatchScriptReport::OUTPUT, output ) )
          {
            WAR << "User request to abort script " << script_r << endl;
            prog.kill();
            // the rest is handled by exit code evaluation
            // in case the script has meanwhile finished.
          }
        }

        std::pair<bool,PatchScriptReport::Action> ret( std::make_pair( false, PatchScriptReport::ABORT ) );

        if ( prog.close() != 0 )
        {
          ret.second = report_r->problem( prog.execError() );
          WAR << "ACTION" << ret.second << "(" << prog.execError() << ")" << endl;
          std::ostringstream sstr;
          sstr << script_r << _(" execution failed") << " (" << prog.execError() << ")" << endl;
          historylog.comment(sstr.str(), /*timestamp*/true);
          return ret;
        }

        report_r->finish();
        ret.first = true;
        return ret;
      }

      /** Execute script and report against report_r.
       * Return \c false if user requested \c ABORT.
       */
      bool executeScript( const Pathname & root_r,
                          const Pathname & script_r,
                          callback::SendReport<PatchScriptReport> & report_r )
      {
        std::pair<bool,PatchScriptReport::Action> action( std::make_pair( false, PatchScriptReport::ABORT ) );

        do {
          action = doExecuteScript( root_r, script_r, report_r );
          if ( action.first )
            return true; // success

          switch ( action.second )
          {
            case PatchScriptReport::ABORT:
              WAR << "User request to abort at script " << script_r << endl;
              return false; // requested abort.
              break;

            case PatchScriptReport::IGNORE:
              WAR << "User request to skip script " << script_r << endl;
              return true; // requested skip.
              break;

            case PatchScriptReport::RETRY:
              break; // again
          }
        } while ( action.second == PatchScriptReport::RETRY );

        // THIS is not intended to be reached:
        INT << "Abort on unknown ACTION request " << action.second << " returned" << endl;
        return false; // abort.
      }

      /** Look for update scripts named 'name-version-release-*' and
       *  execute them. Return \c false if \c ABORT was requested.
       *
       * \see http://en.opensuse.org/Software_Management/Code11/Scripts_and_Messages
       */
      bool RunUpdateScripts( const Pathname & root_r,
                             const Pathname & scriptsPath_r,
                             const std::vector<sat::Solvable> & checkPackages_r,
                             bool aborting_r )
      {
        if ( checkPackages_r.empty() )
          return true; // no installed packages to check

        MIL << "Looking for new update scripts in (" <<  root_r << ")" << scriptsPath_r << endl;
        Pathname scriptsDir( Pathname::assertprefix( root_r, scriptsPath_r ) );
        if ( ! PathInfo( scriptsDir ).isDir() )
          return true; // no script dir

        std::list<std::string> scripts;
        filesystem::readdir( scripts, scriptsDir, /*dots*/false );
        if ( scripts.empty() )
          return true; // no scripts in script dir

        // Now collect and execute all matching scripts.
        // On ABORT: at least log all outstanding scripts.
        // - "name-version-release"
        // - "name-version-release-*"
        bool abort = false;
        std::map<std::string, Pathname> unify; // scripts <md5,path>
        for_( it, checkPackages_r.begin(), checkPackages_r.end() )
        {
          std::string prefix( str::form( "%s-%s", it->name().c_str(), it->edition().c_str() ) );
          for_( sit, scripts.begin(), scripts.end() )
          {
            if ( ! str::hasPrefix( *sit, prefix ) )
              continue;

            if ( (*sit)[prefix.size()] != '\0' && (*sit)[prefix.size()] != '-' )
              continue; // if not exact match it had to continue with '-'

            PathInfo script( scriptsDir / *sit );
            Pathname localPath( scriptsPath_r/(*sit) );	// without root prefix
            std::string unifytag;			// must not stay empty

            if ( script.isFile() )
            {
              // Assert it's set as executable, unify by md5sum.
              filesystem::addmod( script.path(), 0500 );
              unifytag = filesystem::md5sum( script.path() );
            }
            else if ( ! script.isExist() )
            {
              // Might be a dangling symlink, might be ok if we are in
              // instsys (absolute symlink within the system below /mnt).
              // readlink will tell....
              unifytag = filesystem::readlink( script.path() ).asString();
            }

            if ( unifytag.empty() )
              continue;

            // Unify scripts
            if ( unify[unifytag].empty() )
            {
              unify[unifytag] = localPath;
            }
            else
            {
              // translators: We may find the same script content in files with different names.
              // Only the first occurence is executed, subsequent ones are skipped. It's a one-line
              // message for a log file. Preferably start translation with "%s"
              std::string msg( str::form(_("%s already executed as %s)"), localPath.asString().c_str(), unify[unifytag].c_str() ) );
              MIL << "Skip update script: " << msg << endl;
              HistoryLog().comment( msg, /*timestamp*/true );
              continue;
            }

            if ( abort || aborting_r )
            {
              WAR << "Aborting: Skip update script " << *sit << endl;
              HistoryLog().comment(
                  localPath.asString() + _(" execution skipped while aborting"),
                  /*timestamp*/true);
            }
            else
            {
              MIL << "Found update script " << *sit << endl;
              callback::SendReport<PatchScriptReport> report;
              report->start( make<Package>( *it ), script.path() );

              if ( ! executeScript( root_r, localPath, report ) ) // script path without root prefix!
                abort = true; // requested abort.
            }
          }
        }
        return !abort;
      }

      ///////////////////////////////////////////////////////////////////
      //
      ///////////////////////////////////////////////////////////////////

      inline void copyTo( std::ostream & out_r, const Pathname & file_r )
      {
        std::ifstream infile( file_r.c_str() );
        for( iostr::EachLine in( infile ); in; in.next() )
        {
          out_r << *in << endl;
        }
      }

      inline std::string notificationCmdSubst( const std::string & cmd_r, const UpdateNotificationFile & notification_r )
      {
        std::string ret( cmd_r );
#define SUBST_IF(PAT,VAL) if ( ret.find( PAT ) != std::string::npos ) ret = str::gsub( ret, PAT, VAL )
        SUBST_IF( "%p", notification_r.solvable().asString() );
        SUBST_IF( "%P", notification_r.file().asString() );
#undef SUBST_IF
        return ret;
      }

      void sendNotification( const Pathname & root_r,
                             const UpdateNotifications & notifications_r )
      {
        if ( notifications_r.empty() )
          return;

        std::string cmdspec( ZConfig::instance().updateMessagesNotify() );
        MIL << "Notification command is '" << cmdspec << "'" << endl;
        if ( cmdspec.empty() )
          return;

        std::string::size_type pos( cmdspec.find( '|' ) );
        if ( pos == std::string::npos )
        {
          ERR << "Can't send Notification: Missing 'format |' in command spec." << endl;
          HistoryLog().comment( str::Str() << _("Error sending update message notification."), /*timestamp*/true );
          return;
        }

        std::string formatStr( str::toLower( str::trim( cmdspec.substr( 0, pos ) ) ) );
        std::string commandStr( str::trim( cmdspec.substr( pos + 1 ) ) );

        enum Format { UNKNOWN, NONE, SINGLE, DIGEST, BULK };
        Format format = UNKNOWN;
        if ( formatStr == "none" )
          format = NONE;
        else if ( formatStr == "single" )
          format = SINGLE;
        else if ( formatStr == "digest" )
          format = DIGEST;
        else if ( formatStr == "bulk" )
          format = BULK;
        else
        {
          ERR << "Can't send Notification: Unknown format '" << formatStr << " |' in command spec." << endl;
          HistoryLog().comment( str::Str() << _("Error sending update message notification."), /*timestamp*/true );
         return;
        }

        // Take care: commands are ececuted chroot(root_r). The message file
        // pathnames in notifications_r are local to root_r. For physical access
        // to the file they need to be prefixed.

        if ( format == NONE || format == SINGLE )
        {
          for_( it, notifications_r.begin(), notifications_r.end() )
          {
            std::vector<std::string> command;
            if ( format == SINGLE )
              command.push_back( "<"+Pathname::assertprefix( root_r, it->file() ).asString() );
            str::splitEscaped( notificationCmdSubst( commandStr, *it ), std::back_inserter( command ) );

            ExternalProgram prog( command, ExternalProgram::Stderr_To_Stdout, false, -1, true, root_r );
            if ( true ) // Wait for feedback
            {
              for( std::string line = prog.receiveLine(); ! line.empty(); line = prog.receiveLine() )
              {
                DBG << line;
              }
              int ret = prog.close();
              if ( ret != 0 )
              {
                ERR << "Notification command returned with error (" << ret << ")." << endl;
                HistoryLog().comment( str::Str() << _("Error sending update message notification."), /*timestamp*/true );
                return;
              }
            }
          }
        }
        else if ( format == DIGEST || format == BULK )
        {
          filesystem::TmpFile tmpfile;
          std::ofstream out( tmpfile.path().c_str() );
          for_( it, notifications_r.begin(), notifications_r.end() )
          {
            if ( format == DIGEST )
            {
              out << it->file() << endl;
            }
            else if ( format == BULK )
            {
              copyTo( out << '\f', Pathname::assertprefix( root_r, it->file() ) );
            }
          }

          std::vector<std::string> command;
          command.push_back( "<"+tmpfile.path().asString() ); // redirect input
          str::splitEscaped( notificationCmdSubst( commandStr, *notifications_r.begin() ), std::back_inserter( command ) );

          ExternalProgram prog( command, ExternalProgram::Stderr_To_Stdout, false, -1, true, root_r );
          if ( true ) // Wait for feedback otherwise the TmpFile goes out of scope.
          {
            for( std::string line = prog.receiveLine(); ! line.empty(); line = prog.receiveLine() )
            {
              DBG << line;
            }
            int ret = prog.close();
            if ( ret != 0 )
            {
              ERR << "Notification command returned with error (" << ret << ")." << endl;
              HistoryLog().comment( str::Str() << _("Error sending update message notification."), /*timestamp*/true );
              return;
            }
          }
        }
        else
        {
          INT << "Can't send Notification: Missing handler for 'format |' in command spec." << endl;
          HistoryLog().comment( str::Str() << _("Error sending update message notification."), /*timestamp*/true );
          return;
        }
      }


      /** Look for update messages named 'name-version-release-*' and
       *  send notification according to \ref ZConfig::updateMessagesNotify.
       *
       * \see http://en.opensuse.org/Software_Management/Code11/Scripts_and_Messages
       */
      void RunUpdateMessages( const Pathname & root_r,
                              const Pathname & messagesPath_r,
                              const std::vector<sat::Solvable> & checkPackages_r,
                              ZYppCommitResult & result_r )
      {
        if ( checkPackages_r.empty() )
          return; // no installed packages to check

        MIL << "Looking for new update messages in (" <<  root_r << ")" << messagesPath_r << endl;
        Pathname messagesDir( Pathname::assertprefix( root_r, messagesPath_r ) );
        if ( ! PathInfo( messagesDir ).isDir() )
          return; // no messages dir

        std::list<std::string> messages;
        filesystem::readdir( messages, messagesDir, /*dots*/false );
        if ( messages.empty() )
          return; // no messages in message dir

        // Now collect all matching messages in result and send them
        // - "name-version-release"
        // - "name-version-release-*"
        HistoryLog historylog;
        for_( it, checkPackages_r.begin(), checkPackages_r.end() )
        {
          std::string prefix( str::form( "%s-%s", it->name().c_str(), it->edition().c_str() ) );
          for_( sit, messages.begin(), messages.end() )
          {
            if ( ! str::hasPrefix( *sit, prefix ) )
              continue;

            if ( (*sit)[prefix.size()] != '\0' && (*sit)[prefix.size()] != '-' )
              continue; // if not exact match it had to continue with '-'

            PathInfo message( messagesDir / *sit );
            if ( ! message.isFile() || message.size() == 0 )
              continue;

            MIL << "Found update message " << *sit << endl;
            Pathname localPath( messagesPath_r/(*sit) ); // without root prefix
            result_r.rUpdateMessages().push_back( UpdateNotificationFile( *it, localPath ) );
            historylog.comment( str::Str() << _("New update message") << " " << localPath, /*timestamp*/true );
          }
        }
        sendNotification( root_r, result_r.updateMessages() );
      }

      /** jsc#SLE-5116: Log patch status changes to history.
       * Adjust precomputed set if transaction is incomplete.
       */
      void logPatchStatusChanges( const sat::Transaction & transaction_r, TargetImpl & target_r )
      {
        ResPool::ChangedPseudoInstalled changedPseudoInstalled { ResPool::instance().changedPseudoInstalled() };
        if ( changedPseudoInstalled.empty() )
          return;

        if ( ! transaction_r.actionEmpty( ~sat::Transaction::STEP_DONE ) )
        {
          // Need to recompute the patch list if commit is incomplete!
          // We remember the initially established status, then reload the
          // Target to get the current patch status. Then compare.
          WAR << "Need to recompute the patch status changes as commit is incomplete!" << endl;
          ResPool::EstablishedStates establishedStates{ ResPool::instance().establishedStates() };
          target_r.load();
          changedPseudoInstalled = establishedStates.changedPseudoInstalled();
        }

        HistoryLog historylog;
        for ( const auto & el : changedPseudoInstalled )
          historylog.patchStateChange( el.first, el.second );
      }

      /////////////////////////////////////////////////////////////////
    } // namespace
    ///////////////////////////////////////////////////////////////////

    void XRunUpdateMessages( const Pathname & root_r,
                             const Pathname & messagesPath_r,
                             const std::vector<sat::Solvable> & checkPackages_r,
                             ZYppCommitResult & result_r )
    { RunUpdateMessages( root_r, messagesPath_r, checkPackages_r, result_r ); }

    ///////////////////////////////////////////////////////////////////

    IMPL_PTR_TYPE(TargetImpl);

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : TargetImpl::TargetImpl
    //	METHOD TYPE : Ctor
    //
    TargetImpl::TargetImpl( const Pathname & root_r, bool doRebuild_r )
    : _root( root_r )
    , _requestedLocalesFile( home() / "RequestedLocales" )
    , _autoInstalledFile( home() / "AutoInstalled" )
    , _hardLocksFile( Pathname::assertprefix( _root, ZConfig::instance().locksFile() ) )
    , _vendorAttr( Pathname::assertprefix( _root, ZConfig::instance().vendorPath() ) )
    {
      _rpm.initDatabase( root_r, doRebuild_r );

      HistoryLog::setRoot(_root);

      createAnonymousId();
      sigMultiversionSpecChanged();	// HACK: see sigMultiversionSpecChanged
      MIL << "Initialized target on " << _root << endl;
    }

    /**
     * generates a random id using uuidgen
     */
    static std::string generateRandomId()
    {
      std::ifstream uuidprovider( "/proc/sys/kernel/random/uuid" );
      return iostr::getline( uuidprovider );
    }

    /**
     * updates the content of \p filename
     * if \p condition is true, setting the content
     * the the value returned by \p value
     */
    void updateFileContent( const Pathname &filename,
                            boost::function<bool ()> condition,
                            boost::function<std::string ()> value )
    {
        std::string val = value();
        // if the value is empty, then just dont
        // do anything, regardless of the condition
        if ( val.empty() )
            return;

        if ( condition() )
        {
            MIL << "updating '" << filename << "' content." << endl;

            // if the file does not exist we need to generate the uuid file

            std::ofstream filestr;
            // make sure the path exists
            filesystem::assert_dir( filename.dirname() );
            filestr.open( filename.c_str() );

            if ( filestr.good() )
            {
                filestr << val;
                filestr.close();
            }
            else
            {
                // FIXME, should we ignore the error?
                ZYPP_THROW(Exception("Can't openfile '" + filename.asString() + "' for writing"));
            }
        }
    }

    /** helper functor */
    static bool fileMissing( const Pathname &pathname )
    {
        return ! PathInfo(pathname).isExist();
    }

    void TargetImpl::createAnonymousId() const
    {
      // bsc#1024741: Omit creating a new uid for chrooted systems (if it already has one, fine)
      if ( root() != "/" )
        return;

      // Create the anonymous unique id, used for download statistics
      Pathname idpath( home() / "AnonymousUniqueId");

      try
      {
        updateFileContent( idpath,
                           boost::bind(fileMissing, idpath),
                           generateRandomId );
      }
      catch ( const Exception &e )
      {
        WAR << "Can't create anonymous id file" << endl;
      }

    }

    void TargetImpl::createLastDistributionFlavorCache() const
    {
      // create the anonymous unique id
      // this value is used for statistics
      Pathname flavorpath( home() / "LastDistributionFlavor");

      // is there a product
      Product::constPtr p = baseProduct();
      if ( ! p )
      {
          WAR << "No base product, I won't create flavor cache" << endl;
          return;
      }

      std::string flavor = p->flavor();

      try
      {

        updateFileContent( flavorpath,
                           // only if flavor is not empty
                           functor::Constant<bool>( ! flavor.empty() ),
                           functor::Constant<std::string>(flavor) );
      }
      catch ( const Exception &e )
      {
        WAR << "Can't create flavor cache" << endl;
        return;
      }
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : TargetImpl::~TargetImpl
    //	METHOD TYPE : Dtor
    //
    TargetImpl::~TargetImpl()
    {
      _rpm.closeDatabase();
      sigMultiversionSpecChanged();	// HACK: see sigMultiversionSpecChanged
      MIL << "Targets closed" << endl;
    }

    ///////////////////////////////////////////////////////////////////
    //
    // solv file handling
    //
    ///////////////////////////////////////////////////////////////////

    Pathname TargetImpl::defaultSolvfilesPath() const
    {
      return Pathname::assertprefix( _root, ZConfig::instance().repoSolvfilesPath() / sat::Pool::instance().systemRepoAlias() );
    }

    void TargetImpl::clearCache()
    {
      Pathname base = solvfilesPath();
      filesystem::recursive_rmdir( base );
    }

    bool TargetImpl::buildCache()
    {
      Pathname base = solvfilesPath();
      Pathname rpmsolv       = base/"solv";
      Pathname rpmsolvcookie = base/"cookie";

      bool build_rpm_solv = true;
      // lets see if the rpm solv cache exists

      RepoStatus rpmstatus( rpmDbRepoStatus(_root) && RepoStatus(_root/"etc/products.d") );

      bool solvexisted = PathInfo(rpmsolv).isExist();
      if ( solvexisted )
      {
        // see the status of the cache
        PathInfo cookie( rpmsolvcookie );
        MIL << "Read cookie: " << cookie << endl;
        if ( cookie.isExist() )
        {
          RepoStatus status = RepoStatus::fromCookieFile(rpmsolvcookie);
          // now compare it with the rpm database
          if ( status == rpmstatus )
            build_rpm_solv = false;
          MIL << "Read cookie: " << rpmsolvcookie << " says: "
          << (build_rpm_solv ? "outdated" : "uptodate") << endl;
        }
      }

      if ( build_rpm_solv )
      {
        // if the solvfile dir does not exist yet, we better create it
        filesystem::assert_dir( base );

        Pathname oldSolvFile( solvexisted ? rpmsolv : Pathname() ); // to speedup rpmdb2solv

        filesystem::TmpFile tmpsolv( filesystem::TmpFile::makeSibling( rpmsolv ) );
        if ( !tmpsolv )
        {
          // Can't create temporary solv file, usually due to insufficient permission
          // (user query while @System solv needs refresh). If so, try switching
          // to a location within zypps temp. space (will be cleaned at application end).

          bool switchingToTmpSolvfile = false;
          Exception ex("Failed to cache rpm database.");
          ex.remember(str::form("Cannot create temporary file under %s.", base.c_str()));

          if ( ! solvfilesPathIsTemp() )
          {
            base = getZYpp()->tmpPath() / sat::Pool::instance().systemRepoAlias();
            rpmsolv       = base/"solv";
            rpmsolvcookie = base/"cookie";

            filesystem::assert_dir( base );
            tmpsolv = filesystem::TmpFile::makeSibling( rpmsolv );

            if ( tmpsolv )
            {
              WAR << "Using a temporary solv file at " << base << endl;
              switchingToTmpSolvfile = true;
              _tmpSolvfilesPath = base;
            }
            else
            {
              ex.remember(str::form("Cannot create temporary file under %s.", base.c_str()));
            }
          }

          if ( ! switchingToTmpSolvfile )
          {
            ZYPP_THROW(ex);
          }
        }

        // Take care we unlink the solvfile on exception
        ManagedFile guard( base, filesystem::recursive_rmdir );

        ExternalProgram::Arguments cmd;
        cmd.push_back( "rpmdb2solv" );
        if ( ! _root.empty() ) {
          cmd.push_back( "-r" );
          cmd.push_back( _root.asString() );
        }
        cmd.push_back( "-D" );
        cmd.push_back( rpm().dbPath().asString() );
        cmd.push_back( "-X" );	// autogenerate pattern/product/... from -package
        // bsc#1104415: no more application support // cmd.push_back( "-A" );	// autogenerate application pseudo packages
        cmd.push_back( "-p" );
        cmd.push_back( Pathname::assertprefix( _root, "/etc/products.d" ).asString() );

        if ( ! oldSolvFile.empty() )
          cmd.push_back( oldSolvFile.asString() );

        cmd.push_back( "-o" );
        cmd.push_back( tmpsolv.path().asString() );

        ExternalProgram prog( cmd, ExternalProgram::Stderr_To_Stdout );
        std::string errdetail;

        for ( std::string output( prog.receiveLine() ); output.length(); output = prog.receiveLine() ) {
          WAR << "  " << output;
          if ( errdetail.empty() ) {
            errdetail = prog.command();
            errdetail += '\n';
          }
          errdetail += output;
        }

        int ret = prog.close();
        if ( ret != 0 )
        {
          Exception ex(str::form("Failed to cache rpm database (%d).", ret));
          ex.remember( errdetail );
          ZYPP_THROW(ex);
        }

        ret = filesystem::rename( tmpsolv, rpmsolv );
        if ( ret != 0 )
          ZYPP_THROW(Exception("Failed to move cache to final destination"));
        // if this fails, don't bother throwing exceptions
        filesystem::chmod( rpmsolv, 0644 );

        rpmstatus.saveToCookieFile(rpmsolvcookie);

        // We keep it.
        guard.resetDispose();
        sat::updateSolvFileIndex( rpmsolv );	// content digest for zypper bash completion

        // system-hook: Finally send notification to plugins
        if ( root() == "/" )
        {
          PluginExecutor plugins;
          plugins.load( ZConfig::instance().pluginsPath()/"system" );
          if ( plugins )
            plugins.send( PluginFrame( "PACKAGESETCHANGED" ) );
        }
      }
      else
      {
        // On the fly add missing solv.idx files for bash completion.
        if ( ! PathInfo(base/"solv.idx").isExist() )
          sat::updateSolvFileIndex( rpmsolv );
      }
      return build_rpm_solv;
    }

    void TargetImpl::reload()
    {
        load( false );
    }

    void TargetImpl::unload()
    {
      Repository system( sat::Pool::instance().findSystemRepo() );
      if ( system )
        system.eraseFromPool();
    }

    void TargetImpl::load( bool force )
    {
      bool newCache = buildCache();
      MIL << "New cache built: " << (newCache?"true":"false") <<
        ", force loading: " << (force?"true":"false") << endl;

      // now add the repos to the pool
      sat::Pool satpool( sat::Pool::instance() );
      Pathname rpmsolv( solvfilesPath() / "solv" );
      MIL << "adding " << rpmsolv << " to pool(" << satpool.systemRepoAlias() << ")" << endl;

      // Providing an empty system repo, unload any old content
      Repository system( sat::Pool::instance().findSystemRepo() );

      if ( system && ! system.solvablesEmpty() )
      {
        if ( newCache || force )
        {
          system.eraseFromPool(); // invalidates system
        }
        else
        {
          return;     // nothing to do
        }
      }

      if ( ! system )
      {
        system = satpool.systemRepo();
      }

      try
      {
        MIL << "adding " << rpmsolv << " to system" << endl;
        system.addSolv( rpmsolv );
      }
      catch ( const Exception & exp )
      {
        ZYPP_CAUGHT( exp );
        MIL << "Try to handle exception by rebuilding the solv-file" << endl;
        clearCache();
        buildCache();

        system.addSolv( rpmsolv );
      }
      satpool.rootDir( _root );

      // (Re)Load the requested locales et al.
      // If the requested locales are empty, we leave the pool untouched
      // to avoid undoing changes the application applied. We expect this
      // to happen on a bare metal installation only. An already existing
      // target should be loaded before its settings are changed.
      {
        const LocaleSet & requestedLocales( _requestedLocalesFile.locales() );
        if ( ! requestedLocales.empty() )
        {
          satpool.initRequestedLocales( requestedLocales );
        }
      }
      {
        if ( ! PathInfo( _autoInstalledFile.file() ).isExist() )
        {
          // Initialize from history, if it does not exist
          Pathname historyFile( Pathname::assertprefix( _root, ZConfig::instance().historyLogFile() ) );
          if ( PathInfo( historyFile ).isExist() )
          {
            SolvIdentFile::Data onSystemByUser( getUserInstalledFromHistory( historyFile ) );
            SolvIdentFile::Data onSystemByAuto;
            for_( it, system.solvablesBegin(), system.solvablesEnd() )
            {
              IdString ident( (*it).ident() );
              if ( onSystemByUser.find( ident ) == onSystemByUser.end() )
                onSystemByAuto.insert( ident );
            }
            _autoInstalledFile.setData( onSystemByAuto );
          }
          // on the fly removed any obsolete SoftLocks file
          filesystem::unlink( home() / "SoftLocks" );
        }
        // read from AutoInstalled file
        sat::StringQueue q;
        for ( const auto & idstr : _autoInstalledFile.data() )
          q.push( idstr.id() );
        satpool.setAutoInstalled( q );
      }

      // Load the needreboot package specs
      {
        sat::SolvableSpec needrebootSpec;
        needrebootSpec.addProvides( Capability("installhint(reboot-needed)") );
        needrebootSpec.addProvides( Capability("kernel") );
        needrebootSpec.addIdent( IdString("kernel-firmware") );

        Pathname needrebootFile { Pathname::assertprefix( root(), ZConfig::instance().needrebootFile() ) };
        if ( PathInfo( needrebootFile ).isFile() )
          needrebootSpec.parseFrom( needrebootFile );

        Pathname needrebootDir { Pathname::assertprefix( root(), ZConfig::instance().needrebootPath() ) };
        if ( PathInfo( needrebootDir ).isDir() )
        {
          static const StrMatcher isRpmConfigBackup( "\\.rpm(new|save|orig)$", Match::REGEX );

          filesystem::dirForEach( needrebootDir, filesystem::matchNoDots(),
                                  [&]( const Pathname & dir_r, const char *const str_r )->bool
                                  {
                                    if ( ! isRpmConfigBackup( str_r ) )
                                    {
                                      Pathname needrebootFile { needrebootDir / str_r };
                                      if ( PathInfo( needrebootFile ).isFile() )
                                        needrebootSpec.parseFrom( needrebootFile );
                                    }
                                    return true;
                                  });
        }
        satpool.setNeedrebootSpec( std::move(needrebootSpec) );
      }

      if ( ZConfig::instance().apply_locks_file() )
      {
        const HardLocksFile::Data & hardLocks( _hardLocksFile.data() );
        if ( ! hardLocks.empty() )
        {
          ResPool::instance().setHardLockQueries( hardLocks );
        }
      }

      // now that the target is loaded, we can cache the flavor
      createLastDistributionFlavorCache();

      MIL << "Target loaded: " << system.solvablesSize() << " resolvables" << endl;
    }

    ///////////////////////////////////////////////////////////////////
    //
    // COMMIT
    //
    ///////////////////////////////////////////////////////////////////
    ZYppCommitResult TargetImpl::commit( ResPool pool_r, const ZYppCommitPolicy & policy_rX )
    {
      // ----------------------------------------------------------------- //
      ZYppCommitPolicy policy_r( policy_rX );
      bool explicitDryRun = policy_r.dryRun();	// explicit dry run will trigger a fileconflict check, implicit (download-only) not.

      ShutdownLock lck("Zypp commit running.");

      // Fake outstanding YCP fix: Honour restriction to media 1
      // at installation, but install all remaining packages if post-boot.
      if ( policy_r.restrictToMedia() > 1 )
        policy_r.allMedia();

      if ( policy_r.downloadMode() == DownloadDefault ) {
        if ( root() == "/" )
          policy_r.downloadMode(DownloadInHeaps);
        else {
          if ( policy_r.singleTransModeEnabled() )
            policy_r.downloadMode(DownloadInAdvance);
          else
            policy_r.downloadMode(DownloadAsNeeded);
        }
      }
      // DownloadOnly implies dry-run.
      else if ( policy_r.downloadMode() == DownloadOnly )
        policy_r.dryRun( true );
      // ----------------------------------------------------------------- //

      MIL << "TargetImpl::commit(<pool>, " << policy_r << ")" << endl;

      ///////////////////////////////////////////////////////////////////
      // Compute transaction:
      ///////////////////////////////////////////////////////////////////
      ZYppCommitResult result( root() );
      result.rTransaction() = pool_r.resolver().getTransaction();
      result.rTransaction().order();
      // steps: this is our todo-list
      ZYppCommitResult::TransactionStepList & steps( result.rTransactionStepList() );
      if ( policy_r.restrictToMedia() )
      {
        // Collect until the 1st package from an unwanted media occurs.
        // Further collection could violate install order.
        MIL << "Restrict to media number " << policy_r.restrictToMedia() << endl;
        for_( it, result.transaction().begin(), result.transaction().end() )
        {
          if ( makeResObject( *it )->mediaNr() > 1 )
            break;
          steps.push_back( *it );
        }
      }
      else
      {
        result.rTransactionStepList().insert( steps.end(), result.transaction().begin(), result.transaction().end() );
      }
      MIL << "Todo: " << result << endl;

      ///////////////////////////////////////////////////////////////////
      // Prepare execution of commit plugins:
      ///////////////////////////////////////////////////////////////////
      PluginExecutor commitPlugins;
      if ( root() == "/" && ! policy_r.dryRun() )
      {
        commitPlugins.load( ZConfig::instance().pluginsPath()/"commit" );
      }
      if ( commitPlugins )
        commitPlugins.send( transactionPluginFrame( "COMMITBEGIN", steps ) );

      ///////////////////////////////////////////////////////////////////
      // Write out a testcase if we're in dist upgrade mode.
      ///////////////////////////////////////////////////////////////////
      if ( pool_r.resolver().upgradeMode() || pool_r.resolver().upgradingRepos() )
      {
        if ( ! policy_r.dryRun() )
        {
          writeUpgradeTestcase();
        }
        else
        {
          DBG << "dryRun: Not writing upgrade testcase." << endl;
        }
      }

     ///////////////////////////////////////////////////////////////////
      // Store non-package data:
      ///////////////////////////////////////////////////////////////////
      if ( ! policy_r.dryRun() )
      {
        filesystem::assert_dir( home() );
        // requested locales
        _requestedLocalesFile.setLocales( pool_r.getRequestedLocales() );
        // autoinstalled
        {
          SolvIdentFile::Data newdata;
          for ( sat::Queue::value_type id : result.rTransaction().autoInstalled() )
            newdata.insert( IdString(id) );
          _autoInstalledFile.setData( newdata );
        }
        // hard locks
        if ( ZConfig::instance().apply_locks_file() )
        {
          HardLocksFile::Data newdata;
          pool_r.getHardLockQueries( newdata );
          _hardLocksFile.setData( newdata );
        }
      }
      else
      {
        DBG << "dryRun: Not storing non-package data." << endl;
      }

      ///////////////////////////////////////////////////////////////////
      // First collect and display all messages
      // associated with patches to be installed.
      ///////////////////////////////////////////////////////////////////
      if ( ! policy_r.dryRun() )
      {
        for_( it, steps.begin(), steps.end() )
        {
          if ( ! it->satSolvable().isKind<Patch>() )
            continue;

          PoolItem pi( *it );
          if ( ! pi.status().isToBeInstalled() )
            continue;

          Patch::constPtr patch( asKind<Patch>(pi.resolvable()) );
          if ( ! patch ||patch->message().empty()  )
            continue;

          MIL << "Show message for " << patch << endl;
          callback::SendReport<target::PatchMessageReport> report;
          if ( ! report->show( patch ) )
          {
            WAR << "commit aborted by the user" << endl;
            ZYPP_THROW( TargetAbortedException( ) );
          }
        }
      }
      else
      {
        DBG << "dryRun: Not checking patch messages." << endl;
      }

      ///////////////////////////////////////////////////////////////////
      // Remove/install packages.
      ///////////////////////////////////////////////////////////////////

      bool singleTransMode = policy_r.singleTransModeEnabled();

      DBG << "commit log file is set to: " << HistoryLog::fname() << endl;
      if ( ! policy_r.dryRun() || policy_r.downloadMode() == DownloadOnly || singleTransMode )
      {
        // Prepare the package cache. Pass all items requiring download.
        CommitPackageCache packageCache;
        packageCache.setCommitList( steps.begin(), steps.end() );

        bool miss = false;
        if ( policy_r.downloadMode() != DownloadAsNeeded || singleTransMode )
        {
          // Preload the cache. Until now this means pre-loading all packages.
          // Once DownloadInHeaps is fully implemented, this will change and
          // we may actually have more than one heap.
          for_( it, steps.begin(), steps.end() )
          {
            switch ( it->stepType() )
            {
              case sat::Transaction::TRANSACTION_INSTALL:
              case sat::Transaction::TRANSACTION_MULTIINSTALL:
                // proceed: only install actionas may require download.
                break;

              default:
                // next: no download for or non-packages and delete actions.
                continue;
                break;
            }

            PoolItem pi( *it );
            if ( pi->isKind<Package>() || pi->isKind<SrcPackage>() )
            {
              ManagedFile localfile;
              try
              {
                localfile = packageCache.get( pi );
                localfile.resetDispose(); // keep the package file in the cache
              }
              catch ( const AbortRequestException & exp )
              {
                it->stepStage( sat::Transaction::STEP_ERROR );
                miss = true;
                WAR << "commit cache preload aborted by the user" << endl;
                ZYPP_THROW( TargetAbortedException( ) );
                break;
              }
              catch ( const SkipRequestException & exp )
              {
                ZYPP_CAUGHT( exp );
                it->stepStage( sat::Transaction::STEP_ERROR );
                miss = true;
                WAR << "Skipping cache preload package " << pi->asKind<Package>() << " in commit" << endl;
                continue;
              }
              catch ( const Exception & exp )
              {
                // bnc #395704: missing catch causes abort.
                // TODO see if packageCache fails to handle errors correctly.
                ZYPP_CAUGHT( exp );
                it->stepStage( sat::Transaction::STEP_ERROR );
                miss = true;
                INT << "Unexpected Error: Skipping cache preload package " << pi->asKind<Package>() << " in commit" << endl;
                continue;
              }
            }
          }
          packageCache.preloaded( true ); // try to avoid duplicate infoInCache CBs in commit
        }

        if ( miss )
        {
          ERR << "Some packages could not be provided. Aborting commit."<< endl;
        }
        else
        {
          // single trans mode does a test install via rpm
          if ( policy_r.singleTransModeEnabled() ) {
            commitInSingleTransaction( policy_r, packageCache, result );
          } else {
            if ( ! policy_r.dryRun() )
            {
              // if cache is preloaded, check for file conflicts
              commitFindFileConflicts( policy_r, result );
              commit( policy_r, packageCache, result );
            }
            else
            {
              DBG << "dryRun/downloadOnly: Not installing/deleting anything." << endl;
              if ( explicitDryRun ) {
                // if cache is preloaded, check for file conflicts
                commitFindFileConflicts( policy_r, result );
              }
            }
          }
        }
      }
      else
      {
        DBG << "dryRun: Not downloading/installing/deleting anything." << endl;
        if ( explicitDryRun ) {
          // if cache is preloaded, check for file conflicts
          commitFindFileConflicts( policy_r, result );
        }
      }

      {
        // NOTE: Removing rpm in a transaction, rpm removes the /var/lib/rpm compat symlink.
        // We re-create it, in case it was lost to prevent legacy tools from accidentally
        // assuming no database is present.
        if ( ! PathInfo(_root/"/var/lib/rpm",PathInfo::LSTAT).isExist()
          && PathInfo(_root/"/usr/lib/sysimage/rpm").isDir() ) {
          WAR << "(rpm removed in commit?) Inject missing /var/lib/rpm compat symlink to /usr/lib/sysimage/rpm" << endl;
          filesystem::assert_dir( _root/"/var/lib" );
          filesystem::symlink( "../../usr/lib/sysimage/rpm", _root/"/var/lib/rpm" );
        }
      }

      ///////////////////////////////////////////////////////////////////
      // Send result to commit plugins:
      ///////////////////////////////////////////////////////////////////
      if ( commitPlugins )
        commitPlugins.send( transactionPluginFrame( "COMMITEND", steps ) );

      ///////////////////////////////////////////////////////////////////
      // Try to rebuild solv file while rpm database is still in cache
      ///////////////////////////////////////////////////////////////////
      if ( ! policy_r.dryRun() )
      {
        buildCache();
      }

      MIL << "TargetImpl::commit(<pool>, " << policy_r << ") returns: " << result << endl;
      return result;
    }

    ///////////////////////////////////////////////////////////////////
    //
    // COMMIT internal
    //
    ///////////////////////////////////////////////////////////////////
    namespace
    {
      struct NotifyAttemptToModify
      {
        NotifyAttemptToModify( ZYppCommitResult & result_r ) : _result( result_r ) {}

        void operator()()
        { if ( _guard ) { _result.attemptToModify( true ); _guard = false; } }

        TrueBool           _guard;
        ZYppCommitResult & _result;
      };
    } // namespace

    void TargetImpl::commit( const ZYppCommitPolicy & policy_r,
                             CommitPackageCache & packageCache_r,
                             ZYppCommitResult & result_r )
    {
      // steps: this is our todo-list
      ZYppCommitResult::TransactionStepList & steps( result_r.rTransactionStepList() );
      MIL << "TargetImpl::commit(<list>" << policy_r << ")" << steps.size() << endl;

      HistoryLog().stampCommand();

      // Send notification once upon 1st call to rpm
      NotifyAttemptToModify attemptToModify( result_r );

      bool abort = false;

      // bsc#1181328: Some systemd tools require /proc to be mounted
      AssertProcMounted assertProcMounted( _root );

      RpmPostTransCollector postTransCollector( _root );
      std::vector<sat::Solvable> successfullyInstalledPackages;
      TargetImpl::PoolItemList remaining;

      for_( step, steps.begin(), steps.end() )
      {
        PoolItem citem( *step );
        if ( step->stepType() == sat::Transaction::TRANSACTION_IGNORE )
        {
          if ( citem->isKind<Package>() )
          {
            // for packages this means being obsoleted (by rpm)
            // thius no additional action is needed.
            step->stepStage( sat::Transaction::STEP_DONE );
            continue;
          }
        }

        if ( citem->isKind<Package>() )
        {
          Package::constPtr p = citem->asKind<Package>();
          if ( citem.status().isToBeInstalled() )
          {
            ManagedFile localfile;
            try
            {
              localfile = packageCache_r.get( citem );
            }
            catch ( const AbortRequestException &e )
            {
              WAR << "commit aborted by the user" << endl;
              abort = true;
              step->stepStage( sat::Transaction::STEP_ERROR );
              break;
            }
            catch ( const SkipRequestException &e )
            {
              ZYPP_CAUGHT( e );
              WAR << "Skipping package " << p << " in commit" << endl;
              step->stepStage( sat::Transaction::STEP_ERROR );
              continue;
            }
            catch ( const Exception &e )
            {
              // bnc #395704: missing catch causes abort.
              // TODO see if packageCache fails to handle errors correctly.
              ZYPP_CAUGHT( e );
              INT << "Unexpected Error: Skipping package " << p << " in commit" << endl;
              step->stepStage( sat::Transaction::STEP_ERROR );
              continue;
            }

            // create a installation progress report proxy
            RpmInstallPackageReceiver progress( citem.resolvable() );
            progress.connect(); // disconnected on destruction.

            bool success = false;
            rpm::RpmInstFlags flags( policy_r.rpmInstFlags() & rpm::RPMINST_JUSTDB );
            // Why force and nodeps?
            //
            // Because zypp builds the transaction and the resolver asserts that
            // everything is fine.
            // We use rpm just to unpack and register the package in the database.
            // We do this step by step, so rpm is not aware of the bigger context.
            // So we turn off rpms internal checks, because we do it inside zypp.
            flags |= rpm::RPMINST_NODEPS;
            flags |= rpm::RPMINST_FORCE;
            //
            if (p->multiversionInstall())  flags |= rpm::RPMINST_NOUPGRADE;
            if (policy_r.dryRun())         flags |= rpm::RPMINST_TEST;
            if (policy_r.rpmExcludeDocs()) flags |= rpm::RPMINST_EXCLUDEDOCS;
            if (policy_r.rpmNoSignature()) flags |= rpm::RPMINST_NOSIGNATURE;

            attemptToModify();
            try
            {
              progress.tryLevel( target::rpm::InstallResolvableReport::RPM_NODEPS_FORCE );
              if ( postTransCollector.collectScriptFromPackage( localfile ) )
                flags |= rpm::RPMINST_NOPOSTTRANS;
              rpm().installPackage( localfile, flags );
              HistoryLog().install(citem);

              if ( progress.aborted() )
              {
                WAR << "commit aborted by the user" << endl;
                localfile.resetDispose(); // keep the package file in the cache
                abort = true;
                step->stepStage( sat::Transaction::STEP_ERROR );
                break;
              }
              else
              {
                if ( citem.isNeedreboot() ) {
                  auto rebootNeededFile = root() / "/run/reboot-needed";
                  if ( filesystem::assert_file( rebootNeededFile ) == EEXIST)
                    filesystem::touch( rebootNeededFile );
                }

                success = true;
                step->stepStage( sat::Transaction::STEP_DONE );
              }
            }
            catch ( Exception & excpt_r )
            {
              ZYPP_CAUGHT(excpt_r);
              localfile.resetDispose(); // keep the package file in the cache

              if ( policy_r.dryRun() )
              {
                WAR << "dry run failed" << endl;
                step->stepStage( sat::Transaction::STEP_ERROR );
                break;
              }
              // else
              if ( progress.aborted() )
              {
                WAR << "commit aborted by the user" << endl;
                abort = true;
              }
              else
              {
                WAR << "Install failed" << endl;
              }
              step->stepStage( sat::Transaction::STEP_ERROR );
              break; // stop
            }

            if ( success && !policy_r.dryRun() )
            {
              citem.status().resetTransact( ResStatus::USER );
              successfullyInstalledPackages.push_back( citem.satSolvable() );
              step->stepStage( sat::Transaction::STEP_DONE );
            }
          }
          else
          {
            RpmRemovePackageReceiver progress( citem.resolvable() );
            progress.connect(); // disconnected on destruction.

            bool success = false;
            rpm::RpmInstFlags flags( policy_r.rpmInstFlags() & rpm::RPMINST_JUSTDB );
            flags |= rpm::RPMINST_NODEPS;
            if (policy_r.dryRun()) flags |= rpm::RPMINST_TEST;

            attemptToModify();
            try
            {
              rpm().removePackage( p, flags );
              HistoryLog().remove(citem);

              if ( progress.aborted() )
              {
                WAR << "commit aborted by the user" << endl;
                abort = true;
                step->stepStage( sat::Transaction::STEP_ERROR );
                break;
              }
              else
              {
                success = true;
                step->stepStage( sat::Transaction::STEP_DONE );
              }
            }
            catch (Exception & excpt_r)
            {
              ZYPP_CAUGHT( excpt_r );
              if ( progress.aborted() )
              {
                WAR << "commit aborted by the user" << endl;
                abort = true;
                step->stepStage( sat::Transaction::STEP_ERROR );
                break;
              }
              // else
              WAR << "removal of " << p << " failed";
              step->stepStage( sat::Transaction::STEP_ERROR );
            }
            if ( success && !policy_r.dryRun() )
            {
              citem.status().resetTransact( ResStatus::USER );
              step->stepStage( sat::Transaction::STEP_DONE );
            }
          }
        }
        else if ( ! policy_r.dryRun() ) // other resolvables (non-Package)
        {
          // Status is changed as the buddy package buddy
          // gets installed/deleted. Handle non-buddies only.
          if ( ! citem.buddy() )
          {
            if ( citem->isKind<Product>() )
            {
              Product::constPtr p = citem->asKind<Product>();
              if ( citem.status().isToBeInstalled() )
              {
                ERR << "Can't install orphan product without release-package! " << citem << endl;
              }
              else
              {
                // Deleting the corresponding product entry is all we con do.
                // So the product will no longer be visible as installed.
                std::string referenceFilename( p->referenceFilename() );
                if ( referenceFilename.empty() )
                {
                  ERR << "Can't remove orphan product without 'referenceFilename'! " << citem << endl;
                }
                else
                {
                  Pathname referencePath { Pathname("/etc/products.d") / referenceFilename };	// no root prefix for rpmdb lookup!
                  if ( ! rpm().hasFile( referencePath.asString() ) )
                  {
                    // If it's not owned by a package, we can delete it.
                    referencePath = Pathname::assertprefix( _root, referencePath );	// now add a root prefix
                    if ( filesystem::unlink( referencePath ) != 0 )
                      ERR << "Delete orphan product failed: " << referencePath << endl;
                  }
                  else
                  {
                    WAR << "Won't remove orphan product: '/etc/products.d/" << referenceFilename << "' is owned by a package." << endl;
                  }
                }
              }
            }
            else if ( citem->isKind<SrcPackage>() && citem.status().isToBeInstalled() )
            {
              // SrcPackage is install-only
              SrcPackage::constPtr p = citem->asKind<SrcPackage>();
              installSrcPackage( p );
            }

            citem.status().resetTransact( ResStatus::USER );
            step->stepStage( sat::Transaction::STEP_DONE );
          }

        }  // other resolvables

      } // for

      // process all remembered posttrans scripts. If aborting,
      // at least log omitted scripts.
      if ( abort || (abort = !postTransCollector.executeScripts()) )
        postTransCollector.discardScripts();

      // Check presence of update scripts/messages. If aborting,
      // at least log omitted scripts.
      if ( ! successfullyInstalledPackages.empty() )
      {
        if ( ! RunUpdateScripts( _root, ZConfig::instance().update_scriptsPath(),
                                 successfullyInstalledPackages, abort ) )
        {
          WAR << "Commit aborted by the user" << endl;
          abort = true;
        }
        // send messages after scripts in case some script generates output,
        // that should be kept in t %ghost message file.
        RunUpdateMessages( _root, ZConfig::instance().update_messagesPath(),
                           successfullyInstalledPackages,
                           result_r );
      }

      // jsc#SLE-5116: Log patch status changes to history
      // NOTE: Should be the last action as it may need to reload
      // the Target in case of an incomplete transaction.
      logPatchStatusChanges( result_r.transaction(), *this );

      if ( abort )
      {
        HistoryLog().comment( "Commit was aborted." );
        ZYPP_THROW( TargetAbortedException( ) );
      }
    }


    ///////////////////////////////////////////////////////////////////
    /// \brief Convenience SendReport<rpm::SingleTransReport> wrapper.
    ///
    /// Provide convenience methods to invoke a generic \ref report
    /// supplied with the proper \ref UserData and \ref ContentType,
    ///////////////////////////////////////////////////////////////////
    struct SendSingleTransReport : public callback::SendReport<rpm::SingleTransReport>
    {
      /** Convenience to send a \ref contentLogline. */
      void sendLogline( const std::string & line_r, ReportType::loglevel level_r = ReportType::loglevel::msg )
      {
        callback::UserData data { ReportType::contentLogline };
        data.set( "line", std::cref(line_r) );
        data.set( "level", level_r );
        report( data );
      }
      /** Convenience to send a \ref contentLogline translating a rpm loglevel. */
      void sendLoglineRpm( const std::string & line_r, unsigned rpmlevel_r )
      {
        auto u2rpmlevel = []( unsigned rpmlevel_r ) -> ReportType::loglevel {
          switch ( rpmlevel_r ) {
            case RPMLOG_EMERG:  [[fallthrough]];  // system is unusable
            case RPMLOG_ALERT:  [[fallthrough]];  // action must be taken immediately
            case RPMLOG_CRIT:                     // critical conditions
              return ReportType::loglevel::crt;
            case RPMLOG_ERR:                      // error conditions
              return ReportType::loglevel::err;
            case RPMLOG_WARNING:                  // warning conditions
              return ReportType::loglevel::war;
            default:            [[fallthrough]];
            case RPMLOG_NOTICE: [[fallthrough]];  // normal but significant condition
            case RPMLOG_INFO:                     // informational
              return ReportType::loglevel::msg;
            case RPMLOG_DEBUG:
              return ReportType::loglevel::dbg;
          }
        };
        sendLogline( line_r, u2rpmlevel( rpmlevel_r ) );
      }

    private:
      void report( const callback::UserData & userData_r )
      { (*this)->report( userData_r ); }
    };

    const callback::UserData::ContentType rpm::SingleTransReport::contentLogline { "zypp-rpm", "logline" };

    const callback::UserData::ContentType rpm::InstallResolvableReportSA::contentRpmout( "zypp-rpm","installpkgsa" );
    const callback::UserData::ContentType rpm::RemoveResolvableReportSA::contentRpmout( "zypp-rpm","removepkgsa" );
    const callback::UserData::ContentType rpm::CommitScriptReportSA::contentRpmout( "zypp-rpm","scriptsa" );
    const callback::UserData::ContentType rpm::TransactionReportSA::contentRpmout( "zypp-rpm","transactionsa" );
    const callback::UserData::ContentType rpm::CleanupPackageReportSA::contentRpmout( "zypp-rpm","cleanupkgsa" );

    void TargetImpl::commitInSingleTransaction(const ZYppCommitPolicy &policy_r, CommitPackageCache &packageCache_r, ZYppCommitResult &result_r)
    {
      namespace zpt = zypp::proto::target;

      SendSingleTransReport report; // active throughout the whole rpm transaction

      // steps: this is our todo-list
      ZYppCommitResult::TransactionStepList & steps( result_r.rTransactionStepList() );
      MIL << "TargetImpl::commit(<list>" << policy_r << ")" << steps.size() << endl;

      HistoryLog().stampCommand();

      // Send notification once upon calling rpm
      NotifyAttemptToModify attemptToModify( result_r );

      // let zypper know we executed in one big transaction so in case of failures it can show extended error information
      result_r.setSingleTransactionMode( true );

      // bsc#1181328: Some systemd tools require /proc to be mounted
      AssertProcMounted assertProcMounted( _root );

      // Why nodeps?
      //
      // Because zypp builds the transaction and the resolver asserts that
      // everything is fine, or the user decided to ignore problems.
      rpm::RpmInstFlags flags( policy_r.rpmInstFlags()
                               | rpm::RPMINST_NODEPS
                               // skip signature checks, we did that already
                               | rpm::RPMINST_NODIGEST
                               | rpm::RPMINST_NOSIGNATURE
                               // ignore untrusted keys since we already checked those earlier
                               | rpm::RPMINST_ALLOWUNTRUSTED );

      zpt::Commit commit;
      commit.set_flags( flags );
      commit.set_ignorearch( !ZConfig::instance().systemArchitecture().compatibleWith( ZConfig::instance().defaultSystemArchitecture() ) );
      commit.set_arch( ZConfig::instance().systemArchitecture().asString() );
      commit.set_dbpath( rpm().dbPath().asString() );
      commit.set_root( rpm().root().asString() );

      bool abort = false;
      zypp::AutoDispose<std::unordered_map<int, ManagedFile>> locCache([]( std::unordered_map<int, ManagedFile> &data ){
        for ( auto &[_, value] : data ) {
          (void)_; // unsused; for older g++ versions
          value.resetDispose();
        }
        data.clear();
      });

      // fill the transaction
      for ( int stepId = 0; (ZYppCommitResult::TransactionStepList::size_type)stepId < steps.size() && !abort ; ++stepId ) {
        auto &step = steps[stepId];
        PoolItem citem( step );
        if ( step.stepType() == sat::Transaction::TRANSACTION_IGNORE ) {
          if ( citem->isKind<Package>() )
          {
            // for packages this means being obsoleted (by rpm)
            // thius no additional action is needed.
            step.stepStage( sat::Transaction::STEP_DONE );
            continue;
          }
        }

        if ( citem->isKind<Package>() ) {
          Package::constPtr p = citem->asKind<Package>();
          if ( citem.status().isToBeInstalled() )
          {
            try {
              locCache.value()[stepId] = packageCache_r.get( citem );

              zpt::TransactionStep tStep;
              tStep.set_stepid( stepId );
              tStep.mutable_install()->set_pathname( locCache.value()[stepId]->asString() );
              tStep.mutable_install()->set_multiversion( p->multiversionInstall() );

              *commit.mutable_steps()->Add( ) = std::move(tStep);
            }
            catch ( const AbortRequestException &e )
            {
              WAR << "commit aborted by the user" << endl;
              abort = true;
              step.stepStage( sat::Transaction::STEP_ERROR );
              break;
            }
            catch ( const SkipRequestException &e )
            {
              ZYPP_CAUGHT( e );
              WAR << "Skipping package " << p << " in commit" << endl;
              step.stepStage( sat::Transaction::STEP_ERROR );
              continue;
            }
            catch ( const Exception &e )
            {
              // bnc #395704: missing catch causes abort.
              // TODO see if packageCache fails to handle errors correctly.
              ZYPP_CAUGHT( e );
              INT << "Unexpected Error: Skipping package " << p << " in commit" << endl;
              step.stepStage( sat::Transaction::STEP_ERROR );
              continue;
            }
          } else {

            zpt::TransactionStep tStep;
            tStep.set_stepid( stepId );
            tStep.mutable_remove()->set_name( p->name() );
            tStep.mutable_remove()->set_version( p->edition().version() );
            tStep.mutable_remove()->set_release( p->edition().release() );
            tStep.mutable_remove()->set_arch( p->arch().asString() );

            *commit.mutable_steps()->Add() = std::move(tStep);

          }
        } else if ( citem->isKind<SrcPackage>() && citem.status().isToBeInstalled() ) {
          // SrcPackage is install-only
          SrcPackage::constPtr p = citem->asKind<SrcPackage>();

          try {
            // provide on local disk
            locCache.value()[stepId] = provideSrcPackage( p );

            zpt::TransactionStep tStep;
            tStep.set_stepid( stepId );
            tStep.mutable_install()->set_pathname( locCache.value()[stepId]->asString() );
            tStep.mutable_install()->set_multiversion( false );
            *commit.mutable_steps()->Add() = std::move(tStep);

          }  catch ( const Exception &e ) {
            ZYPP_CAUGHT( e );
            INT << "Unexpected Error: Skipping package " << p << " in commit" << endl;
            step.stepStage( sat::Transaction::STEP_ERROR );
            continue;
          }
        }
      }

      std::vector<sat::Solvable> successfullyInstalledPackages;

      if ( commit.steps_size() ) {

        // create the event loop early
        auto loop = zyppng::EventLoop::create();

        attemptToModify();


        // transaction related variables:
        //
        // the index of the step in the transaction list that we currenty execute.
        // this can be -1
        int currentStepId = -1;

        // sync flag, every time zypp-rpm finishes executing a step it writes a tag into
        // the script fd, once we receive it we set this flag to true and ignore all output
        // that is written to the pipe ( aside from buffering it ) until we finalize the current report
        // and start a new one
        bool gotEndOfScript = false;

        // the possible reports we emit during the transaction
        std::unique_ptr<callback::SendReport <rpm::TransactionReportSA>>        transactionreport;
        std::unique_ptr<callback::SendReport <rpm::InstallResolvableReportSA>>  installreport;
        std::unique_ptr<callback::SendReport <rpm::RemoveResolvableReportSA>>   uninstallreport;
        std::unique_ptr<callback::SendReport <rpm::CommitScriptReportSA>>       scriptreport;
        std::unique_ptr<callback::SendReport <rpm::CleanupPackageReportSA>>     cleanupreport;

        // this will be set if we receive a transaction error description
        std::optional<zpt::TransactionError> transactionError;

        // infos about the currently executed script, empty if no script is currently executed
        std::string currentScriptType;
        std::string currentScriptPackage;

        // buffer to collect rpm output per report, this will be written to the log once the
        // report ends
        std::string rpmmsg;

        // maximum number of lines that we are buffering in rpmmsg
        constexpr auto MAXRPMMESSAGELINES = 10000;

        // current number of lines in the rpmmsg line buffer. This is capped to MAXRPMMESSAGELINES
        unsigned    lineno = 0;

        // the sources to communicate with zypp-rpm, we will associate pipes with them further down below
        auto msgSource = zyppng::AsyncDataSource::create();
        auto scriptSource = zyppng::AsyncDataSource::create();


        // helper function that sends RPM output to the currently active report, writing a warning to the log
        // if there is none
        const auto &sendRpmLineToReport = [&]( const std::string &line ){

          const auto &sendLogRep = [&]( auto &report, const auto &cType ){
            callback::UserData cmdout(cType);
            if ( currentStepId >= 0 )
              cmdout.set( "solvable", steps.at(currentStepId).satSolvable() );
            cmdout.set( "line", line );
            report->report(cmdout);
          };

          if ( installreport ) {
            sendLogRep( (*installreport), rpm::InstallResolvableReportSA::contentRpmout );
          } else if ( uninstallreport ) {
            sendLogRep( (*uninstallreport), rpm::RemoveResolvableReportSA::contentRpmout );
          } else if ( scriptreport ) {
            sendLogRep( (*scriptreport), rpm::CommitScriptReportSA::contentRpmout );
          } else if ( transactionreport ) {
            sendLogRep( (*transactionreport), rpm::TransactionReportSA::contentRpmout );
          } else if ( cleanupreport ) {
            sendLogRep( (*cleanupreport), rpm::CleanupPackageReportSA::contentRpmout );
          } else {
            WAR << "Got rpm output without active report " << line; // no endl! - readLine does not trim
          }

          // remember rpm output
          if ( lineno >= MAXRPMMESSAGELINES ) {
            if ( line.find( " scriptlet failed, " ) == std::string::npos )	// always log %script errors
              return;
          }
          rpmmsg += line;
          if ( line.back() != '\n' )
            rpmmsg += '\n';
        };


        // callback and helper function to process data that is received on the script FD
        const auto &processDataFromScriptFd = [&](){

          while ( scriptSource->canReadLine() ) {

            if ( gotEndOfScript )
              return;

            std::string l = scriptSource->readLine().asString();
            if( str::endsWith( l, endOfScriptTag ) ) {
              gotEndOfScript = true;
              std::string::size_type rawsize { l.size() - endOfScriptTag.size() };
              if ( not rawsize )
                return;
              l = l.substr( 0, rawsize );
            }
            L_DBG("zypp-rpm") << "[rpm> " << l; // no endl! - readLine does not trim
            sendRpmLineToReport( l );
          }
        };
        scriptSource->sigReadyRead().connect( processDataFromScriptFd );

        // helper function that just waits until the end of script tag was received on the scriptSource
        const auto &waitForScriptEnd = [&]() {

          // nothing to wait for
          if ( gotEndOfScript )
            return;

          // we process all available data
          processDataFromScriptFd();

          // end of script is always sent by zypp-rpm, we need to wait for it to keep order
          while ( scriptSource->canRead() && !gotEndOfScript ) {
            // readyRead will trigger processDataFromScriptFd so no need to call it again
            // we still got nothing, lets wait for more
            scriptSource->waitForReadyRead( 100 );
          }
        };

        const auto &aboutToStartNewReport = [&](){

          if ( transactionreport || installreport || uninstallreport || scriptreport || cleanupreport ) {
            ERR << "There is still a running report, this is a bug" << std::endl;
            assert(false);
          }

          gotEndOfScript = false;
        };

        const auto &writeRpmMsgToHistory = [&](){
          if ( rpmmsg.size() == 0 )
            return;

          if ( lineno >= MAXRPMMESSAGELINES )
            rpmmsg += "[truncated]\n";

          std::ostringstream sstr;
          sstr << "rpm output:" << endl << rpmmsg << endl;
          HistoryLog().comment(sstr.str());
        };

        // helper function that closes the current report and cleans up the ressources
        const auto &finalizeCurrentReport = [&]() {
          sat::Transaction::Step *step = nullptr;
          Resolvable::constPtr resObj;
          if ( currentStepId >= 0 ) {
            step = &steps.at(currentStepId);
            resObj = makeResObject( step->satSolvable() );
          }

          if ( installreport ) {
            waitForScriptEnd();
            if ( step->stepStage() == sat::Transaction::STEP_ERROR ) {

              HistoryLog().comment(
                str::form("%s install failed", step->ident().c_str()),
                true /*timestamp*/);

              writeRpmMsgToHistory();

              ( *installreport)->finish( resObj, rpm::InstallResolvableReportSA::INVALID );
            } else {
              ( *installreport)->progress( 100, resObj );
              ( *installreport)->finish( resObj, rpm::InstallResolvableReportSA::NO_ERROR );

              if ( currentStepId >= 0 )
                locCache.value().erase( currentStepId );
              successfullyInstalledPackages.push_back( step->satSolvable() );

              PoolItem citem( *step );
              if ( !( flags & rpm::RPMINST_TEST ) ) {
                // @TODO are we really doing this just for install?
                if ( citem.isNeedreboot() ) {
                  auto rebootNeededFile = root() / "/run/reboot-needed";
                  if ( filesystem::assert_file( rebootNeededFile ) == EEXIST)
                    filesystem::touch( rebootNeededFile );
                }
                citem.status().resetTransact( ResStatus::USER );
                HistoryLog().install(citem);
              }

              HistoryLog().comment(
                str::form("%s installed ok", step->ident().c_str()),
                true /*timestamp*/);

              writeRpmMsgToHistory();
            }
          }
          if ( uninstallreport ) {
            waitForScriptEnd();
            if ( step->stepStage() == sat::Transaction::STEP_ERROR ) {

              HistoryLog().comment(
                str::form("%s uninstall failed", step->ident().c_str()),
                true /*timestamp*/);

              writeRpmMsgToHistory();

              ( *uninstallreport)->finish( resObj, rpm::RemoveResolvableReportSA::INVALID );
            } else {
              ( *uninstallreport)->progress( 100, resObj );
              ( *uninstallreport)->finish( resObj, rpm::RemoveResolvableReportSA::NO_ERROR );

              PoolItem citem( *step );
              HistoryLog().remove(citem);

              HistoryLog().comment(
                str::form("%s removed ok", step->ident().c_str()),
                true /*timestamp*/);

              writeRpmMsgToHistory();
            }
          }
          if ( scriptreport ) {
            waitForScriptEnd();
            ( *scriptreport)->progress( 100, resObj );
            ( *scriptreport)->finish( resObj, rpm::CommitScriptReportSA::NO_ERROR );
          }
          if ( transactionreport ) {
            waitForScriptEnd();
            ( *transactionreport)->progress( 100 );
            ( *transactionreport)->finish( rpm::TransactionReportSA::NO_ERROR );
          }
          if ( cleanupreport ) {
            waitForScriptEnd();
            ( *cleanupreport)->progress( 100 );
            ( *cleanupreport)->finish( rpm::CleanupPackageReportSA::NO_ERROR );
          }
          currentStepId = -1;
          lineno = 0;
          rpmmsg.clear();
          currentScriptType.clear();
          currentScriptPackage.clear();
          installreport.reset();
          uninstallreport.reset();
          scriptreport.reset();
          transactionreport.reset();
          cleanupreport.reset();
        };

        // This sets up the process and pushes the required transactions steps to it
        // careful when changing code here, zypp-rpm relies on the exact order data is transferred:
        //
        // 1) Size of the commit message , sizeof(zyppng::rpc::HeaderSizeType)
        // 2) The Commit Proto message, directly serialized to the FD, without Envelope
        // 3) 2 writeable FDs that are set up by the parent Process when forking. The first FD is to be used for message sending, the second one for script output

        constexpr std::string_view zyppRpmBinary(ZYPP_RPM_BINARY);

        const char *argv[] = {
          //"gdbserver",
          //"localhost:10001",
          zyppRpmBinary.data(),
          nullptr
        };
        auto prog = zyppng::Process::create();

        // we set up a pipe to communicate with the process, it is too dangerous to use stdout since librpm
        // might print to it.
        auto messagePipe = zyppng::Pipe::create();
        if ( !messagePipe )
          ZYPP_THROW( target::rpm::RpmSubprocessException( "Failed to create message pipe" ) );

        // open a pipe that we are going to use to receive script output, this is a librpm feature, there is no other
        // way than a FD to redirect that output
        auto scriptPipe = zyppng::Pipe::create();
        if ( !scriptPipe )
          ZYPP_THROW( target::rpm::RpmSubprocessException( "Failed to create scriptfd" ) );

        prog->addFd( messagePipe->writeFd );
        prog->addFd( scriptPipe->writeFd );

        // set up the AsyncDataSource to read script output
        if ( !scriptSource->open( scriptPipe->readFd ) )
          ZYPP_THROW( target::rpm::RpmSubprocessException( "Failed to open scriptFD to subprocess" ) );

        prog->sigStarted().connect( [&](){

          // close the ends of the pipes we do not care about
          messagePipe->unrefWrite();
          scriptPipe->unrefWrite();

          // read the stdout and forward it to our log
          prog->stdoutDevice()->connectFunc( &zyppng::IODevice::sigReadyRead, [&](){
            while( prog->stdoutDevice()->canReadLine() ) {
              L_DBG("zypp-rpm") << "<stdout> " << prog->stdoutDevice()->readLine().asStringView(); // no endl! - readLine does not trim
            }
          });

          // read the stderr and forward it to our log
          prog->stderrDevice()->connectFunc( &zyppng::IODevice::sigReadyRead, [&](){
            while( prog->stderrDevice()->canReadLine() ) {
              L_ERR("zypp-rpm") << "<stderr> " << prog->stderrDevice()->readLine().asStringView(); // no endl! - readLine does not trim
            }
          });

          {
            // write the commit message in blocking mode
            const auto outFd = prog->stdinFd();
            OnScopeExit unblock([&](){
              io::setFDBlocking( outFd, false );
            });
            io::setFDBlocking( outFd );

            // first we push the commit information to the process, starting with the byte size
            zyppng::rpc::HeaderSizeType msgSize = commit.ByteSizeLong();
            const auto written = zyppng::eintrSafeCall( ::write, outFd, &msgSize, sizeof(zyppng::rpc::HeaderSizeType) );
            if ( written != sizeof(zyppng::rpc::HeaderSizeType) ) {
              prog->stop( SIGKILL );
              ZYPP_THROW( target::rpm::RpmSubprocessException( "Failed to write commit size to subprocess" ) );
            }

            zyppng::FileOutputStream fo ( outFd );
            if ( !commit.SerializeToZeroCopyStream( &fo ) ) {
              prog->stop( SIGKILL );
              ZYPP_THROW( target::rpm::RpmSubprocessException( "Failed to write commit to subprocess" ) );
            }
            fo.Flush();
          }

        });

        // this is the source for control messages from zypp-rpm , we will get structured data information
        // in form of protobuf messages
        if ( !msgSource->open( messagePipe->readFd ) )
          ZYPP_THROW( target::rpm::RpmSubprocessException( "Failed to open read stream to subprocess" ) );

        zyppng::rpc::HeaderSizeType pendingMessageSize = 0;
        const auto &processMessages = [&] ( ) {

          // lambda function that parses the passed message type and checks if the stepId is a valid offset
          // in the steps list.
          const auto &parseMsgWithStepId = [&steps]( const auto &m, auto &p ){
            if ( !p.ParseFromString( m.value() ) ) {
              ERR << "Failed to parse " << m.messagetypename() << " message from zypp-rpm." << std::endl;
              return false;
            }

            auto id = p.stepid();
            if ( id < 0 || id >= steps.size() ) {
              ERR << "Received invalid stepId: " << id << " in " << m.messagetypename() << " message from zypp-rpm, ignoring." << std::endl;
              return false;
            }
            return true;
          };

          while ( msgSource->bytesAvailable() ) {

            if ( pendingMessageSize == 0 ) {
              if ( msgSource->bytesAvailable() >= sizeof( zyppng::rpc::HeaderSizeType ) ) {
                msgSource->read( reinterpret_cast<char *>( &pendingMessageSize ),  sizeof( zyppng::rpc::HeaderSizeType ) );
              }
            }

            if ( msgSource->bytesAvailable() < pendingMessageSize ) {
              return;
            }

            auto bytes = msgSource->read( pendingMessageSize );
            pendingMessageSize = 0;

            zypp::proto::Envelope m;
            if (! m.ParseFromArray( bytes.data(), bytes.size() ) ) {
              // if we get a misformed message we can not simply kill zypp-rpm since it might be executing the transaction, all we can do is
              // continue ( this should normally not happen , but code needs to handle it ).
              ERR << "Received misformed message from zypp-rpm, ignoring" << std::endl;
              return;
            }

            // due to librpm behaviour we need to make sense of the order of messages we receive
            // because we first get a PackageFinished BEFORE getting a PackageError, same applies to
            // Script related messages. What we do is remember the current step we are in and only close
            // the step when we get the start of the next one
            const auto &mName = m.messagetypename();
            if (  mName == "zypp.proto.target.RpmLog" )  {

              zpt::RpmLog p;
              if ( !p.ParseFromString( m.value() ) ) {
                ERR << "Failed to parse " << m.messagetypename() << " message from zypp-rpm." << std::endl;
                continue;
              }
              ( p.level() >= RPMLOG_ERR     ? L_ERR("zypp-rpm")
              : p.level() >= RPMLOG_WARNING ? L_WAR("zypp-rpm")
              : L_DBG("zypp-rpm") ) << "[rpm " << p.level() << "> " << p.line(); // no endl! - readLine does not trim
              report.sendLoglineRpm( p.line(), p.level() );

            } else if (  mName == "zypp.proto.target.PackageBegin" )  {
              finalizeCurrentReport();

              zpt::PackageBegin p;
              if ( !parseMsgWithStepId( m, p ) )
                continue;

              aboutToStartNewReport();

              auto & step = steps.at( p.stepid() );
              currentStepId = p.stepid();
              if ( step.stepType() == sat::Transaction::TRANSACTION_ERASE ) {
                uninstallreport = std::make_unique< callback::SendReport <rpm::RemoveResolvableReportSA> > ();
                ( *uninstallreport )->start( makeResObject( step.satSolvable() ) );
              } else {
                installreport = std::make_unique< callback::SendReport <rpm::InstallResolvableReportSA> > ();
                ( *installreport )->start( makeResObject( step.satSolvable() ) );
              }

            } else if (  mName == "zypp.proto.target.PackageFinished" )  {
              zpt::PackageFinished p;
              if ( !parseMsgWithStepId( m, p ) )
                continue;

              if ( p.stepid() < 0 || p.stepid() > steps.size() )
                continue;

              // here we only set the step stage to done, we however need to wait for the next start in order to send
              // the finished report since there might be a error pending to be reported
              steps[ p.stepid() ].stepStage( sat::Transaction::STEP_DONE );

            } else if (  mName == "zypp.proto.target.PackageProgress" )  {
              zpt::PackageProgress p;
              if ( !parseMsgWithStepId( m, p ) )
                continue;

              if ( uninstallreport )
                (*uninstallreport)->progress( p.amount(), makeResObject( steps.at( p.stepid() ) ));
              else if ( installreport )
                (*installreport)->progress( p.amount(), makeResObject( steps.at( p.stepid() ) ));
              else
                ERR << "Received a " << mName << " message but there is no corresponding report running." << std::endl;

            } else if (  mName == "zypp.proto.target.PackageError" )  {
              zpt::PackageError p;
              if ( !parseMsgWithStepId( m, p ) )
                continue;

              if ( p.stepid() >= 0 && p.stepid() < steps.size() )
                steps[ p.stepid() ].stepStage( sat::Transaction::STEP_ERROR );

              finalizeCurrentReport();

            } else if (  mName == "zypp.proto.target.ScriptBegin" )  {
              finalizeCurrentReport();

              zpt::ScriptBegin p;
              if ( !p.ParseFromString( m.value() ) ) {
                ERR << "Failed to parse " << m.messagetypename() << " message from zypp-rpm." << std::endl;
                continue;
              }

              aboutToStartNewReport();

              Resolvable::constPtr resPtr;
              const auto stepId = p.stepid();
              if ( stepId >= 0 && stepId < steps.size() ) {
                resPtr = makeResObject( steps.at(stepId).satSolvable() );
              }

              currentStepId = p.stepid();
              scriptreport = std::make_unique< callback::SendReport <rpm::CommitScriptReportSA> > ();
              currentScriptType = p.scripttype();
              currentScriptPackage = p.scriptpackage();
              (*scriptreport)->start( p.scripttype(), p.scriptpackage(), resPtr );

            } else if (  mName == "zypp.proto.target.ScriptFinished" )  {

              // we just read the message, we do not act on it because a ScriptError is reported after ScriptFinished

            } else if (  mName == "zypp.proto.target.ScriptError" )  {

              zpt::ScriptError p;
              if ( !p.ParseFromString( m.value() ) ) {
                ERR << "Failed to parse " << m.messagetypename() << " message from zypp-rpm." << std::endl;
                continue;
              }

              Resolvable::constPtr resPtr;
              const auto stepId = p.stepid();
              if ( stepId >= 0 && stepId < steps.size() ) {
                resPtr = makeResObject( steps.at(stepId).satSolvable() );

                if ( p.fatal() ) {
                  steps.at( stepId ).stepStage( sat::Transaction::STEP_ERROR );
                }

              }

              HistoryLog().comment(
                str::form("Failed to execute %s script for %s ", currentScriptType.c_str(), currentScriptPackage.size() ? currentScriptPackage.c_str() : "unknown" ),
                true /*timestamp*/);

              writeRpmMsgToHistory();

              if ( !scriptreport ) {
                ERR << "Received a ScriptError message, but there is no running report. " << std::endl;
                continue;
              }

              // before killing the report we need to wait for the script end tag
              waitForScriptEnd();
              (*scriptreport)->finish( resPtr, p.fatal() ? rpm::CommitScriptReportSA::CRITICAL : rpm::CommitScriptReportSA::WARN );

              // manually reset the current report since we already sent the finish(), rest will be reset by the new start
              scriptreport.reset();
              currentStepId = -1;

            } else if (  mName == "zypp.proto.target.CleanupBegin" ) {
              finalizeCurrentReport();

              zpt::CleanupBegin beg;
              if ( !beg.ParseFromString( m.value() ) ) {
                ERR << "Failed to parse " << m.messagetypename() << " message from zypp-rpm." << std::endl;
                continue;
              }

              aboutToStartNewReport();
              cleanupreport = std::make_unique< callback::SendReport <rpm::CleanupPackageReportSA> > ();
              (*cleanupreport)->start( beg.nvra() );
            } else if (  mName == "zypp.proto.target.CleanupFinished" )  {

              finalizeCurrentReport();

            } else if (  mName == "zypp.proto.target.CleanupProgress" )  {
              zpt::CleanupProgress prog;
              if ( !prog.ParseFromString( m.value() ) ) {
                ERR << "Failed to parse " << m.messagetypename() << " message from zypp-rpm." << std::endl;
                continue;
              }

              if ( !cleanupreport ) {
                ERR << "Received a CleanupProgress message, but there is no running report. " << std::endl;
                continue;
              }

              (*cleanupreport)->progress( prog.amount() );

            } else if (  mName == "zypp.proto.target.TransBegin" ) {
              finalizeCurrentReport();

              zpt::TransBegin beg;
              if ( !beg.ParseFromString( m.value() ) ) {
                ERR << "Failed to parse " << m.messagetypename() << " message from zypp-rpm." << std::endl;
                continue;
              }

              aboutToStartNewReport();
              transactionreport = std::make_unique< callback::SendReport <rpm::TransactionReportSA> > ();
              (*transactionreport)->start( beg.name() );
            } else if (  mName == "zypp.proto.target.TransFinished" )  {

              finalizeCurrentReport();

            } else if (  mName == "zypp.proto.target.TransProgress" )  {
              zpt::TransProgress prog;
              if ( !prog.ParseFromString( m.value() ) ) {
                ERR << "Failed to parse " << m.messagetypename() << " message from zypp-rpm." << std::endl;
                continue;
              }

              if ( !transactionreport ) {
                ERR << "Received a TransactionProgress message, but there is no running report. " << std::endl;
                continue;
              }

              (*transactionreport)->progress( prog.amount() );
            } else if ( mName == "zypp.proto.target.TransactionError" ) {

              zpt::TransactionError error;
              if ( !error.ParseFromString( m.value() ) ) {
                ERR << "Failed to parse " << m.messagetypename() << " message from zypp-rpm." << std::endl;
                continue;
              }

              // this value is checked later
              transactionError = std::move(error);

            } else {
              ERR << "Received unexpected message from zypp-rpm: "<< m.messagetypename() << ", ignoring" << std::endl;
              return;
            }

          }
        };
        msgSource->connectFunc( &zyppng::AsyncDataSource::sigReadyRead, processMessages );

        // track the childs lifetime
        int zyppRpmExitCode = -1;
        prog->connectFunc( &zyppng::Process::sigFinished, [&]( int code ){
          zyppRpmExitCode = code;
          loop->quit();
        });

        if ( !prog->start( argv ) ) {
          HistoryLog().comment( "Commit was aborted, failed to run zypp-rpm" );
          ZYPP_THROW( target::rpm::RpmSubprocessException( prog->execError() ) );
        }

        loop->run();

        // make sure to read ALL available messages
        processMessages();

        // we will not receive a new start message , so we need to manually finalize the last report
        finalizeCurrentReport();

        // make sure to read all data from the log source
        bool readMsgs = false;
        while( prog->stderrDevice()->canReadLine() ) {
          readMsgs = true;
          MIL << "zypp-rpm: " << prog->stderrDevice()->readLine().asStringView();
        }
        while( prog->stdoutDevice()->canReadLine() ) {
          readMsgs = true;
          MIL << "zypp-rpm: " << prog->stdoutDevice()->readLine().asStringView();
        }

        while ( scriptSource->canReadLine() ) {
          readMsgs = true;
          MIL << "rpm-script-fd: " << scriptSource->readLine().asStringView();
        }
        if ( scriptSource->bytesAvailable() > 0 ) {
          readMsgs = true;
          MIL << "rpm-script-fd: " << scriptSource->readAll().asStringView();
        }
        if ( readMsgs )
          MIL << std::endl;

        switch ( zyppRpmExitCode ) {
          // we need to look at the summary, handle finishedwitherrors like no error here
          case zypprpm::NoError:
          case zypprpm::RpmFinishedWithError:
            break;
          case zypprpm::RpmFinishedWithTransactionError: {
            // here zypp-rpm sent us a error description
            if ( transactionError ) {

              std::ostringstream sstr;
              sstr << _("Executing the transaction failed because of the following problems:") << "\n";
              for ( const auto & err : transactionError->problems() ) {
                sstr << "    " << err.message() << "\n";
              }
              sstr << std::endl;
              ZYPP_THROW( rpm::RpmTransactionFailedException( sstr.str() ) );

            } else {
              ZYPP_THROW( rpm::RpmTransactionFailedException("RPM failed with a unexpected error, check the logs for more information.") );
            }
            break;
          }
          case zypprpm::FailedToOpenDb:
            ZYPP_THROW( rpm::RpmDbOpenException( rpm().root(), rpm().dbPath() ) );
            break;
          case zypprpm::WrongHeaderSize:
          case zypprpm::WrongMessageFormat:
            ZYPP_THROW( rpm::RpmSubprocessException("Failed to communicate with zypp-rpm, this is most likely a bug. Consider to fall back to legacy transaction strategy.") );
            break;
          case zypprpm::RpmInitFailed:
            ZYPP_THROW( rpm::RpmInitException( rpm().root(), rpm().dbPath() ) );
            break;
          case zypprpm::FailedToReadPackage:
            ZYPP_THROW( rpm::RpmSubprocessException("zypp-rpm was unable to read a package, check the logs for more information.") );
            break;
          case zypprpm::FailedToAddStepToTransaction:
            ZYPP_THROW( rpm::RpmSubprocessException("zypp-rpm failed to build the transaction, check the logs for more information.") );
            break;
          case zypprpm::RpmOrderFailed:
            ZYPP_THROW( rpm::RpmSubprocessException("zypp-rpm failed to order the transaction, check the logs for more information.") );
            break;
        }

        for ( int stepId = 0; (ZYppCommitResult::TransactionStepList::size_type)stepId < steps.size() && !abort; ++stepId ) {
          auto &step = steps[stepId];
          PoolItem citem( step );

          if ( step.stepStage() == sat::Transaction::STEP_TODO ) {
            // other resolvables (non-Package) that are not handled by zypp-rpm
            if ( !citem->isKind<Package>() && !policy_r.dryRun() ) {
              // Status is changed as the buddy package buddy
              // gets installed/deleted. Handle non-buddies only.
              if ( ! citem.buddy() && citem->isKind<Product>() ) {
                Product::constPtr p = citem->asKind<Product>();

                if ( citem.status().isToBeInstalled() ) {
                  ERR << "Can't install orphan product without release-package! " << citem << endl;
                } else {
                  // Deleting the corresponding product entry is all we con do.
                  // So the product will no longer be visible as installed.
                  std::string referenceFilename( p->referenceFilename() );

                  if ( referenceFilename.empty() ) {
                    ERR << "Can't remove orphan product without 'referenceFilename'! " << citem << endl;
                  } else {
                    Pathname referencePath { Pathname("/etc/products.d") / referenceFilename };	// no root prefix for rpmdb lookup!

                    if ( ! rpm().hasFile( referencePath.asString() ) ) {
                      // If it's not owned by a package, we can delete it.
                      referencePath = Pathname::assertprefix( _root, referencePath );	// now add a root prefix
                      if ( filesystem::unlink( referencePath ) != 0 )
                        ERR << "Delete orphan product failed: " << referencePath << endl;
                    } else {
                      WAR << "Won't remove orphan product: '/etc/products.d/" << referenceFilename << "' is owned by a package." << endl;
                    }
                  }
                }
                citem.status().resetTransact( ResStatus::USER );
                step.stepStage( sat::Transaction::STEP_DONE );
              }
            }
          }
        }
      }

      // Check presence of update scripts/messages. If aborting,
      // at least log omitted scripts.
      if ( ! successfullyInstalledPackages.empty() )
      {
        if ( ! RunUpdateScripts( _root, ZConfig::instance().update_scriptsPath(),
               successfullyInstalledPackages, abort ) )
        {
          WAR << "Commit aborted by the user" << endl;
          abort = true;
        }
        // send messages after scripts in case some script generates output,
        // that should be kept in t %ghost message file.
        RunUpdateMessages( _root, ZConfig::instance().update_messagesPath(),
          successfullyInstalledPackages,
          result_r );
      }

      // jsc#SLE-5116: Log patch status changes to history
      // NOTE: Should be the last action as it may need to reload
      // the Target in case of an incomplete transaction.
      logPatchStatusChanges( result_r.transaction(), *this );

      if ( abort ) {
        HistoryLog().comment( "Commit was aborted." );
        ZYPP_THROW( TargetAbortedException( ) );
      }
    }

    ///////////////////////////////////////////////////////////////////

    rpm::RpmDb & TargetImpl::rpm()
    {
      return _rpm;
    }

    bool TargetImpl::providesFile (const std::string & path_str, const std::string & name_str) const
    {
      return _rpm.hasFile(path_str, name_str);
    }

    ///////////////////////////////////////////////////////////////////
    namespace
    {
      parser::ProductFileData baseproductdata( const Pathname & root_r )
      {
        parser::ProductFileData ret;
        PathInfo baseproduct( Pathname::assertprefix( root_r, "/etc/products.d/baseproduct" ) );

        if ( baseproduct.isFile() )
        {
          try
          {
            ret = parser::ProductFileReader::scanFile( baseproduct.path() );
          }
          catch ( const Exception & excpt )
          {
            ZYPP_CAUGHT( excpt );
          }
        }
        else if ( PathInfo( Pathname::assertprefix( root_r, "/etc/products.d" ) ).isDir() )
        {
          ERR << "baseproduct symlink is dangling or missing: " << baseproduct << endl;
        }
        return ret;
      }

      inline Pathname staticGuessRoot( const Pathname & root_r )
      {
        if ( root_r.empty() )
        {
          // empty root: use existing Target or assume "/"
          Pathname ret ( ZConfig::instance().systemRoot() );
          if ( ret.empty() )
            return Pathname("/");
          return ret;
        }
        return root_r;
      }

      inline std::string firstNonEmptyLineIn( const Pathname & file_r )
      {
        std::ifstream idfile( file_r.c_str() );
        for( iostr::EachLine in( idfile ); in; in.next() )
        {
          std::string line( str::trim( *in ) );
          if ( ! line.empty() )
            return line;
        }
        return std::string();
      }
    } // namespace
    ///////////////////////////////////////////////////////////////////

    Product::constPtr TargetImpl::baseProduct() const
    {
      ResPool pool(ResPool::instance());
      for_( it, pool.byKindBegin<Product>(), pool.byKindEnd<Product>() )
      {
        Product::constPtr p = (*it)->asKind<Product>();
        if ( p->isTargetDistribution() )
          return p;
      }
      return nullptr;
    }

    LocaleSet TargetImpl::requestedLocales( const Pathname & root_r )
    {
      const Pathname needroot( staticGuessRoot(root_r) );
      const Target_constPtr target( getZYpp()->getTarget() );
      if ( target && target->root() == needroot )
        return target->requestedLocales();
      return RequestedLocalesFile( home(needroot) / "RequestedLocales" ).locales();
    }

    void TargetImpl::updateAutoInstalled()
    {
      MIL << "updateAutoInstalled if changed..." << endl;
      SolvIdentFile::Data newdata;
      for ( auto id : sat::Pool::instance().autoInstalled() )
        newdata.insert( IdString(id) );	// explicit ctor!
      _autoInstalledFile.setData( std::move(newdata) );
    }

    std::string TargetImpl::targetDistribution() const
    { return baseproductdata( _root ).registerTarget(); }
    // static version:
    std::string TargetImpl::targetDistribution( const Pathname & root_r )
    { return baseproductdata( staticGuessRoot(root_r) ).registerTarget(); }

    std::string TargetImpl::targetDistributionRelease() const
    { return baseproductdata( _root ).registerRelease(); }
    // static version:
    std::string TargetImpl::targetDistributionRelease( const Pathname & root_r )
    { return baseproductdata( staticGuessRoot(root_r) ).registerRelease();}

    std::string TargetImpl::targetDistributionFlavor() const
    { return baseproductdata( _root ).registerFlavor(); }
    // static version:
    std::string TargetImpl::targetDistributionFlavor( const Pathname & root_r )
    { return baseproductdata( staticGuessRoot(root_r) ).registerFlavor();}

    Target::DistributionLabel TargetImpl::distributionLabel() const
    {
      Target::DistributionLabel ret;
      parser::ProductFileData pdata( baseproductdata( _root ) );
      ret.shortName = pdata.shortName();
      ret.summary = pdata.summary();
      return ret;
    }
    // static version:
    Target::DistributionLabel TargetImpl::distributionLabel( const Pathname & root_r )
    {
      Target::DistributionLabel ret;
      parser::ProductFileData pdata( baseproductdata( staticGuessRoot(root_r) ) );
      ret.shortName = pdata.shortName();
      ret.summary = pdata.summary();
      return ret;
    }

    std::string TargetImpl::distributionVersion() const
    {
      if ( _distributionVersion.empty() )
      {
        _distributionVersion = TargetImpl::distributionVersion(root());
        if ( !_distributionVersion.empty() )
          MIL << "Remember distributionVersion = '" << _distributionVersion << "'" << endl;
      }
      return _distributionVersion;
    }
    // static version
    std::string TargetImpl::distributionVersion( const Pathname & root_r )
    {
      std::string distributionVersion = baseproductdata( staticGuessRoot(root_r) ).edition().version();
      if ( distributionVersion.empty() )
      {
        // ...But the baseproduct method is not expected to work on RedHat derivatives.
        // On RHEL, Fedora and others the "product version" is determined by the first package
        // providing 'system-release'. This value is not hardcoded in YUM and can be configured
        // with the $distroverpkg variable.
        scoped_ptr<rpm::RpmDb> tmprpmdb;
        if ( ZConfig::instance().systemRoot() == Pathname() )
        {
          try
          {
              tmprpmdb.reset( new rpm::RpmDb );
              tmprpmdb->initDatabase( /*default ctor uses / but no additional keyring exports */ );
          }
          catch( ... )
          {
            return "";
          }
        }
        rpm::librpmDb::db_const_iterator it;
        if ( it.findByProvides( ZConfig::instance().distroverpkg() ) )
          distributionVersion = it->tag_version();
      }
      return distributionVersion;
    }


    std::string TargetImpl::distributionFlavor() const
    {
      return firstNonEmptyLineIn( home() / "LastDistributionFlavor" );
    }
    // static version:
    std::string TargetImpl::distributionFlavor( const Pathname & root_r )
    {
      return firstNonEmptyLineIn( staticGuessRoot(root_r) / "/var/lib/zypp/LastDistributionFlavor" );
    }

    ///////////////////////////////////////////////////////////////////
    namespace
    {
      std::string guessAnonymousUniqueId( const Pathname & root_r )
      {
        // bsc#1024741: Omit creating a new uid for chrooted systems (if it already has one, fine)
        std::string ret( firstNonEmptyLineIn( root_r / "/var/lib/zypp/AnonymousUniqueId" ) );
        if ( ret.empty() && root_r != "/" )
        {
          // if it has nonoe, use the outer systems one
          ret = firstNonEmptyLineIn( "/var/lib/zypp/AnonymousUniqueId" );
        }
        return ret;
      }
    }

    std::string TargetImpl::anonymousUniqueId() const
    {
      return guessAnonymousUniqueId( root() );
    }
    // static version:
    std::string TargetImpl::anonymousUniqueId( const Pathname & root_r )
    {
      return guessAnonymousUniqueId( staticGuessRoot(root_r) );
    }

    ///////////////////////////////////////////////////////////////////

    void TargetImpl::vendorAttr( VendorAttr vendorAttr_r )
    {
      MIL << "New VendorAttr: " << vendorAttr_r << endl;
      _vendorAttr = std::move(vendorAttr_r);
    }
    ///////////////////////////////////////////////////////////////////

    void TargetImpl::installSrcPackage( const SrcPackage_constPtr & srcPackage_r )
    {
      // provide on local disk
      ManagedFile localfile = provideSrcPackage(srcPackage_r);
      // create a installation progress report proxy
      RpmInstallPackageReceiver progress( srcPackage_r );
      progress.connect(); // disconnected on destruction.
      // install it
      rpm().installPackage ( localfile );
    }

    ManagedFile TargetImpl::provideSrcPackage( const SrcPackage_constPtr & srcPackage_r )
    {
      // provide on local disk
      repo::RepoMediaAccess access_r;
      repo::SrcPackageProvider prov( access_r );
      return prov.provideSrcPackage( srcPackage_r );
    }
    ////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
