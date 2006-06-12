/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* QueueItemInstall.cc
 *
 * Copyright (C) 2000-2002 Ximian, Inc.
 * Copyright (C) 2005 SUSE Linux Products GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include "zypp/CapFactory.h"
#include "zypp/CapSet.h"
#include "zypp/Package.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"

#include "zypp/base/Algorithm.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"

#include "zypp/solver/detail/QueueItemInstall.h"
#include "zypp/solver/detail/QueueItemEstablish.h"
#include "zypp/solver/detail/QueueItemUninstall.h"
#include "zypp/solver/detail/QueueItemRequire.h"
#include "zypp/solver/detail/QueueItemConflict.h"
#include "zypp/solver/detail/QueueItem.h"
#include "zypp/solver/detail/ResolverContext.h"
#include "zypp/solver/detail/ResolverInfoConflictsWith.h"
#include "zypp/solver/detail/ResolverInfoMisc.h"
#include "zypp/solver/detail/ResolverInfoNeededBy.h"
#include "zypp/solver/detail/Helper.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

using namespace std;

IMPL_PTR_TYPE(QueueItemInstall);

//---------------------------------------------------------------------------

std::ostream &
QueueItemInstall::dumpOn( std::ostream & os ) const
{
    os << "[" << (_soft?"Soft":"") << "Install: ";
    os << _item;
    if (_upgrades) {
	os << ", Upgrades ";
	os << _upgrades;
    }
    if (!_deps_satisfied_by_this_install.empty()) {
	os << ", Satisfies [";
	for (CapSet::const_iterator iter = _deps_satisfied_by_this_install.begin();
	    iter != _deps_satisfied_by_this_install.end(); iter++)
	{
	    if (iter != _deps_satisfied_by_this_install.begin()) os << ", ";
	    os << (*iter);
	}
	os << "]";
    }
    if (!_needed_by.empty()) {
	os << ", Needed by ";
	for (PoolItemList::const_iterator it = _needed_by.begin(); it != _needed_by.end(); ++it) {
	    if (it != _needed_by.begin()) os << ", ";
	    os << *it;
	}
    }
    if (_explicitly_requested) os << ", Explicit !";
    os << "]";
    return os;
}

//---------------------------------------------------------------------------

QueueItemInstall::QueueItemInstall (const ResPool & pool, PoolItem_Ref item, bool soft)
    : QueueItem (QUEUE_ITEM_TYPE_INSTALL, pool)
    , _item (item)
    , _soft (soft)
    , _channel_priority (0)
    , _other_penalty (0)
    , _explicitly_requested (false)
{
    Resolvable::constPtr res = item.resolvable();

    // Atoms are by default parallel installable (cf #181103)
    bool install_in_parallel = isKind<Atom>( res );

    // if its not an atom, check if its a package with 'install-only' set
    if (!install_in_parallel) {
	Package::constPtr pkg = asKind<Package>( res );
	install_in_parallel = (pkg != NULL) && pkg->installOnly();
    }

    // if its not parallel installable
    //   check if this install upgrades anything

    if (!install_in_parallel) {
	_upgrades = Helper::findInstalledItem (pool, item);
    }

    _XDEBUG("QueueItemInstall::QueueItemInstall(" << item << (soft?", soft":"") << ") upgrades " << _upgrades);
}
 

QueueItemInstall::~QueueItemInstall()
{
}

//---------------------------------------------------------------------------

bool
QueueItemInstall::isSatisfied (ResolverContext_Ptr context) const
{
    return context->isPresent (_item);
}


//---------------------------------------------------------------------------

// Handle items which freshen or supplement us -> re-establish them

// see also FreshenState in Resolver.cc
// see also UninstallEstablishItem in QueueItemUninstall.cc

typedef map<string, PoolItem_Ref> EstablishMap;

struct InstallEstablishItem
{
    EstablishMap establishmap;

    InstallEstablishItem ()
    { }


    // provider has a freshens on a just to-be-installed item
    //   re-establish provider, maybe its incomplete now

    bool operator()( const CapAndItem & cai )
    {
	_XDEBUG("QueueItemInstall::InstallEstablishItem (" << cai.item << ", " << cai.cap << ")");

	// only consider best architecture, best edition

	PoolItem_Ref item( cai.item );

	EstablishMap::iterator it = establishmap.find( item->name() );

	if (it != establishmap.end()) {					// item with same name found
	    int cmp = it->second->arch().compare( item->arch() );
	    if (cmp < 0) {						// new item has better arch
		it->second = item;
	    }
	    else if (cmp == 0) {					// new item has equal arch
		if (it->second->edition().compare( item->edition() ) < 0) {
		    it->second = item;				// new item has better edition
		}
	    }
	}
	else {
	    establishmap[item->name()] = item;
	}
	return true;
    }
};



//---------------------------------------------------------------------------

// Handle items which conflict us -> uninstall them

struct UninstallConflicting
{
    ResolverContext_Ptr _context;
    const Capability _provided_cap;
    PoolItem _install_item;		// the to-be-installed item issuing the conflict
    PoolItem _upgrade_item;		// the installed, to-be-upgraded item (might be empty if its a fresh install)
    QueueItemList & _qil;
    bool ignored;

    UninstallConflicting( ResolverContext_Ptr ctx, const Capability & provided_cap, PoolItem install_item, PoolItem upgrade_item, QueueItemList & qil )
	: _context( ctx )
	, _provided_cap( provided_cap )
	, _install_item( install_item )
	, _upgrade_item( upgrade_item )
	, _qil( qil )
	, ignored( false )
    { }


    // conflicting_item provides a capability (conflicting_cap), _install_item lists as conflicts.

    bool operator()( const CapAndItem & cai)
    {
	PoolItem_Ref conflicting_item = cai.item;

	_XDEBUG("UninstallConflicting(" << conflicting_item << ", cap " << cai.cap << ")");

	if (conflicting_item == _install_item) {				// self conflict ?
	    WAR << "Ignoring self-conflicts" << endl;
	    return true;
	}
	if (conflicting_item == _upgrade_item) {				// upgrade conflict ?
	    _XDEBUG("We're upgrading the conflicting item");
	    return true;
	}

	const Capability conflicting_cap = cai.cap;
	ResolverInfo_Ptr log_info;
	QueueItemUninstall_Ptr uninstall_item;

	IgnoreMap ignoreMap = _context->getIgnoreConflicts();		// ignored conflict ?
	// checking for ignoring dependencies
	for (IgnoreMap::iterator it = ignoreMap.begin(); it != ignoreMap.end(); it++) {
	    if (it->first == conflicting_item
		&& it->second == conflicting_cap)
	    {
		_XDEBUG("Found ignoring conflicts " << conflicting_cap << " for " << conflicting_item);
		ignored = true;
		return false;		// stop iteration
	    } else {
		_XDEBUG("Ignoring conflict " << it->second << " for " <<  it->first << " does not fit");	    
	    }
	}

	/* Check to see if we conflict with ourself and don't create
	 * an uninstall item for it if we do.  This is Debian's way of
	 * saying that one and only one item with this provide may
	 * exist on the system at a time.
	 */

	if (compareByNVR (conflicting_item.resolvable(), _install_item.resolvable()) == 0) {
		return true;
	}

#warning Make behaviour configurable
	// If the package is installed or is set to be installed by the user,
	// let the user decide deleting conflicting package. This is only an info.
	// Try at first updating packages.
	//
	ResStatus confl_status = _context->getStatus( conflicting_item );
	if (confl_status.isToBeInstalled()			// scheduled for installation
	    || confl_status.staysInstalled())			// not scheduled at all but installed
	{
	    ResolverInfoMisc_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_CONFLICT_CANT_INSTALL,
									       _install_item, RESOLVER_INFO_PRIORITY_VERBOSE, _provided_cap);
	    misc_info->setOtherPoolItem (conflicting_item);
	    misc_info->setOtherCapability (conflicting_cap);
	    _context->addInfo (misc_info);
	}

