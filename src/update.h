#include "zypp/PoolItem.h"

#include "Zypper.h"

#include "utils/misc.h"

/**
 * Are there applicable patches?
 */
void patch_check();

/**
 * Lists available updates of installed resolvables of specified \a kind.
 * if repo_alias != "", restrict updates to this repository.
 * if best_effort == true, any version greater than the installed one will do.
 * Prints the table of updates to stdout.
 *
 * \param kind  resolvable type
 * \param best_effort
 */
void list_updates(Zypper & zypper,
                  const ResKindSet & kinds,
                  bool best_effort);

/**
 * List available fixes to all issues or issues specified in --bugzilla
 * or --cve options, or look for --issues[=str[ in numbers and descriptions
 */
void list_patches_by_issue(Zypper & zypper);

/**
 * Mark patches for installation according to bugzilla or CVE number specified
 * in --cve or --bugzilla or --bz.
 */
void mark_updates_by_issue(Zypper & zypper);

void selectable_update_report(Zypper & zypper, const zypp::ui::Selectable & s);
