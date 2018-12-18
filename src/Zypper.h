/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_H
#define ZYPPER_H

#include <string>
#include <vector>

#include <boost/utility/string_ref.hpp>

#include <zypp/base/Exception.h>
#include <zypp/base/NonCopyable.h>
#include <zypp/base/PtrTypes.h>
#include <zypp/base/Flags.h>
#include <zypp/TriBool.h>

#include <zypp/RepoInfo.h>
#include <zypp/RepoManager.h> // for RepoManagerOptions
#include <zypp/SrcPackage.h>
#include <zypp/TmpPath.h>

#include "Config.h"
#include "Command.h"
#include "utils/getopt.h"
#include "output/Out.h"

#include "commands/basecommand.h"

// As a matter of fact namespaces std, boost and zypp have overlapping
// symbols (e.g. shared_ptr). We default to the ones used in namespace zypp.
// Symbols from other namespaces should be used explicitly and not by using
// the whole namespace.
using namespace zypp;

// Convenience
using std::cout;
using std::cerr;
using std::endl;

struct Options;

/** directory for storing manually installed (zypper install foo.rpm) RPM files
 */
#define ZYPPER_RPM_CACHE_DIR "/var/cache/zypper/RPMS"

inline std::string dashdash( std::string optname_r )
{ return optname_r.insert( 0, "--" ); }

/**
 * \bug The RepoInfo lists kept herein may lack housekeeping data added by the
 * RepoManager. Consider using your own RepoInfos only for those not
 * maintained by RepoManager. (bnc #544432)
*/
struct RuntimeData
{
  RuntimeData()
  : show_media_progress_hack( false )
  , force_resolution( indeterminate )
  , solve_before_commit( true )
  , solve_with_update( false )
  , plain_patch_command( false )
  , commit_pkgs_total( 0 )
  , commit_pkg_current( 0 )
  , rpm_pkgs_total( 0 )
  , rpm_pkg_current( 0 )
  , seen_verify_hint( false )
  , action_rpm_download( false )
  , waiting_for_input( false )
  , entered_commit( false )
  , tmpdir( zypp::myTmpDir() / "zypper" )
  {
    filesystem::assert_dir( tmpdir );
  }

  std::list<RepoInfo> repos;
  std::list<RepoInfo> temporary_repos;		///< repos not visible to RepoManager/System
  std::set<std::string> plusContentRepos;
  /**
   * Used by requestMedia callback
   * \todo but now it uses label, remove this variable?
   */
  RepoInfo current_repo;

  std::set<SrcPackage::constPtr> srcpkgs_to_install;

  // hack to enable media progress reporting in the commit phase in normal
  // output level
  bool show_media_progress_hack;

  // Indicates an ongoing raw meta-data refresh.
  // If not empty call zypper.out().progress(
  //  "raw-refresh", raw_refresh_progress_label) in the media download
  // progress callback.
  //! \todo better way to do this would be to propagate the download progress
  // all the way up from the media back-end through fetcher and downloader
  // into the RepoManager::refreshMetadata(), so that we get a combined percentage
  std::string raw_refresh_progress_label;

  /** Used to override the command line option */
  TriBool force_resolution;

  /**
   * Set to <tt>false</tt> to avoid calling of the solver
   * in \ref solve_and_commit(). Needed after Resolver::doUpdate()
   */
  bool solve_before_commit;
  bool solve_with_update;	///< Include an 'update all packages' job; BEWARE: don't set it in the 1st solver run computing the PPP status
  bool plain_patch_command;	///< plain patch may update updatestack only; handle it in Summary and on --with-update

  unsigned commit_pkgs_total;
  unsigned commit_pkg_current;
  unsigned rpm_pkgs_total;
  unsigned rpm_pkg_current;

  bool seen_verify_hint;
  bool action_rpm_download;

  //! \todo move this to a separate Status struct
  bool waiting_for_input;
  bool entered_commit;	// bsc#946750 - give ZYPPER_EXIT_ERR_COMMIT priority over ZYPPER_EXIT_ON_SIGNAL

  //! Temporary directory for any use, e.g. for temporary repositories.
  Pathname tmpdir;
};

typedef shared_ptr<RepoManager> RepoManager_Ptr;

class Zypper : private base::NonCopyable
{
public:
  typedef std::vector<std::string>  ArgList;

  static Zypper & instance();