	_XDEBUG("because: '" << conflicting_item << "'conflicts " << conflicting_cap);

	QueueItemUninstall_Ptr uninstall_qitem = new QueueItemUninstall (_context->pool(), conflicting_item, QueueItemUninstall::CONFLICT);
	uninstall_qitem->setDueToConflict ();
	log_info = new ResolverInfoConflictsWith (conflicting_item, _install_item,
						  conflicting_cap);
	uninstall_qitem->addInfo (log_info);
	_qil.push_front (uninstall_qitem);

	return true;
    }
};

//---------------------------------------------------------------------------------------

bool
QueueItemInstall::process (ResolverContext_Ptr context, QueueItemList & qil)
{
    ResStatus status = context->getStatus(_item);

    _XDEBUG( "QueueItemInstall::process(" << *this << "):" << status);

    /* If we are trying to upgrade item A with item B and they both have the
	same version number, do nothing.  This shouldn't happen in general with
	zypp, but can come up with the installer & autopull. */

    if (_upgrades
	&& compareByNVRA(_item.resolvable(), _upgrades.resolvable()) == 0)
    {
	if (_item->kind() == ResTraits<Package>::kind) {
	    ResolverInfo_Ptr info;
	    _DEBUG("install of " << _item << " upgrades itself, skipping");

	    info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_SKIPPING, _item, RESOLVER_INFO_PRIORITY_VERBOSE);
	    context->addInfo (info);
	    goto finished;
	}
	else {
	    _DEBUG("re-install " << _item);
	}
    }

    // check if this install is still needed
    //   (maybe other resolver processing made this install obsolete

    if (!_needed_by.empty()) {
	bool still_needed = false;

	_XDEBUG( "still needed ");

	for (PoolItemList::const_iterator iter = _needed_by.begin(); iter != _needed_by.end() && !still_needed; ++iter) {
	    ResStatus status = iter->status();
	    _XDEBUG("by: [status: " << status << "] " << *iter);
	    if (! status.isToBeUninstalled()
		&& ! status.isImpossible())
	    {
		still_needed = true;
	    }
	}

	if (! still_needed)
	    goto finished;
    }

    /* If we are in verify mode and this install is about to fail, don't let it happen...
	   instead, we try to back out of the install by removing whatever it was that
	   needed this. */

    if (context->verifying()
	&& (status.isToBeUninstalled() || status.isImpossible())
	&& !_needed_by.empty()) {

	QueueItemUninstall_Ptr uninstall_item;

	for (PoolItemList::const_iterator iter = _needed_by.begin(); iter != _needed_by.end(); iter++) {
	    uninstall_item = new QueueItemUninstall (pool(), *iter, QueueItemUninstall::BACKOUT);
	    qil.push_front (uninstall_item);
	}

	goto finished;
    }

    // If this install is due to a needed, convert it to a normal install

    if (status.isNeeded()) {
	context->setStatus (_item, _soft ? ResStatus::toBeInstalledSoft :  ResStatus::toBeInstalled);
    }


    // if this install upgrades an installed resolvable, explicitly uninstall this one
    //   in order to ensure that all dependencies are still met after the upgrade

    if (!_upgrades) {

	_XDEBUG("trying simple install of " <<  _item);
	if (!context->install (_item, context->verifying() || _soft, _other_penalty))
	    goto finished;

    }
    else {

	QueueItemUninstall_Ptr uninstall_item;

	_XDEBUG("trying upgrade install of " << _item);

	if (!context->upgrade (_item, _upgrades, context->verifying() || _soft, _other_penalty)) {
	    // invalid solution
	    ResolverInfo_Ptr info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_INVALID_SOLUTION, PoolItem_Ref(), RESOLVER_INFO_PRIORITY_VERBOSE);
	    context->addError (info);
	    goto finished;
	}

	// the upgrade will uninstall the installed one, take care of this

	uninstall_item = new QueueItemUninstall (pool(), _upgrades, QueueItemUninstall::UPGRADE );
	uninstall_item->setUpgradedTo (_item);

	if (_explicitly_requested)
	    uninstall_item->setExplicitlyRequested ();

	qil.push_front (uninstall_item);

    }

    /* Log which item need this install */

    if (!_needed_by.empty()) {

	ResolverInfoNeededBy_Ptr info;

	info = new ResolverInfoNeededBy (_item);
	info->addRelatedPoolItemList (_needed_by);
	context->addInfo (info);
    }

    // we're done if this isn't currently uninstalled or incomplete

    if (! (status.staysUninstalled()
	   || status.isToBeUninstalledDueToUnlink()
	   || status.isIncomplete()
	   || status.isSatisfied()))
    {
	_XDEBUG("status " << status << " -> finished");
	goto finished;
    }

    _XDEBUG("status " << status << " -> NOT finished");
    {	// just a block for local initializers, the goto above makes this necessary

	ResolverInfoMisc_Ptr misc_info;

	if (_upgrades) {

	    misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_UPDATING, _item, RESOLVER_INFO_PRIORITY_VERBOSE);
	    misc_info->setOtherPoolItem (_upgrades);

	} else {

	    misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_INSTALLING, _item, RESOLVER_INFO_PRIORITY_VERBOSE);

	}

	context->addInfo (misc_info);
	logInfo (context);

	/* Construct require items for each of the item's requires that is still unsatisfied. */

	CapSet caps;

	caps = _item->dep (Dep::REQUIRES);

	for (CapSet::const_iterator iter = caps.begin(); iter != caps.end(); iter++) {

	    const Capability cap = *iter;
	    _XDEBUG("this requires " << cap);

	    if (!context->requirementIsMet (cap)) {
		_XDEBUG("this requirement is still unfulfilled");
		QueueItemRequire_Ptr req_item = new QueueItemRequire (pool(), cap );
		req_item->addPoolItem (_item);
		qil.push_front (req_item);
	    }

	}

	caps = _item->dep (Dep::RECOMMENDS);

	for (CapSet::const_iterator iter = caps.begin(); iter != caps.end(); iter++) {

	    const Capability cap = *iter;
	    _XDEBUG("this recommends " << cap);

	    if (!context->requirementIsMet (cap)) {
		_XDEBUG("this recommends is still unfulfilled");
		QueueItemRequire_Ptr req_item = new QueueItemRequire (pool(), cap, true);	// this is a soft requires
		req_item->addPoolItem (_item);
		qil.push_front (req_item);
	    }

	}

	/* Construct conflict items for each of the item's conflicts. */

	caps = _item->dep (Dep::CONFLICTS);
	for (CapSet::const_iterator iter = caps.begin(); iter != caps.end(); iter++) {
	    const Capability cap = *iter;
	    _XDEBUG("this conflicts with '" << cap << "'");
	    QueueItemConflict_Ptr conflict_item = new QueueItemConflict (pool(), cap, _item );
	    // Push the QueueItem at the END of the list in order to favourite conflicts caused
	    // by obsolating this item.
	    qil.push_back (conflict_item);
	}

	/* Construct conflict items for each of the item's obsoletes. */

	caps = _item->dep (Dep::OBSOLETES);
	IgnoreMap ignoreMap = context->getIgnoreObsoletes();
	
	for (CapSet::const_iterator iter = caps.begin(); iter != caps.end(); iter++) {
	    const Capability cap = *iter;
	    bool found = false;
	    for (IgnoreMap::iterator it = ignoreMap.begin();
		 it != ignoreMap.end(); it++) {
		if (it->first == _item
		    && it->second == cap) {
		    _XDEBUG("Found ignoring obsoletes " << cap << " for " << _item);
		    found = true;
		    break;
		}
	    }
	    if (!found) {	    
		_XDEBUG("this obsoletes " <<  cap);
		QueueItemConflict_Ptr conflict_item = new QueueItemConflict (pool(), cap, _item );
		conflict_item->setActuallyAnObsolete();
		// Push the QueueItem at the BEGIN of the list in order to favourite this confict
		// comparing to "normal" conflicts, cause this item will be deleted. So other
		// conflicts will not be regarded in the future.
		qil.push_front (conflict_item);
	    }
	}

	// Go over each provides of the to-be-uninstalled item and
	// - re-establish any freshens
	// - re-establish any supplements
	// - find items that conflict with us and try to uninstall it if it is useful

	InstallEstablishItem establish;

	caps = _item->dep (Dep::PROVIDES);
	bool ignored = false;

	for (CapSet::const_iterator iter = caps.begin(); iter != caps.end(); iter++) {
	    const Capability cap = *iter;

	    /* Construct establish items for each of those which
		freshen or supplement and provides of this resolvable. */

	    _XDEBUG("Re-establish all freshens on " << cap);
	    // pool ()->foreachFresheningResItem (cap, establish_freshens_cb, &info);

	    Dep dep( Dep::FRESHENS);
	    invokeOnEach( pool().byCapabilityIndexBegin( cap.index(), dep ), // begin()
			  pool().byCapabilityIndexEnd( cap.index(), dep ),   // end()
			  resfilter::ByCapMatch( cap ),
			  functor::functorRef<bool,CapAndItem>( establish ) );

	    dep = Dep::SUPPLEMENTS;
	    invokeOnEach( pool().byCapabilityIndexBegin( cap.index(), dep ), // begin()
			  pool().byCapabilityIndexEnd( cap.index(), dep ),   // end()
			  resfilter::ByCapMatch( cap ),
			  functor::functorRef<bool,CapAndItem>( establish ) );

	    if (!ignored) {
		// Search items that conflict with us and try to uninstall it if it is useful

		UninstallConflicting info( context, cap, _item, _upgrades, qil );

		Dep dep( Dep::CONFLICTS );
		invokeOnEach( pool().byCapabilityIndexBegin( cap.index(), dep ),
			      pool().byCapabilityIndexEnd( cap.index(), dep ),
			      resfilter::ByCapMatch( cap ),
			      functor::functorRef<bool,CapAndItem>( info ) );

		ignored = info.ignored;		// user choose to ignore these conflitcs
	    }

	} // iterate over all provides

	// schedule all collected items for establish

	for (EstablishMap::iterator it = establish.establishmap.begin(); it != establish.establishmap.end(); ++it) {
	    QueueItemEstablish_Ptr establish_item = new QueueItemEstablish (pool(), it->second, true);
	    qil.push_front( establish_item );
	}

    } // end of goto-over-definitions-to-finished block

 finished:

    return true;
}


