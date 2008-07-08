#include "Zypper.h"

#include "utils.h"

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

/** \todo remove from this header after xu is dropped */ 
bool xml_list_patches();
/** \todo remove from this header after xu is dropped */
void xml_list_updates(const ResKindSet & kinds);

/**
 * \param kind  resolvable type
 * \param skip_interactive whether to skip updates that need user interaction
 * \param best_effort
 */
void mark_updates(Zypper & zypper,
                  const ResKindSet & kinds,
                  bool skip_interactive,
                  bool best_effort);
