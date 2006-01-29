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
    os << "[Install: ";
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

QueueItemInstall::QueueItemInstall (const ResPool & pool, PoolItem_Ref item)
    : QueueItem (QUEUE_ITEM_TYPE_INSTALL, pool)
    , _item (item)
    , _channel_priority (0)
    , _other_penalty (0)
    , _explicitly_requested (false)
{
    // check if this install upgrades anything
    _upgrades = Helper::findInstalledItem (pool, item);

    DBG << "QueueItemInstall::QueueItemInstall(" << item << ") upgrades " << _upgrades << endl;
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

// Handle system item's that conflict with us -> uninstall them

struct BuildConflictList : public resfilter::OnCapMatchCallbackFunctor
{
    PoolItemList items;

    bool operator()( PoolItem_Ref provider, const Capability & match )
    {
	items.push_front (provider);
	return true;
    }
};


//---------------------------------------------------------------------------

// Handle items which freshen us -> re-establish them

struct EstablishFreshens : public resfilter::OnCapMatchCallbackFunctor
{
    const ResPool & pool;
    QueueItemList & qil;

    EstablishFreshens (const ResPool & p, QueueItemList &l)
	: pool (p)
	, qil (l)
    { }


    // provider has a freshens on a just to-be-installed item
    //   re-establish provider, maybe its incomplete now

    bool operator()( PoolItem_Ref provider, const Capability & match )
    {
	DBG << "EstablishFreshens (" << provider << ", " << match << ")" << endl;

	QueueItemEstablish_Ptr establish_item = new QueueItemEstablish (pool, provider);
	qil.push_front (establish_item);
	return true;
    }
};


//---------------------------------------------------------------------------------------

bool
QueueItemInstall::process (ResolverContext_Ptr context, QueueItemList & qil)
{
    DBG <<  "QueueItemInstall::process(" << *this << ")" << endl;

    ResStatus status = context->getStatus(_item);

    /* If we are trying to upgrade item A with item B and they both have the
	same version number, do nothing.  This shouldn't happen in general with
	zypp, but can come up with the installer & autopull. */

    if (_upgrades
	&& compareByNVRA(_item.resolvable(), _upgrades.resolvable()) == 0)
    {
	ResolverInfo_Ptr info;

	DBG << "install upgrades itself, skipping" << endl;

	info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_SKIPPING, _item, RESOLVER_INFO_PRIORITY_VERBOSE);
	context->addInfo (info);
	goto finished;
    }

    // check if this install is still needed
    //   (maybe other resolver processing made this install obsolete

    if (!_needed_by.empty()) {
	bool still_needed = false;

	DBG <<  "still needed " << endl;

	for (PoolItemList::const_iterator iter = _needed_by.begin(); iter != _needed_by.end() && !still_needed; ++iter) {
	    ResStatus status = iter->status();
	    DBG << "by: [status: " << status << "] " << *iter << endl;
	    if (! status.isToBeUninstalled()) {
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
	&& status.isToBeUninstalled()
	&& !_needed_by.empty()) {

	QueueItemUninstall_Ptr uninstall_item;

	for (PoolItemList::const_iterator iter = _needed_by.begin(); iter != _needed_by.end(); iter++) {
	    uninstall_item = new QueueItemUninstall (pool(), *iter, QueueItemUninstall::BACKOUT);
	    qil.push_front (uninstall_item);
	}

	goto finished;
    }

    // if this install upgrades an installed resolvable, explicitly uninstall this one
    //   in order to ensure that all dependencies are still met after the upgrade

    if (!_upgrades) {

	    DBG  << "simple install of " <<  _item  << endl;
	    context->install (_item, context->verifying() /* is_soft */, _other_penalty);

    } else {

	QueueItemUninstall_Ptr uninstall_item;

	DBG << "upgrade install of " << _item << endl;

	context->upgrade (_item, _upgrades, context->verifying() /* is_soft */, _other_penalty);

	// the upgrade will uninstall the installed one, take care of this

	uninstall_item = new QueueItemUninstall (pool(), _upgrades, QueueItemUninstall::UPGRADE);
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

    if (! (status.isUninstalled()
	|| status.isToBeUninstalledDueToUnlink()
	|| status.isIncomplete()
	|| status.isSatisfied()))
    {
	goto finished;
    }

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
	    DBG << "this requires " << cap << endl;

	    if (!context->requirementIsMet (cap)) {
		DBG << "this requirement is still unfulfilled" << endl;
		QueueItemRequire_Ptr req_item = new QueueItemRequire (pool(), cap);
		req_item->addPoolItem (_item);
		qil.push_front (req_item);
	    }

	}

	/* Construct conflict items for each of the item's conflicts. */

	caps = _item->dep (Dep::CONFLICTS);
	for (CapSet::const_iterator iter = caps.begin(); iter != caps.end(); iter++) {

	    const Capability cap = *iter;
	    DBG << "this conflicts with '" << cap << "'" << endl;
	    QueueItemConflict_Ptr conflict_item = new QueueItemConflict (pool(), cap, _item);
	    qil.push_front (conflict_item);

	}

	/* Construct conflict items for each of the item's obsoletes. */

	caps = _item->dep (Dep::OBSOLETES);
	for (CapSet::const_iterator iter = caps.begin(); iter != caps.end(); iter++) {

	    const Capability cap = *iter;
	    DBG << "this obsoletes " <<  cap << endl;
	    QueueItemConflict_Ptr conflict_item = new QueueItemConflict (pool(), cap, _item);
	    conflict_item->setActuallyAnObsolete();
	    qil.push_front (conflict_item);

	}

	/* Construct uninstall items for system item's that conflict with us. */

	BuildConflictList conflicts;
	caps = _item->dep (Dep::PROVIDES);
	for (CapSet::const_iterator iter = caps.begin(); iter != caps.end(); iter++) {
	    const Capability cap = *iter;

	    // pool()->foreachConflictingResItem (dep, build_conflict_list, &conflicts);

	    Dep dep( Dep::CONFLICTS);
	    invokeOnEach( pool().byCapabilityIndexBegin( cap.index(), dep ), // begin()
			  pool().byCapabilityIndexEnd( cap.index(), dep ),   // end()
			  resfilter::callOnCapMatchIn( dep, cap, functor::functorRef<bool,PoolItem,Capability>(conflicts)) );
	}

	for (PoolItemList::const_iterator iter = conflicts.items.begin(); iter != conflicts.items.end(); ++iter) {

	    PoolItem_Ref conflicting_item = *iter;
	    ResolverInfo_Ptr log_info;
	    QueueItemUninstall_Ptr uninstall_item;

	    /* Check to see if we conflict with ourself and don't create
	     * an uninstall item for it if we do.  This is Debian's way of
	     * saying that one and only one item with this provide may
	     * exist on the system at a time.
	     */

	    if (compareByNVR (conflicting_item.resolvable(), _item.resolvable()) == 0) {
		continue;
	    }

	    DBG << "because: '" << conflicting_item << "'" << endl;

	    uninstall_item = new QueueItemUninstall (pool(), conflicting_item, QueueItemUninstall::CONFLICT);
	    uninstall_item->setDueToConflict ();
	    log_info = new ResolverInfoConflictsWith (conflicting_item, _item);
	    uninstall_item->addInfo (log_info);
	    qil.push_front (uninstall_item);
	}


	/* Construct establish items for each of those which freshen this resolvable. */

	EstablishFreshens info( pool(), qil );

	CapFactory factory;
	Capability cap = factory.parse (_item->kind(), _item->name(), Rel::EQ, _item->edition());
	// pool ()->foreachFresheningResItem (cap, establish_freshens_cb, &info);
	Dep dep( Dep::FRESHENS);
	invokeOnEach( pool().byCapabilityIndexBegin( cap.index(), dep ), // begin()
		      pool().byCapabilityIndexEnd( cap.index(), dep ),   // end()
		      resfilter::callOnCapMatchIn( dep, cap, functor::functorRef<bool,PoolItem,Capability>(info)) );

    } // end of block

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

