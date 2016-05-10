/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/


/** \file SolverRequester.h
 *
 */

#ifndef SOLVERREQUESTER_H_
#define SOLVERREQUESTER_H_

#include <string>

#include <zypp/ZConfig.h>
#include <zypp/Date.h>
#include <zypp/PoolItem.h>

#include "Zypper.h"
#include "PackageArgs.h"
#include "utils/misc.h" // for ResKindSet; might make sense to move this elsewhere

class Out;

///////////////////////////////////////////////////////////////////
/// \class CliMatchPatch
/// \brief Functor testing whether a Patch matches CLI options (non-patches pass)
///////////////////////////////////////////////////////////////////
struct CliMatchPatch
{
  /** Default Ctor: No filter */
  CliMatchPatch()
  {}

  /** Ctor: Filter according to zypper CLI options */
  CliMatchPatch( Zypper & zypper )
  {
    for ( const auto & val : zypper.cOptValues( "date" ) )
    {
      // ISO 8601 format
      Date v( std::string(val), "%F");
      if ( v && ( !_dateBefore || v < _dateBefore ) )
      {
	_dateBefore = v;
      }
    }
    for ( const auto & val : zypper.cOptValues( "category" ) )
    {
      str::split( str::toLower( std::string(val) ), std::inserter(_categories, _categories.end()), "," );
      for ( const std::string & cat : _categories )
      {
	if ( Patch::categoryEnum( cat ) == Patch::CAT_OTHER )
	{
	  zypper.out().warning( str::Format(_("Suspicious category filter value '%1%'.")) % cat );
	}
      }
    }
    for ( const auto & val : zypper.cOptValues( "severity" ) )
    {
      str::split( str::toLower( std::string(val) ), std::inserter(_severities, _severities.end()), "," );
      for ( const std::string & sev : _severities )
      {
	if ( Patch::severityFlag( sev ) == Patch::SEV_OTHER )
	{
	  zypper.out().warning( str::Format(_("Suspicious severity filter value '%1%'.")) % sev );
	}
      }
    }
  }

  enum class Missmatch
  {
    None	= 0,
    Date	= 1<<0,
    Category	= 1<<1,
    Severity	= 1<<2,
  };

  Missmatch missmatch( const Patch::constPtr & patch_r ) const
  {
    if ( patch_r )	// non-patches pass
    {
      if ( _dateBefore && patch_r->timestamp() > _dateBefore )
	return Missmatch::Date;
      if ( ! ( _categories.empty() || patch_r->isCategory( _categories ) ) )
	return Missmatch::Category;
      if ( ! ( _severities.empty() || patch_r->isSeverity( _severities ) ) )
	return Missmatch::Severity;
    }
    return Missmatch::None;
  }

  bool operator()( const Patch::constPtr & patch_r ) const
  { return missmatch( patch_r ) == Missmatch::None; }

private:
  friend class SolverRequester;	// SolverRequester::updatePatches uses _dateBefore
  Date _dateBefore;
  std::set<std::string> _categories;
  std::set<std::string> _severities;
};


/**
 * Issue various requests to the dependency resolver, based on given
 * arguments (\ref PackageArgs) and options (\ref SolverRequester::Options).
 *
 *   SolverRequester::Options sropts;
 *   sropts.force = cmdoptions.find("force") != cmdoptions.end()
 *   ...
 *
 *   vector<string> args;
 *   args.push_back("stellarium");
 *   args.push_back("kstars");
 *
 *   SolverRequester sr(sropts);
 *   sr.install(args);              // implicit conversion to PackageArgs
 *
 *   sr.printFeedback(out());
 */
class SolverRequester
{
public:
  struct Options
  {
    Options()
      : force( false )
      , oldpackage( false )
      , force_by_cap( false )
      , force_by_name( false )
      , best_effort( false )
      , skip_interactive( false )
      , allow_vendor_change( ZConfig::instance().solver_allowVendorChange() )
    {}

    void setForceByName( bool value = true );
    void setForceByCap ( bool value = true );

    /**
     * If true, the requester will force the operation in some defined cases,
     * even if the operation would otherwise not be allowed.
     *
     * \todo define the cases
     */
    bool force;

    /**
     * If true, an explicit version downgrade upon install will be allowed.
     * Enables rollback without need to use the 'malicious --force'.
     */
    bool oldpackage;

    /**
     * Force package selection by capabilities they provide.
     * Do not try to select packages by name first.
     */
    bool force_by_cap;

    /**
     * Force package selection by name.
     * Do not fall back to selection by capability if give package is not found
     * by name.
     */
    bool force_by_name;

    /**
     * Whether to request updates via Selectable::updateCandidate() or
     * via addRequires(higher-than-installed)
     */
    bool best_effort;

    /**
     * Whether to skip installs/updates that need user interaction.
     */
    bool skip_interactive;

    /** Patch specific CLI option filter */
    CliMatchPatch cliMatchPatch;

    /** Whether to ignore vendor when selecting packages */
    bool allow_vendor_change;

    /** Aliases of the repos from which the packages should be installed */
    std::list<std::string> from_repos;
  };


  /**
   * A piece of feedback from the SolverRequester.
   */
  class Feedback
  {
  public:
    enum Id
    {
      /**
       * Given combination of arguments and options makes no sense or the
       * functionality is not defined and implemented
       */
      INVALID_REQUEST,

      /** The search for the string in object names failed, but will try caps */
      NOT_FOUND_NAME_TRYING_CAPS,
      NOT_FOUND_NAME,
      NOT_FOUND_CAP,

      /** Removal or update was requested, but there's no installed item. */
      NOT_INSTALLED,

      /** Removal by capability requested, but no provider is installed. */
      NO_INSTALLED_PROVIDER,

      /** Selected object is already installed. */
      ALREADY_INSTALLED,
      NO_UPD_CANDIDATE,
      UPD_CANDIDATE_CHANGES_VENDOR,
      UPD_CANDIDATE_HAS_LOWER_PRIO,
      UPD_CANDIDATE_IS_LOCKED,

      /**
       * The installed package is no longer available in repositories.
       * => can't reinstall, can't update/downgrade..
       */
      NOT_IN_REPOS,

      /**
       * Selected object is not the highest available, because of user
       * restrictions like repo(s), version, architecture.
       */
      UPD_CANDIDATE_USER_RESTRICTED,
      INSTALLED_LOCKED,

      /**
       * Selected object is older that the installed. Won't allow downgrade
       * unless --oldpackage is used.
       */
      SELECTED_IS_OLDER,

      // ********* patch/pattern/product specialties **************************

      /**
       * !patch.status().isRelevant()
       */
      PATCH_NOT_NEEDED,

      /**
       * Skipping a patch because it is marked as interactive or has
       * license to confirm and --skip-interactive is requested
       * (also --non-interactive, since it implies --skip-interactive)
       */
      PATCH_INTERACTIVE_SKIPPED,

      /**
       * Patch was requested (install/update command), but it's locked
       * (set to ignore). Its installation can be forced with --force.
       *
       * In the 'patch' command, do not issue this feedback, silently skip
       * the unwanted patch.
       */
      PATCH_UNWANTED,

      /**
       * Patch is not in the specified category
       */
      PATCH_WRONG_CAT,
      /**
       * Patch is not in the specified severity
       */
      PATCH_WRONG_SEV,

      /**
       * Patch is too new and a date limit was specified
       */
      PATCH_TOO_NEW,

      // ********** zypp requests *********************************************

      SET_TO_INSTALL,
      FORCED_INSTALL,
      SET_TO_REMOVE,
      ADDED_REQUIREMENT,
      ADDED_CONFLICT
    };

    Feedback( const Id id,
	      const PackageSpec & reqpkg,
	      const PoolItem & selected = PoolItem(),
	      const PoolItem & installed = PoolItem() )
    : _id( id )
    , _reqpkg( reqpkg )
    , _objsel( selected )
    , _objinst( installed )
    {}

    Feedback( const Id id, const PackageSpec & reqpkg, std::string userdata )
    : _id( id )
    , _reqpkg( reqpkg )
    , _userdata( std::move(userdata) )
    {}

    Id id() const
    { return _id; }

    const PoolItem selectedObj() const
    { return _objsel; }

    std::string asUserString( const SolverRequester::Options & opts ) const;
    void print( Out & out, const SolverRequester::Options & opts ) const;

  private:
    Id _id;

    /** Specification of requested package as devised from command line args. */
    PackageSpec _reqpkg;

    /** The selected object */
    PoolItem _objsel;
    /** The installed object */
    PoolItem _objinst;
    /** For printing; content depends on _id */
    std::string _userdata;
  };

public:
  SolverRequester( const Options & opts = Options() )
  : _opts( opts )
  , _command( ZypperCommand::NONE )
  {}

public:
  /** Request installation of specified objects. */
  void install(const PackageArgs & args);

  /**
   * Request removal of specified objects.
   * \note Passed \a args must be constructed with
   *       PackageArgs::Options::do_by_default = false.
   */
  void remove( const PackageArgs & args );

  /**
   * Variant of remove(const PackageArgs&) to avoid implicit conversion
   * from vector<string> to PackageArgs.
   */
  void remove( const std::vector<std::string> & rawargs, const ResKind & kind = ResKind::package )
  {
    PackageArgs::Options opts;
    opts.do_by_default = false;
    remove( PackageArgs( rawargs, kind, opts ) );
  }

  /** Request update of specified objects. */
  void update( const PackageArgs & args );

  /** Request update of all satisfied patterns.
   * TODO define this */
  void updatePatterns();

  /**
   * Request installation of all needed patches.
   * If there are any needed patches flagged "affects package management", only
   * these get selected (need to call this again once these are installed).
   */
  void updatePatches();

  /**
   * Set specified patch for installation.
   *
   * \param selected  Selected patch
   * \return True if the patch was set for installation, false otherwise.
   */
  bool installPatch( const PoolItem & selected );

  bool hasFeedback( const Feedback::Id id ) const;
  void printFeedback( Out & out ) const
  { for_( fb, _feedback.begin(), _feedback.end() ) fb->print( out, _opts ); }

  const std::set<PoolItem> & toInstall() const { return _toinst; }
  const std::set<PoolItem> & toRemove() const { return _toremove; }
  const std::set<Capability> & requires() const { return _requires; }
  const std::set<Capability> & conflicts() const { return _conflicts; }

private:
  void installRemove( const PackageArgs & args );

  /**
   * Requests installation or update to the best of objects available in repos
   * according to specified arguments and options.
   *
   * If at least one provider of given \a cap is already installed,
   * this method checks for available update candidates and tries to select
   * the best for installation (thus update). Reports any problems or interesting
   * info back to user.
   *
   * \param cap       Capability object carrying specification of requested
   *                  object (name/edtion/arch). The requester will request
   *                  the best of matching objects.
   * \param repoalias Restrict the candidate lookup to specified repo
   *                  (in addition to those given in Options::from_repos).
   *
   * \note Glob can be used in the capability name for matching by name.
   * \see Options
   */
  void install( const PackageSpec & pkg );

  /**
   * Request removal of all packages matching given \a cap by name/edition/arch,
   * or providing the capability.
   *
   * Glob can be used in the capability name for matching by name.
   * If \a repoalias is given, restrict the candidate lookup to specified repo.
   *
   * \see Options
   */
  void remove( const PackageSpec & pkg );

  /**
   * Update to specified \a selected and report if there's any better
   * candidate when restrictions like vendor change, repository priority,
   * requested repositories, arch, edition and similar are not applied.
   * Use this method only if you have already chosen the candidate. The \a pkg
   * only carries information about the original request so that
   * proper feedback can be provided.
   *
   * \param pkg       Specification of required package
   * \param selected  Corresponding selected pool item
   */
  void updateTo( const PackageSpec & pkg, const PoolItem & selected );

  /**
   * Set specified patch for installation.
   *
   * \param pkg            Specification of required patch (for feedback)
   * \param selected       Selected patch
   * \param ignore_pkgmgmt Whether to ignore the "affects package management"
   *                       flag. If false and the patch is flagged as such, this
   *                       method will do nothing and return false.
   * \return True if the patch was set for installation, false otherwise.
   */
  bool installPatch( const PackageSpec & pkg, const PoolItem & selected, bool ignore_pkgmgmt = true );

  // TODO provide also public versions of these, taking optional Options and
  // reporting errors via an output argument.

  void setToInstall( const PoolItem & pi );
  void setToRemove( const PoolItem & pi );
  void addRequirement( const PackageSpec & pkg );
  void addConflict( const PackageSpec & pkg );

  void addFeedback( const Feedback::Id id,
		    const PackageSpec & reqpkg,
		    const PoolItem & selected = PoolItem(),
		    const PoolItem & installed = PoolItem() )
  { _feedback.push_back( Feedback( id, reqpkg, selected, installed ) ); }

  void addFeedback( const Feedback::Id id_r, const PackageSpec & reqpkg_r, std::string userdata_r )
  { _feedback.push_back( Feedback( id_r, reqpkg_r, std::move(userdata_r) ) ); }

private:
  /** Various options to be applied to each requested package */
  Options _opts;

  /** Requester command being executed.
   * \note This may be different from command given on command line.
   */
  ZypperCommand _command;

  /** Various feedback from the requester. */
  std::vector<Feedback> _feedback;

  std::set<PoolItem> _toinst;
  std::set<PoolItem> _toremove;
  std::set<Capability> _requires;
  std::set<Capability> _conflicts;
};

#endif /* SOLVERREQUESTER_H_ */
