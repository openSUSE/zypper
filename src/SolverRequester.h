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

#include "PackageArgs.h"
#include "utils/misc.h" // for ResKindSet; might make sense to move this elsewhere

/**
 * Issue various requests to the dependency resolver, based on given
 * arguments (\ref PackageArgs) and options (\ref SolverRequester::Options).
 */
class SolverRequester
{
public:
  struct Options
  {
    Options()
      : force(false)
      , force_by_cap(false)
      , force_by_name(false)
      , best_effort(false)
      , skip_interactive(false)
    {}

    /**
     * If true, the requester will force the operation in some defined cases,
     * even if the operation would otherwise not be allowed.
     *
     * \todo define the cases
     */
    bool force;

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

    /** Aliases of the repos from which the packages should be installed */
    std::list<std::string> from_repos;
  };

public:
  SolverRequester(const Options & opts = Options())
    : _opts(opts)
  {}

public:
  /** Request installation of specified objects. */
  void install(const PackageArgs & args);

  /** Request removal of specified objects. */
  void remove(const PackageArgs & args);

  /** Request update of specified objects. */
  void update(const PackageArgs & args);

  /** Request update of all satisfied patterns.
   * TODO define this */
  void updatePatterns();

  /**
   * Request installation of all needed patches.
   * If there are any needed patches flagged "affects package management", only
   * these get selected (need to call this again once these are installed).
   */
  void updatePatches();

private:
  void installRemove(const PackageArgs & args, bool doinst);
  void install(const zypp::Capability & cap, const std::string & repoalias);
  void remove(const zypp::Capability & cap);
  void update(const zypp::Capability & cap, const std::string & repoalias);

  /**
   * Set the best version of the patch for installation.
   *
   * \param s              The patch selectable.
   * \param ignore_pkgmgmt Whether to ignore the "affects package management"
   *                       flag. If false and the patch is flagged as such, this
   *                       method will do nothing and return false.
   * \return True if the patch was set for installation, false otherwise.
   */
  bool installPatch(zypp::ui::Selectable & s, bool ignore_pkgmgmt = true);

  // TODO provide also public versions of these, taking optional Options and
  // reporting errors via an output argument.

  Options _opts;
};

#endif /* SOLVERREQUESTER_H_ */