  int main( int argc, char ** argv );

  // setters & getters
  Out & out();

  Out *outputWriter ( );
  void setOutputWriter( Out * out );
  Config & config()				{ return _config; }

  //deprecated, global options were replaced by Config
  ZYPP_DEPRECATED const Config & globalOpts() const	{ return _config; }
  ZYPP_DEPRECATED Config & globalOptsNoConst()		{ return _config; }

  const ZypperCommand & command() const		{ return _command; }
  const std::string & commandHelp() const	{ return _command_help; }
  const ArgList & arguments() const		{ return _arguments; }
  RuntimeData & runtimeData()			{ return _rdata; }

  void initRepoManager()
  { _rm.reset( new RepoManager( _config.rm_options ) ); }

  RepoManager & repoManager()
  { if ( !_rm ) _rm.reset( new RepoManager( _config.rm_options ) ); return *_rm; }


  int exitCode() const				{ return _exitCode; }
  void setExitCode( int exit )			{
    WAR << "setExitCode " << exit << endl;
    _exitCode = exit;
  }

  int exitInfoCode() const			{ return _exitInfoCode; }
  void setExitInfoCode( int exit )		{
    WAR << "setExitInfoCode " << exit << endl;
    _exitInfoCode = exit;
  }

  bool runningShell() const			{ return _running_shell; }
  bool runningHelp() const			{ return _running_help; }

  unsigned exitRequested() const		{ return _exit_requested; }
  void requestExit( bool do_exit = true )	{ _exit_requested = do_exit ? 1 : 0; }
  void requestImmediateExit()			{ _exit_requested = 2; }

  /** Pending SigINT? Check at some frequently called place to avoid exiting from within the signal handler. */
  void immediateExitCheck()
  { if ( _exit_requested > 1 ) immediateExit( /* not called from within a sighandler */false ); }

  /** E.g. from SIGNINT handler (main.cc) */
  void immediateExit( bool fromWithinSigHandler_r = true )
  {
    extern bool sigExitOnce;	// main.c.
    if ( !sigExitOnce )
      return;	// no nested calls to Zypper::immediateExit
    sigExitOnce = false;

    WAR << "Immediate Exit requested (" << fromWithinSigHandler_r << ")." << endl;
    cleanup();
    if ( fromWithinSigHandler_r )
    {
      // _exit will call no dtor, so cleanup the worst mess...
      filesystem::recursive_rmdir( zypp::myTmpDir() );
      _exit( runtimeData().entered_commit ? ZYPPER_EXIT_ERR_COMMIT : ZYPPER_EXIT_ON_SIGNAL );
    }
    // else
    exit( runtimeData().entered_commit ? ZYPPER_EXIT_ERR_COMMIT : ZYPPER_EXIT_ON_SIGNAL );
  }

  int argc()					{ return _running_shell ? _sh_argc : _argc; }
  char ** argv()				{ return _running_shell ? _sh_argv : _argv; }

  void cleanup();
  void cleanupForSubcommand();

public:
  ~Zypper();

private:
  Zypper();

  void processGlobalOptions();
  void processCommandOptions();
  void commandShell();
  void shellCleanup();
  void safeDoCommand();
  void doCommand();

  void setCommand( const ZypperCommand & command )	{ _command = command; }
  void setRunningShell( bool value = true )		{ _running_shell = value; }
  void setRunningHelp( bool value = true )		{ _running_help = value; }

  void assertZYppPtrGod();
  
private:

  int     _argc;
  char ** _argv;

  Out * _out_ptr;
  Config _config;
  ZypperCommand _command;
  ArgList _arguments;
  std::string _command_help;

  int   _exitCode;
  int   _exitInfoCode;	// hack for exitcodes that don't abort but are reported if the main action succeeded (e.g. 106, 107)
  bool  _running_shell;
  bool  _running_help;
  unsigned  _exit_requested;

  RuntimeData _rdata;

  RepoManager_Ptr   _rm;

  int _sh_argc;
  char **_sh_argv;
};

void print_unknown_command_hint( Zypper & zypper );
void print_command_help_hint( Zypper & zypper );

class ExitRequestException : public Exception
{
public:
  ExitRequestException(const std::string & msg ) : Exception(msg) {}
  virtual ~ExitRequestException();
};

#endif /*ZYPPER_H*/

// Local Variables:
// c-basic-offset: 2
// End:
