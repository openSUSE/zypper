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
  /** Convenience to return properly casted _commandOptions. */
  template<class Opt_>
  shared_ptr<Opt_> commandOptionsAs() const
  { return dynamic_pointer_cast<Opt_>( _commandOptions ); }

  /** Convenience to return _commandOptions or default constructed Options. */
  template<class Opt_>
  shared_ptr<Opt_> commandOptionsOrDefaultAs() const
  {
    shared_ptr<Opt_> myopt = commandOptionsAs<Opt_>();
    if ( ! myopt )
      myopt.reset( new Opt_() );
    return myopt;
  }

private:
  /** Convenience to return command options for \c Opt_, either casted from _commandOptions or newly created.
   * Not for public use, only to init a new commands _commandOptions.
   */
  template<class Opt_>
  shared_ptr<Opt_> assertCommandOptions()
  {
    shared_ptr<Opt_> myopt( commandOptionsAs<Opt_>() );
    if ( ! myopt )
    {
      myopt.reset( new Opt_() );
      _commandOptions = myopt;
    }
    return myopt;
  }

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
  parsed_opts   _copts;
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

  /** Command specific options (see also _copts). */
  shared_ptr<Options>  _commandOptions;
};

void print_unknown_command_hint( Zypper & zypper );
void print_command_help_hint( Zypper & zypper );

///////////////////////////////////////////////////////////////////
/// \class Options
/// \brief Base class for command specific option values.
///
/// Option set for a specific command should be derived from \ref Options.
/// Place an instance in \ref Zypper and access it e.g. via
/// \ref Zypper::commandOptionsAs.
///
/// The \ref MixinOptions template may be used to build command
/// options combining multiple common option sets.
///
/// \see \ref MixinOptions
///////////////////////////////////////////////////////////////////
struct Options
{
  //Options() : _command( "" ) {}	// FIXME: DefaultCtor is actually undesired!
  Options( const ZypperCommand & command_r ) : _command( command_r ) {}
  virtual ~Options() {}

  /** The command. */
  const ZypperCommand & command() const
  { return _command; }

  /** The command name (optionally suffixed). */
  std::string commandName( const std::string & suffix_r = std::string() ) const
  { std::string ret( _command.asString() ); if ( ! suffix_r.empty() ) ret += suffix_r; return ret; }

  /** The command help text written to a stream. */
  virtual std::ostream & showHelpOn( std::ostream & out ) const	// FIXME: become pure virtual
  {
    out
      << _command << " ...?\n"
      << "This is just a placeholder for a commands help.\n"
      << "Please file a bug report if this text is displayed.\n"
      ;
    return out;
  }

  /** The command help as string. */
  std::string helpString() const
  { std::ostringstream str; showHelpOn( str ); return str.str(); }

  /** Show user help on command. */
  void showUserHelp( Zypper & zypper_r ) const
  { zypper_r.out().info( helpString(), Out::QUIET ); }	// always visible

private:
  ZypperCommand _command;	//< my command
};

///////////////////////////////////////////////////////////////////
/// \class OptionsMixin
/// \brief (Optional) common base class for options mixins.
///////////////////////////////////////////////////////////////////
struct OptionsMixin
{};

///////////////////////////////////////////////////////////////////
/// \class MixinOptions<TZypperCommand,Mixins...>
/// \brief Build command options combining multiple common option sets.
/// \code
///   struct CommonOptionsMixin : public OptionsMixin
///   { /* common option values */ };
///
///   struct ExoticOptionsMixin : public OptionsMixin
///   { /* exotic option values */ };
///
///   struct AOptions : public MixinOptions<ZypperCommand::A_e, CommonOptionsMixin>
///   {
///     /* common option values */
///     /* command A specific options */
///   };
///
///   struct BOptions : public MixinOptions<ZypperCommand::B_e, CommonOptionsMixin, ExoticOptionsMixin>
///   {
///     /* common option values */
///     /* more common option values */
///     /* command B specific options */
///   };
/// \endcode
/// \see \ref Options
///////////////////////////////////////////////////////////////////
template <const ZypperCommand& TZypperCommand, class... TMixins>
struct MixinOptions : public Options, public TMixins...
{
  MixinOptions() : Options( TZypperCommand ) {}
};

///////////////////////////////////////////////////////////////////
/// \class CommandBase<Derived,Options>
/// \brief Base class for command specific implementation classes.
///////////////////////////////////////////////////////////////////
template <class Derived_, class Options_>
struct CommandBase
{
  CommandBase( Zypper & zypper_r )
  : CommandBase( zypper_r, zypper_r.commandOptionsAs<Options_>() )
  {}

  CommandBase( Zypper & zypper_r, shared_ptr<Options_> options_r )
  : _zypper( zypper_r )
  , _options( options_r )
  {
    if ( ! _options )
    {
      _options.reset( new Options_() );
      MIL << commandName() << "( no options provided )" << endl;
    }
  }

  Options_ & options()			{ return *_options; }
  const Options_ & options() const	{ return *_options; }

 /** Command name (optionally suffixed). */
  std::string commandName( const std::string & suffix_r = std::string() ) const
  { return _options->commandName( std::move(suffix_r) ); }

  /** The Command help text written to a stream. */
  std::ostream & showHelpOn( std::ostream & out ) const
  { return _options->showHelpOn( out ); }

  /** Show user help on command. */
  void showHelp() const
  { _options->showHelp( _zypper ); }

  /** Run a command action.
   * \code
   *   void action()
   *   {
   *      throw( Out::Error( ZYPPER_EXIT_ERR_ZYPP, "error", "detail" ) );
   *      _zypper.setExitCode( ZYPPER_EXIT_ERR_ZYPP );
   *   }
   * \endcode
   * Thrown Out::Error and zypper are evaluated and reported to the user.
   * Other exceptions pass by.
   * \return zypper exitCode
   */
  int run( void (Derived_::*action_r)() = &Derived_::action )
  {
    MIL << "run: " << commandName() << " action " << action_r << endl;
    try
    {
      (self().*(action_r))();
    }
    catch ( const Out::Error & error_r )
    {
      error_r.report( _zypper );
    }
    return _zypper.exitCode();
  }
  /** Execute a command action (run + final "Done"/"Finished with error." message).
   * \return zypper exitCode
   */
  int execute( void (Derived_::*action_r)() = &Derived_::action )
  {
    run( action_r );
    // finished
    _zypper.out().gap();
    if ( _zypper.exitCode() != ZYPPER_EXIT_OK )
      _zypper.out().info( _options->commandName(": ")+MSG_WARNINGString(_("Finished with error.") ).str() );
    else
      _zypper.out().info( _options->commandName(": ")+_("Done.") );
    return _zypper.exitCode();
  }

protected:
  ~CommandBase() {}
  Zypper & 		_zypper;	//< my Zypper
  shared_ptr<Options_>	_options;	//< my Options
private:
  Derived_ &       self()       { return *static_cast<Derived_*>( this ); }
  const Derived_ & self() const { return *static_cast<const Derived_*>( this ); }
};
///////////////////////////////////////////////////////////////////

class ExitRequestException : public Exception
{
public:
  ExitRequestException(const std::string & msg ) : Exception(msg) {}
};

#endif /*ZYPPER_H*/

// Local Variables:
// c-basic-offset: 2
// End:
