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
 * \param kind  resolvable type
 * \param skip_interactive whether to skip updates that need user interaction
 * \param best_effort
 */
void mark_updates(Zypper & zypper,
                  const ResKindSet & kinds,
                  bool skip_interactive,
                  bool best_effort);

/**
 * Find best (according to edition) uninstalled item
 * with same kind/name/arch as \a item.
 *
 * Similar to zypp::solver::detail::Helper::findUpdateItem()
 * but allows changing the vendor and does not allow chaning arch.
 */
# warning get rid of findUpdateItem, make new API in zypp
zypp::PoolItem findUpdateItem(const zypp::ResPool & pool, const zypp::PoolItem item);

/**
 * Finds the best object in the Selectable.
 *
 * \todo FIXME if there is no installed object in the selectable, it chooses
 *       a random arch from the available objects (the availableBegin()). Choose
 *       the best compatible instead.
 * \todo All of this should be done in libzypp, using defined policies.
 */
# warning get rid of findTheBest, make new API in zypp
zypp::PoolItem findTheBest( const zypp::ResPool & pool, const zypp::ui::Selectable & s);