QueueItem_Ptr
QueueItemInstall::copy (void) const
{
    QueueItemInstall_Ptr new_install = new QueueItemInstall (pool(), _item);
    new_install->QueueItem::copy(this);

    new_install->_upgrades = _upgrades;
    new_install->_deps_satisfied_by_this_install = CapSet(_deps_satisfied_by_this_install.begin(), _deps_satisfied_by_this_install.end());
    new_install->_needed_by = PoolItemList (_needed_by.begin(), _needed_by.end());
    new_install->_channel_priority = _channel_priority;
    new_install->_other_penalty = _other_penalty;
    new_install->_explicitly_requested = _explicitly_requested;

    return new_install;
}


int
QueueItemInstall::cmp (QueueItem_constPtr item) const
{
    int cmp = this->compare (item);
    if (cmp != 0)
	return cmp;
    QueueItemInstall_constPtr install = dynamic_pointer_cast<const QueueItemInstall>(item);
    return compareByNVRA (_item.resolvable(), install->_item.resolvable());
}

//---------------------------------------------------------------------------

void
QueueItemInstall::addDependency (const Capability & dep)
{
    _deps_satisfied_by_this_install.insert (dep);
}


void
QueueItemInstall::addNeededBy (PoolItem_Ref item)
{
    _needed_by.push_front (item);
}

///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////

