/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>
#include <sstream>
#include <optional>

#include <zypp/ZYppFactory.h>
#include <zypp/base/Logger.h>
#include <zypp/TriBool.h>
#include <zypp/FileChecker.h>
#include <zypp/base/InputStream.h>
#include <zypp/base/IOStream.h>

#include <zypp/media/MediaException.h>
#include <zypp/misc/CheckAccessDeleted.h>

#include "misc.h"		// confirm_licenses
#include "repos.h"		// get_repo - used in dist_upgrade
#include "utils/misc.h"
#include "utils/prompt.h"	// Continue? and solver problem prompt
#include "utils/pager.h"	// to view the summary
#include "utils/messages.h"
#include "global-settings.h"
#include "CommitSummary.h"

#include "solve-commit.h"
#include "commands/needs-rebooting.h"

using namespace zypp;
extern ZYpp::Ptr God;

///////////////////////////////////////////////////////////////////
/// bsc#1183268: Patch reboot-needed flag overrules included packages.
///
/// Prevent creation of /run/reboot-needed by packages if patches
/// overruled them (i.e. claim no reboot needed).
///////////////////////////////////////////////////////////////////
class PatchRebootRulesWatchdog
{
  PatchRebootRulesWatchdog( const PatchRebootRulesWatchdog & ) = delete;
  PatchRebootRulesWatchdog & operator=( const PatchRebootRulesWatchdog & ) = delete;
public:
  PatchRebootRulesWatchdog( bool on_r )
  {
    PathInfo watch { Pathname(Zypper::instance().config().root_dir)/"/run/reboot-needed" };
    MIL << "Initial reboot-needed: " << watch << endl;
    if ( on_r && not watch.isExist() )
      _watch = watch.path(); // take care it stays absent after commit
  }
  ~PatchRebootRulesWatchdog()
  {
    if ( not _watch.empty() ) {
      MIL << "PATCH_REBOOT_RULES overrules reboot-needed" << endl;
      filesystem::unlink( _watch );
    }
  }
private:
  Pathname _watch;
};

///////////////////////////////////////////////////////////////////
namespace {
  inline ColorString tagProblem() {
    // translators: meaning 'dependency problem' found during solving
    return MSG_WARNINGString(_("Problem: "));
  }
  inline ColorString tagProblem( unsigned i ) {
    std::string s { std::to_string(i)+": " };
    // TODO: fixup .po files "Problem: %s" to fit here
    // TranslatorExplanation %d is the problem number
    return MSG_WARNINGString(str::Format(_("Problem: %s")) % s);
  }
  inline ColorString tagSolution( unsigned i ) {
    // TranslatorExplanation %d is the solution number
    return HIGHLIGHTString(str::Format(_(" Solution %d: ")) % i);
  }

  // Print a problems description (optionally verbose)
  inline std::ostream & dumpProblem( std::ostream & str, unsigned nr, const ResolverProblem & problem_r, bool verbose_r = false )
  {
    MLSep sep;
    // description() is one out of all in completeProblemInfo()
    // picked by the resolver to represent the problem.
    if ( not verbose_r ) {
      str << sep << tagProblem(nr) << problem_r.description();
    } else {
      str << sep << tagProblem(nr) << _("Detailed information: ");
      for ( const std::string & pRule : problem_r.completeProblemInfo() ) {
        str << sep << "- " << pRule;
      }
    }
    const std::string & details { problem_r.details() };
    if ( not details.empty() )
      str << endl << details << endl;
    return str;
  }

  // Print a single problem solution
  inline std::ostream & dumpSolution( std::ostream & str, unsigned nr, const ProblemSolution & solution_r )
  {
    MLSep sep;
    str << sep << tagSolution(nr) << solution_r.description();
    const std::string & details { solution_r.details() };
    if ( not details.empty() )
      str << sep << indent( details, 2 );
    return str;
  }

  // Print a problems solutions
  inline std::ostream & dumpSolutions( std::ostream & str, const ProblemSolutionList & solutions_r )
  {
    MLSep sep;
    unsigned n = 0;
    for ( const auto & solPtr : solutions_r )
      dumpSolution( str << sep, ++n, *solPtr );
    return str;
  }
} // namespace
///////////////////////////////////////////////////////////////////

//! @return true to retry solving now,
//!         false to cancel,
//!         indeterminate to continue
static TriBool show_problem( Zypper & zypper, unsigned nr, const ResolverProblem & prob, ProblemSolutionList & todo )
{
  const ProblemSolutionList & solutions { prob.solutions() };
  unsigned problem_count = God->resolver()->problems().size();
  unsigned solution_count = solutions.size();

  std::string prompt_text;
  if ( problem_count > 1 )
    prompt_text = PL_(
      "Choose the above solution using '1' or skip, retry or cancel",
      "Choose from above solutions by number or skip, retry or cancel",
      solution_count );
  else
    prompt_text = PL_(
      // translators: translate 'c' to whatever you translated the 'c' in
      // "c" and "s/r/c" strings
      "Choose the above solution using '1' or cancel using 'c'",
      "Choose from above solutions by number or cancel",
      solution_count );

  PromptOptions popts;
  unsigned default_reply;
  std::ostringstream numbers;
  for ( unsigned i = 1; i <= solution_count; i++ )
    numbers << i << "/";

  unsigned extInfoAnswer = 0;
  unsigned cancelAnswer = 0;
  int retryAnswer = -1;
  int skipAnswer = -1;

  if ( problem_count > 1 )
  {
    default_reply = solution_count + 2;
    // translators: answers for dependency problem solution input prompt:
    // "Choose from above solutions by number or skip, retry or cancel"
    // Translate the letters to whatever is suitable for your language.
    // The anserws must be separated by slash characters '/' and must
    // correspond to skip/retry/cancel/detailed information in that order.
    // The answers should be lower case letters.
    popts.setOptions( numbers.str() + _("s/r/c/d"), default_reply );
    skipAnswer    = solution_count;
    retryAnswer   = solution_count + 1;
    cancelAnswer  = solution_count + 2;
    extInfoAnswer = solution_count + 3;
  }
  else
  {
    default_reply = solution_count;
    // translators: answers for dependency problem solution input prompt:
    // "Choose from above solutions by number or cancel"
    // Translate the letter 'c' to whatever is suitable for your language
    // and to the same as you translated it in the "s/r/c/d" string
    // See the "s/r/c/d" comment for other details.
    // One letter string  for translation can be tricky, so in case of problems,
    // please report a bug against zypper at bugzilla.suse.com, we'll try to solve it.
    popts.setOptions( numbers.str() + _("c/d"), default_reply );
    cancelAnswer  = solution_count;
    extInfoAnswer = solution_count + 1;
  }

  for ( unsigned i = 0; i < solution_count; i++ )
    popts.setOptionHelp( i, str::Format( _("Choose solution %1%") ) % (i+1) );
  if ( skipAnswer != -1 ) {
    popts.setOptionHelp( skipAnswer,  _("Skip problem and continue.") );
  }
  if ( retryAnswer != -1 ) {
    popts.setOptionHelp( retryAnswer, _("Retry solving immediately.") );
  }
  popts.setOptionHelp( cancelAnswer,  _("Choose no solution and cancel.") );
  popts.setOptionHelp( extInfoAnswer, _("Toggle show detailed conflict information.") );

  bool showExtInfo = false;
  unsigned reply = default_reply;

  while ( true ) {

    std::ostringstream fulltext;
    dumpProblem( fulltext, nr, prob, showExtInfo ) << endl;
    dumpSolutions( fulltext, solutions ) << endl;

    zypper.out().prompt( PROMPT_DEP_RESOLVE, prompt_text, popts, fulltext.str() );
    reply = get_prompt_reply( zypper, PROMPT_DEP_RESOLVE, popts );

    if ( reply == extInfoAnswer ) {
      showExtInfo = !showExtInfo;
      continue;
    }
    // cancel
    if ( reply == cancelAnswer )
      return false;
    // retry
    if (  reply == static_cast<unsigned>(retryAnswer) )
      return true;
    // skip
    if ( reply == static_cast<unsigned>(skipAnswer) )
      return indeterminate; // continue with next problem

    break;
  }

  zypper.out().info( str::Format(_("Applying solution %s")) % (reply + 1), Out::HIGH );
  ProblemSolutionList::const_iterator reply_i = solutions.begin();
  std::advance( reply_i, reply );
  todo.push_back( *reply_i );

  return indeterminate; // continue with next problem
}

// return true to retry solving, false to cancel transaction
static bool show_problems( Zypper & zypper )
{
  Resolver_Ptr resolver = getZYpp()->resolver();
  const ResolverProblemList & rproblems( resolver->problems() );

  // display the number of problems
  if ( rproblems.size() > 1 )
    zypper.out().info( MSG_WARNINGString(str::Format(PL_("%d Problem:", "%d Problems:", rproblems.size())) % rproblems.size()).str() );
  else if ( rproblems.empty() )
  {
    // should not happen! If solve() failed at least one problem must be set!
    INT << "Resolver failed but reported no problems." << endl;
    zypper.out().error(_("Resolver failed but reported no problems.") );
    report_a_bug( zypper.out() );
    return false; // will cancel the transaction
  }

  // for many problems, list them shortly first
  //! \todo handle resolver problems caused by --capability mode arguments specially to give proper output (bnc #337007)
  if (rproblems.size() > 1)
  {
    unsigned n = 0;
    for ( const auto & probPtr : rproblems )
      zypper.out().info() << tagProblem(++n) << probPtr->description() <<endl;
  }

  unsigned n = 0;
  bool retry = true;
  ProblemSolutionList todo;
  // now list all problems with solution proposals
  for ( const auto & probPtr : rproblems )
  {
    zypper.out().info( "", Out::NORMAL, Out::TYPE_NORMAL );	// visual separator
    TriBool stopnow = show_problem( zypper, ++n, *probPtr, todo );
    if ( !indeterminate( stopnow ) )
    {
      retry = bool(stopnow);
      break;
    }
  }

  if ( retry )
  {
    zypper.out().info(_("Resolving dependencies...") );
    resolver->applySolutions( todo );
  }
  return retry;
}

static void dump_pool()
{
  if ( !base::logger::isExcessive() )
    return;

  int count = 1;
  static bool full_pool_shown = false;

  XXX << "---------------------------------------" << endl;
  for ( ResPool::const_iterator it = God->pool().begin(); it != God->pool().end(); ++it, ++count )
  {
    if ( !full_pool_shown		// show item if not shown all before
      || it->status().transacts()	// or transacts
      || !it->isBroken() )		// or broken status
    {
      XXX << count << ": " << *it << endl;
    }
  }
  XXX << "---------------------------------------" << endl;
  full_pool_shown = true;
}


static void set_force_resolution( Zypper & zypper )
{
  // --force-resolution command line parameter value
  TriBool force_resolution = zypper.runtimeData().force_resolution;

  if ( !indeterminate( SolverSettings::instance()._forceResolution ) ) {
    force_resolution = SolverSettings::instance()._forceResolution;
  }

  // if --force-resolution was not specified on the command line,
  // use value from zypper.conf
  if ( zypper.config().solver_forceResolutionCommands.find(zypper.command()) != zypper.config().solver_forceResolutionCommands.end() )
    force_resolution = true;

  // if we still don't have the value, force
  // the resolution by default for the remove commands and the
  // rug_compatible mode. Don't force resolution in non-interactive mode
  // and for update and dist-upgrade command (complex solver request).
  // bnc #369980
  if ( indeterminate(force_resolution) )
  {
    if ( !zypper.config().non_interactive && zypper.command() == ZypperCommand::REMOVE )
      force_resolution = true;
    else
      force_resolution = false;
  }

  // save the setting
  zypper.runtimeData().force_resolution = force_resolution;

  DBG << "force resolution: " << force_resolution << endl;
  std::ostringstream s;
  s << _("Force resolution:") << " " << asYesNo( bool(force_resolution) );
  zypper.out().info(s.str(), Out::HIGH);

  God->resolver()->setForceResolve( bool(force_resolution) );
}

static void set_clean_deps( Zypper & zypper )
{
  if ( zypper.command() == ZypperCommand::REMOVE )
  {
    const auto &solverSettings = SolverSettings::instance();
    if ( solverSettings._cleanDeps == true )
      God->resolver()->setCleandepsOnRemove(true);
    else if ( solverSettings._cleanDeps == false )
      God->resolver()->setCleandepsOnRemove(false);
  }
  DBG << "clean deps on remove: " << God->resolver()->cleandepsOnRemove() << endl;
}

static void set_no_recommends( Zypper & zypper )
{
  bool no_recommends = !zypper.config().solver_installRecommends;

  const auto &solverSettings = SolverSettings::instance();

  // override zypper.conf in these cases:
  if ( zypper.command() == ZypperCommand::REMOVE )
    // never install recommends when removing packages
    no_recommends = true;
  else if ( solverSettings._recommends == false )
    // install also recommended packages unless --no-recommends is specified
    no_recommends = true;
  else if ( solverSettings._recommends == true )
    no_recommends = false;

  DBG << "no recommends (only requires): " << no_recommends << endl;
  God->resolver()->setOnlyRequires(no_recommends);
}


static void set_ignore_recommends_of_installed(Zypper & zypper)
{
  bool ignore = true;
  if ( ( zypper.command() == ZypperCommand::DIST_UPGRADE && SolverSettings::instance()._recommends != false )
    || zypper.command() == ZypperCommand::INSTALL_NEW_RECOMMENDS )
    ignore = false;
  DBG << "ignore recommends of already installed packages: " << ignore << endl;
  God->resolver()->setIgnoreAlreadyRecommended( ignore );
}


static void set_solver_flags( Zypper & zypper )
{
  set_force_resolution( zypper );
  set_clean_deps( zypper );
  set_no_recommends( zypper );
  set_ignore_recommends_of_installed( zypper );

  God->resolver()->setUpdateMode( zypper.runtimeData().solve_with_update );

  const auto &solverSettings = SolverSettings::instance();
  God->resolver()->setFocus( solverSettings._focus );
  // Use resolver->dupSet... if ZypperCommand::DIST_UPGRADE
  if ( zypper.command() == ZypperCommand::DIST_UPGRADE )
  {
    if ( !indeterminate( solverSettings._allowDowngrade ) ) God->resolver()->dupSetAllowDowngrade( bool(solverSettings._allowDowngrade) );
    if ( !indeterminate( solverSettings._allowNameChange ) ) God->resolver()->dupSetAllowNameChange( bool(solverSettings._allowNameChange) );
    if ( !indeterminate( solverSettings._allowArchChange ) ) God->resolver()->dupSetAllowArchChange( bool(solverSettings._allowArchChange) );
    if ( !indeterminate( solverSettings._allowVendorChange ) ) God->resolver()->dupSetAllowVendorChange( bool(solverSettings._allowVendorChange) );
  }
  else
  {
    if ( !indeterminate( solverSettings._allowDowngrade ) ) God->resolver()->setAllowDowngrade( bool(solverSettings._allowDowngrade) );
    if ( !indeterminate( solverSettings._allowNameChange ) ) God->resolver()->setAllowNameChange( bool(solverSettings._allowNameChange) );
    if ( !indeterminate( solverSettings._allowArchChange ) ) God->resolver()->setAllowArchChange( bool(solverSettings._allowArchChange) );
    if ( !indeterminate( solverSettings._allowVendorChange ) ) God->resolver()->setAllowVendorChange( bool(solverSettings._allowVendorChange) );
  }
}


/**
 * Run the solver.
 *
 * \return <tt>true</tt> if a solution has been found, <tt>false</tt> otherwise
 */
bool resolve( Zypper & zypper )
{
  dump_pool(); // debug
  set_solver_flags(zypper);
  DBG << "Calling the solver..." << endl;
  return God->resolver()->resolvePool();
}

static bool verify( Zypper & zypper )
{
  dump_pool();
  set_solver_flags( zypper );
  zypper.out().info(_("Verifying dependencies..."), Out::HIGH );
  DBG << "Calling the solver to verify system..." << endl;
  return God->resolver()->verifySystem();
}

static bool dist_upgrade( Zypper & zypper )
{
  dump_pool();
  set_solver_flags( zypper );

  // Test for repositories to upgrade to (--from)
  // If those are specified addUpgradeRepo and solve,
  // otherwise perform a full dist upgrade.

  auto & dupSettings = DupSettings::instance();
  if ( dupSettings._fromRepos.size() ) {
    std::list<RepoInfo> specified;
    std::list<std::string> not_found;

    get_repos( zypper, dupSettings._fromRepos.begin(), dupSettings._fromRepos.end(), specified, not_found );
    report_unknown_repos( zypper.out(), not_found );

    if ( !not_found.empty() )
    {
      zypper.setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
      ZYPP_THROW( ExitRequestException("Some of specified repositories were not found.") );
    }

    if ( ! specified.empty() )
    {
      // Here: do upgrade for the specified repos:
      Resolver_Ptr resolver( God->resolver() );
      ResPool      pool    ( God->pool() );
      for_( it, specified.begin(), specified.end() )
      {
        Repository repo( pool.reposFind( it->alias() ) );
        MIL << "Adding upgrade repository: " << repo.alias() << endl;
        resolver->addUpgradeRepo( repo );
      }

      DBG << "Calling the solver..." << endl;
      //! \todo Somehow tell set_solver_flags/set_ignore_recommends_of_installed that
      //! this is no full upgrade. Until then set setIgnoreAlreadyRecommended again here:
      resolver->setIgnoreAlreadyRecommended( true );
      return resolver->resolvePool();
    }
  }

  // Here: compute the full upgrade

  zypper.out().info(_("Computing upgrade..."), Out::HIGH );
  DBG << "Calling the solver doUpgrade()..." << endl;
  return God->resolver()->doUpgrade();
}

/**
 * To be called after setting solver flags and calling solver methods
 * (like doUpdate(), doUpgrade(), verify(), and resolve()) to generate
 * solver testcase.
 */
static void make_solver_test_case( Zypper & zypper )
{
  static const std::string testcase_dir( "/var/log/zypper.solverTestCase" );

  zypper.out().info(_("Generating solver test case...") );
  if ( God->resolver()->createSolverTestcase( testcase_dir ) )
    zypper.out().info( str::Format(_("Solver test case generated successfully at %s.")) % testcase_dir );
  else
  {
    zypper.out().error(_("Error creating the solver test case.") );
    zypper.setExitCode( ZYPPER_EXIT_ERR_ZYPP );
  }
}

SolveAndCommitPolicy::SolveAndCommitPolicy( )
{
  // set up the various commit flags which are scattered all over the place
  _zyppCommitPolicy
    .dryRun( DryRunSettings::instance().isEnabled() )
    .replaceFiles( FileConflictPolicy::instance()._replaceFiles )
    .allowDowngrade( false );

  _zyppCommitPolicy.downloadMode( DownloadDefault );
  _zyppCommitPolicy.syncPoolAfterCommit( _zyppCommitPolicy.dryRun() ? false : Zypper::instance().runningShell() );
}

bool SolveAndCommitPolicy::forceCommit() const
{ return _forceCommit; }

SolveAndCommitPolicy &SolveAndCommitPolicy::forceCommit(bool enable)
{ _forceCommit = enable; return *this; }

const Summary::ViewOptions &SolveAndCommitPolicy::summaryOptions() const
{ return _summaryOptions; }

SolveAndCommitPolicy &SolveAndCommitPolicy::summaryOptions( Summary::ViewOptions options )
{ _summaryOptions = options; return *this; }

ZYppCommitPolicy &SolveAndCommitPolicy::zyppCommitPolicy()
{ return _zyppCommitPolicy; }

const ZYppCommitPolicy &SolveAndCommitPolicy::zyppCommitPolicy() const
{ return _zyppCommitPolicy; }

SolveAndCommitPolicy &SolveAndCommitPolicy::zyppCommitPolicy( ZYppCommitPolicy policy )
{ _zyppCommitPolicy = policy; return *this; }

SolveAndCommitPolicy & SolveAndCommitPolicy::downloadMode( DownloadMode dlMode )
{
  // set up the DownloadMode and emit a info if we auto override it due to singletrans
  // the libzypp code will do the same but only emit a warning to the logs, lets be a bit more verbose
  if ( dlMode != _zyppCommitPolicy.downloadMode() ) {

    if ( dlMode == DownloadAsNeeded && _zyppCommitPolicy.singleTransModeEnabled() ) {
      Zypper::instance().out().warning( _("DownloadAsNeeded can not be used with ZYPP_SINGLE_RPMTRANS=1, falling back to DownloadInAdvance") );
      dlMode = DownloadInAdvance;
    }
    _zyppCommitPolicy.downloadMode( dlMode );
  }
  return *this;
}

DownloadMode SolveAndCommitPolicy::downloadMode() const
{ return _zyppCommitPolicy.downloadMode(); }

/** fate #300763
 * This is called after each commit to notify user about running processes that
 * use libraries or other files that have been removed since their execution.
 */
static void notify_processes_using_deleted_files( Zypper & zypper )
{
  if ( ! zypper.config().psCheckAccessDeleted ) {
    zypper.out().info( str::form(_("Check for running processes using deleted libraries is disabled in zypper.conf. Run '%s' to check manually."),
                                 "zypper ps -s" ) );
  } else {
    zypper.out().info(_("Checking for running processes using deleted libraries..."), Out::HIGH );
    CheckAccessDeleted checker( false ); // wait for explicit call to check()
    try
    {
      checker.check();
    }
    catch( const Exception & e )
    {
      if ( zypper.out().verbosity() > Out::NORMAL )
      {
        if ( e.historySize() )
    zypper.out().error( e, _("Check failed:") );
        else
            zypper.out().info( str::Str() << ( ColorContext::MSG_WARNING << _("Skip check:") ) << " " << e.asUserString() );
      }
    }

    // Don't suggest "zypper ps" if zypper is the only prog with deleted open files.
    if ( checker.size() > 1 || ( checker.size() == 1 && checker.begin()->pid != str::numstring(::getpid()) ) )
    {
      zypper.out().info( str::Format(_("There are running programs which still use files and libraries deleted or updated by recent upgrades. They should be restarted to benefit from the latest updates. Run '%1%' to list these programs.") )
      % "zypper ps -s" );
    }
  }

  zypper.out().info(" ");
  NeedsRebootingCmd::checkRebootNeeded( zypper, indeterminate );
}

static void show_update_messages( Zypper & zypper, const UpdateNotifications & messages )
{
  if ( messages.empty() )
    return;

  zypper.out().info(_("Update notifications were received from the following packages:") );
  MIL << "Received " << messages.size() << " update notification(s):" << endl;

  std::ostringstream msg;
  for_( it, messages.begin(), messages.end() )
  {
    MIL << "- From " << it->solvable().asString() << " in file " << Pathname::showRootIf( zypper.config().root_dir, it->file() ) << endl;
    zypper.out().info( it->solvable().asString() + " (" + Pathname::showRootIf(zypper.config().root_dir, it->file()) + ")" );
    {
      msg << str::form(_("Message from package %s:"), it->solvable().name().c_str() ) << endl << endl;
      InputStream istr( Pathname::assertprefix( zypper.config().root_dir, it->file() ) );
      iostr::copy( istr, msg );
      msg << endl << "-----------------------------------------------------------------------------" << endl;
    }
  }

  PromptOptions popts;
  popts.setOptions(_("y/n"), 1 );
  std::string prompt_text( _("View the notifications now?") );
  unsigned reply;

  zypper.out().prompt( PROMPT_YN_INST_REMOVE_CONTINUE, prompt_text, popts );
  reply = get_prompt_reply( zypper, PROMPT_YN_INST_REMOVE_CONTINUE, popts );

  if ( reply == 0 )
    show_text_in_pager( msg.str() );
}


// ----------------------------------------------------------------------------
// commit
// ----------------------------------------------------------------------------

/**
 * Calls the appropriate solver function with flags according to current
 * command and options, show the summary, and commits.
 *
 * @return ZYPPER_EXIT_OK - successful commit,
 *  ZYPPER_EXIT_ERR_ZYPP - if ZYppCommitResult contains resolvables with errors,
 *  ZYPPER_EXIT_INF_REBOOT_NEEDED - if one of patches to be installed needs machine reboot,
 *  ZYPPER_EXIT_INF_RESTART_NEEDED - if one of patches to be installed needs package manager restart
 */

