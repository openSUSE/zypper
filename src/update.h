#ifndef ZYPPER_SRC_UPDATE_H
#define ZYPPER_SRC_UPDATE_H

#include <zypp/PoolItem.h>

#include "Zypper.h"
#include "issue.h"

#include "utils/misc.h"
#include "SolverRequester.h"

struct PatchSelector {
  std::set<Issue> _requestedIssues;
  std::set<std::string> _requestedPatchCategories;
  std::set<std::string> _requestedPatchSeverity;
  std::vector<zypp::Date> _requestedPatchDates;
};

/**
 * Are there applicable patches?
 */
void patch_check(bool updatestackOnly);

/**
 * Lists available updates of installed resolvables of specified \a kind.
 * if repo_alias != "", restrict updates to this repository.
 * if best_effort == true, any version greater than the installed one will do.
 * if idsOnly == true, only the names of the updates will be printed
 * Prints the table of updates to stdout.
 * \param kind  resolvable type
 * \param best_effort
 */
void list_updates(Zypper & zypper,
                  const ResKindSet & kinds,
                  bool best_effort,
                  bool all,
                  bool idsOnly,
                  const PatchSelector &patchSel_r = PatchSelector() );

/**
 * List available fixes to all issues or issues specified in --bugzilla
 * or --cve options, or look for --issue[=str[ in numbers and descriptions
 * (only printing the names of patches if idsOnly == true)
 */
void list_patches_by_issue(Zypper & zypper, bool all_r, bool idsOnly, const PatchSelector &sel );

/**
 * Mark patches for installation according to bugzilla or CVE number specified
 * in --cve or --bugzilla or --bz.
 */
void mark_updates_by_issue(Zypper & zypper, const std::set<Issue> &issues, SolverRequester::Options srOpts);

#endif