void solve_and_commit ( Zypper &zypper, SolveAndCommitPolicy policy )
{
  bool need_another_solver_run = true;
  bool dryRunEtc = policy.zyppCommitPolicy().dryRun() || ( policy.zyppCommitPolicy().downloadMode() == DownloadOnly );
  do
  {
    // CALL SOLVER

    // doUpdate sets this flag, if no other jobs are to be included
    if ( not zypper.runtimeData().solve_update_only )
    {
      MIL << "solving..." << endl;

      while ( true )
      {
        bool success;
        if ( zypper.command() == ZypperCommand::VERIFY )
          success = verify(zypper);
        else if ( zypper.command() == ZypperCommand::DIST_UPGRADE )
        {
          zypper.out().info(_("Computing distribution upgrade...") );
          success = dist_upgrade(zypper);
        }
        else
        {
          zypper.out().info(_("Resolving package dependencies...") );
          success = resolve( zypper );
        }

        // go on, we've got solution or we don't want a solution (we want testcase)
        if ( success || SolverSettings::instance()._debugSolver )
          break;

        success = show_problems( zypper );
        if (!success)
        {
          zypper.setExitCode( ZYPPER_EXIT_ERR_ZYPP ); // bnc #242736
          return;
        }
      }
    } else {
      MIL << "Computing package update..." << endl;
      set_solver_flags( zypper );   // bsc#1201972: make sure 'up' also respects solver options
      zypp::getZYpp()->resolver()->doUpdate();
    }

    if ( SolverSettings::instance()._debugSolver )
    {
      make_solver_test_case( zypper );
      return;	// ZYPPER_EXIT_OK
    }

    MIL << "got solution, showing summary" << endl;

    // SHOW SUMMARY

    Summary summary( God->pool(), policy.summaryOptions() );

    if ( zypper.out().verbosity() == Out::HIGH )
      summary.setViewOption( Summary::SHOW_VERSION );
    else if ( zypper.out().verbosity() == Out::DEBUG )
      summary.setViewOption( Summary::SHOW_ALL );

    // if running on SUSE Linux Enterprise, report unsupported packages
    if ( runningOnEnterprise() )
      summary.setViewOption( Summary::SHOW_UNSUPPORTED );
    else
      summary.unsetViewOption( Summary::SHOW_UNSUPPORTED );

    if ( policy.zyppCommitPolicy().downloadMode() == DownloadOnly )
      summary.setDownloadOnly( true );

    // show the summary
    if ( zypper.out().type() == Out::TYPE_XML )
      summary.dumpAsXmlTo( cout );
    else
      summary.dumpTo( cout );


    if ( summary.packagesToGetAndInstall()
      || summary.packagesToRemove()
      || !zypper.runtimeData().srcpkgs_to_install.empty() )
    {
      if ( zypper.command() == ZypperCommand::VERIFY )
        zypper.out().info(_("Some of the dependencies of installed packages are broken."
        " In order to fix these dependencies, the following actions need to be taken:") );

      // check root user
      if ( zypper.command() == ZypperCommand::VERIFY && geteuid() != 0
        && !zypper.config().changedRoot )
      {
        zypper.out().error(_("Root privileges are required to fix broken package dependencies.") );
        zypper.setExitCode( ZYPPER_EXIT_ERR_PRIVILEGES );
        return;
      }

      // PROMPT

      bool show_p_option = ( summary.packagesToRemove() && ( zypper.command() == ZypperCommand::INSTALL
                                                          || zypper.command() == ZypperCommand::UPDATE) )
                        || ( summary.packagesToGetAndInstall() && zypper.command() == ZypperCommand::REMOVE );

      PromptOptions popts;
      // translators: These are the "Continue?" prompt options corresponding to
      // "Yes / No / show Problems / Versions / Arch / Repository /
      // vendor / Details / show in pager". This prompt will appear
      // after install/update and similar command installation summary.
      // Translate to whathever is suitable for your language
      // The anserws must be separated by slash characters '/' and must
      // correspond to the above options, in that exact order.
      // The answers should be lower case letters, but in general, any UTF-8
      // string will do.
      //! \todo add c for changelog and x for explain (show the dep tree)
      popts.setOptions(_("y/n/p/v/a/r/m/d/g"), 0 );
      popts.setShownCount( 3 );
      if ( !( zypper.runtimeData().force_resolution && show_p_option ) )
        popts.disable( 2 );
      // translators: help text for 'y' option in the 'Continue?' prompt
      popts.setOptionHelp( 0, _("Yes, accept the summary and proceed with installation/removal of packages.") );
      // translators: help text for 'n' option in the 'Continue?' prompt
      popts.setOptionHelp( 1, _("No, cancel the operation.") );
      // translators: help text for 'p' option in the 'Continue?' prompt
      popts.setOptionHelp( 2, _("Restart solver in no-force-resolution mode in order to show dependency problems.") );
      // translators: help text for 'v' option in the 'Continue?' prompt
      popts.setOptionHelp( 3, _("Toggle display of package versions.") );
      // translators: help text for 'a' option in the 'Continue?' prompt
      popts.setOptionHelp( 4, _("Toggle display of package architectures.") );
      // translators: help text for 'r' option in the 'Continue?' prompt
      popts.setOptionHelp( 5, _("Toggle display of repositories from which the packages will be installed.") );
      // translators: help text for 'm' option in the 'Continue?' prompt
      popts.setOptionHelp( 6, _("Toggle display of package vendor names.") );
      // translators: help text for 'd' option in the 'Continue?' prompt
      popts.setOptionHelp( 7, _("Toggle between showing all details and as few details as possible.") );
      // translators: help text for 'g' option in the 'Continue?' prompt
      popts.setOptionHelp( 8, _("View the summary in pager.") );
      // translators: help text for 'x' option in the 'Continue?' prompt
      // popts.setOptionHelp( 8, _("Explain why the packages are going to be installed.") );

      std::string prompt_text( text::qContinue() );

      bool do_commit = false;
      unsigned reply;
      do
      {
        zypper.out().prompt( PROMPT_YN_INST_REMOVE_CONTINUE, prompt_text, popts );
        reply = get_prompt_reply( zypper, PROMPT_YN_INST_REMOVE_CONTINUE, popts );

        switch( reply )
        {
        case 0: // y
        {
          do_commit = true;
          need_another_solver_run = false;
          break;
        }
        case 2: // p
        {
          // one more solver solver run with force-resoltion off
          zypper.runtimeData().force_resolution = false;
          // undo solver changes before retrying
          God->resolver()->undo();
          continue;
        }
        case 3: // v - show version
        {
          summary.toggleViewOption( Summary::SHOW_VERSION );
          summary.dumpTo( cout );
          break;
        }
        case 4: // a - show arch
        {
          summary.toggleViewOption( Summary::SHOW_ARCH );
          summary.dumpTo( cout );
          break;
        }
        case 5: // r - show repos
        {
          summary.toggleViewOption( Summary::SHOW_REPO );
          summary.dumpTo( cout );
          break;
        }
        case 6: // m - show vendor
        {
          summary.toggleViewOption( Summary::SHOW_VENDOR );
          summary.dumpTo( cout );
          break;
        }
        case 7: // d - show details (all attributes)
        {
          summary.toggleViewOption( Summary::DETAILS );
          summary.dumpTo( cout );
          break;
        }
        case 8: // g - view in pager
        {
          std::ostringstream s;
          summary.setForceNoColor( true );
          summary.dumpTo( s );
          summary.setForceNoColor( false );
          show_text_in_pager( s.str() );
          break;
        }
        default: // n - no
          need_another_solver_run = false;
        }
      } while ( reply > 2 );

      if ( need_another_solver_run )
        continue;

      if ( !do_commit )
      {
        zypper.setExitCode( ZYPPER_EXIT_ERR_ZYPP );
        return;
      }
      else
      {
        // COMMIT

        if ( !confirm_licenses( zypper ) )
        {
          zypper.setExitCode( ZYPPER_EXIT_ERR_ZYPP );
          return;
        }

        std::optional<ZYppCommitResult> result;
        try
        {
          RuntimeData & gData = Zypper::instance().runtimeData();
          // Total packages to download & install.
          // To be used to write overall progress of retrieving packages.
          gData.commit_pkgs_total = summary.packagesToGetAndInstall();
          gData.commit_pkg_current = 0;
          // To be used to show overall progress of rpm transactions.
          gData.rpm_pkgs_total = God->resolver()->getTransaction().actionSize();
          gData.rpm_pkg_current = 0;
          gData.entered_commit = true;	// bsc#946750 - give ZYPPER_EXIT_ERR_COMMIT priority over ZYPPER_EXIT_ON_SIGNAL

          MIL << "committing..." << endl;
          if ( zypper.out().verbosity() >= Out::HIGH )
          {
            std::ostringstream s;
            s << _("committing");
            if ( policy.zyppCommitPolicy().dryRun() )
              s << " " << _("(dry run)");
            zypper.out().info( s.str(), Out::HIGH );
          }

          // bsc#1183268: Patch reboot-needed flag overrules included packages.
          PatchRebootRulesWatchdog guard { summary.hasViewOption( Summary::PATCH_REBOOT_RULES ) && not summary.needMachineReboot() };

          MIL << "Using commit policy: " << policy.zyppCommitPolicy() << endl;
          result = God->commit( policy.zyppCommitPolicy() );

          gData.entered_commit = false;

          if ( !result->allDone() && !( dryRunEtc && result->noError() ) )
          { zypper.setExitCode( result->attemptToModify() ? ZYPPER_EXIT_ERR_COMMIT : ZYPPER_EXIT_ERR_ZYPP ); }	// error message comes later....

          MIL << endl << "DONE" << endl;
          if ( zypper.out().verbosity() >= Out::HIGH )
          {
            std::ostringstream s;
            s << *result;
            zypper.out().info( s.str(), Out::HIGH );
          }

          show_update_messages( zypper, result->updateMessages() );
        }
        catch ( const media::MediaException & e )
        {
          ZYPP_CAUGHT( e );
          zypper.out().error( e, _("Problem retrieving the package file from the repository:"),
                              _("Please see the above error message for a hint.") );
          zypper.setExitCode( ZYPPER_EXIT_ERR_COMMIT );
          return;
        }
        catch ( repo::RepoException & e )
        {
          ZYPP_CAUGHT( e );
          bool refresh_needed = false;
          if ( !e.info().baseUrlsEmpty() )
          {
            try
            {
              RepoManager manager( zypper.config().rm_options );
              for( RepoInfo::urls_const_iterator it = e.info().baseUrlsBegin(); it != e.info().baseUrlsEnd(); ++it )
              {
                if ( manager.checkIfToRefreshMetadata( e.info(), *it, RepoManager::RefreshForced ) == RepoManager::REFRESH_NEEDED )
                {
                  refresh_needed = true;
                  break;
                }
              }
            }
            catch ( const Exception & )
            { DBG << "check if to refresh exception caught, ignoring" << endl; }
          }

          std::string hint;
          if ( refresh_needed )
            // translators: the first %s is 'zypper refresh' and the second is repo alias
            hint = str::Format(_("Repository '%s' is out of date. Running '%s' might help.")) % e.info().alias() % "zypper refresh";
          else
            hint = _("Please see the above error message for a hint.");
          zypper.out().error( e, _("Problem retrieving the package file from the repository:"), hint );
          zypper.setExitCode( ZYPPER_EXIT_ERR_COMMIT );
          return;
        }
        catch ( const FileCheckException & e )
        {
          ZYPP_CAUGHT( e );
          zypper.out().error( e,
              _("The package integrity check failed. This may be a problem"
              " with the repository or media. Try one of the following:\n"
              "\n"
              "- just retry previous command\n"
              "- refresh the repositories using 'zypper refresh'\n"
              "- use another installation medium (if e.g. damaged)\n"
              "- use another repository") );
          zypper.setExitCode( ZYPPER_EXIT_ERR_COMMIT );
          return;
        }
        catch ( const Exception & e )
        {
          ZYPP_CAUGHT( e );
          zypper.out().error( e,
              _("Problem occurred during or after installation or removal of packages:"),
              _("Please see the above error message for a hint.") );
          zypper.setExitCode( ZYPPER_EXIT_ERR_COMMIT );
          return;
        }

        if ( zypper.exitCode() != ZYPPER_EXIT_OK )
        {
          if ( result && result->singleTransactionMode() ) {

            CommitSummary cSummary( *result );
            if ( zypper.out().verbosity() == Out::HIGH )
              cSummary.setViewOption( CommitSummary::ViewOptions( CommitSummary::SHOW_VERSION | CommitSummary::SHOW_REPO ) );
            else if ( zypper.out().verbosity() == Out::DEBUG )
              cSummary.setViewOption( CommitSummary::SHOW_ALL );

            // show the summary
            if ( zypper.out().type() == Out::TYPE_XML )
              cSummary.dumpAsXmlTo( cout );
            else
              cSummary.dumpTo( cout );

          } else {
            CommitSummary::showBasicErrorMessage( zypper );
          }

        }
        else
        {
          // install any pending source packages
          //! \todo This won't be necessary once we get a new solver flag
          //! for installing source packages without their build deps
          if ( !zypper.runtimeData().srcpkgs_to_install.empty() )
            install_src_pkgs( zypper, policy.zyppCommitPolicy().downloadMode() );

          // set return value to 'reboot needed'
          if ( summary.needMachineReboot() )
          {
            zypper.setExitCode( ZYPPER_EXIT_INF_REBOOT_NEEDED );
            zypper.out().warning(_("One of the installed patches requires a reboot of"
            " your machine. Reboot as soon as possible."), Out::QUIET );

            // bsc#1217873: Make sure reboot-needed is remembered until next boot.
            Pathname file { Pathname(Zypper::instance().config().root_dir)/"/run/reboot-needed" };
            if ( filesystem::assert_file( file ) == EEXIST )
              filesystem::touch( file );
          }
          // set return value to 'restart needed' (restart of package manager)
          // however, 'reboot needed' takes precedence
          else if ( zypper.exitCode() != ZYPPER_EXIT_INF_REBOOT_NEEDED && summary.needPkgMgrRestart() )
          {
            zypper.setExitCode( ZYPPER_EXIT_INF_RESTART_NEEDED );
            zypper.out().warning(_("One of the installed patches affects the package"
            " manager itself. Run this command once more to install any other"
            " needed patches." ), Out::QUIET, Out::TYPE_NORMAL ); // don't show this to machines
          }
        }

        // check for running services (fate #300763)
        if ( !( zypper.config().changedRoot || dryRunEtc )
          && ( summary.packagesToRemove() || summary.packagesToUpgrade() || summary.packagesToDowngrade() ) )
        {
          notify_processes_using_deleted_files( zypper );
        }
      }
    }
    // noting to do
    else
    {
      //used to make sure locales and metadata is written even if there is no packages to be installed/removed
      if ( policy.forceCommit() )
        God->commit( ZYppCommitPolicy() );

      if ( zypper.command() == ZypperCommand::VERIFY )
        zypper.out().info(_("Dependencies of all installed packages are satisfied.") );
      else
        zypper.out().info(_("Nothing to do.") );

      break;
    }
  }
  while ( need_another_solver_run );
}
