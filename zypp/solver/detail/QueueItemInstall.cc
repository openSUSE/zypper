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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "zypp/solver/temporary/ResItem.h"
#include "zypp/solver/temporary/World.h"

#include "zypp/solver/detail/QueueItemInstall.h"
#include "zypp/solver/detail/QueueItemUninstall.h"
#include "zypp/solver/detail/QueueItemRequire.h"
#include "zypp/solver/detail/QueueItemConflict.h"
#include "zypp/solver/detail/QueueItem.h"
#include "zypp/solver/detail/ResolverContext.h"
#include "zypp/solver/detail/ResolverInfoConflictsWith.h"
#include "zypp/solver/detail/ResolverInfoMisc.h"
#include "zypp/solver/detail/ResolverInfoNeededBy.h"
#include "zypp/solver/detail/ResItemAndDependency.h"
#include "zypp/CapSet.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"

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

string
QueueItemInstall::asString ( void ) const
{
	return toString (*this);
}


string
QueueItemInstall::toString ( const QueueItemInstall & item)
{
	string ret = "[Install: ";
	ret += item._resItem->asString();
	if (item._upgrades != NULL) {
	    ret += ", Upgrades ";
	    ret += item._upgrades->asString();
	}
	if (!item._deps_satisfied_by_this_install.empty()) {
	    ret += ", Satisfies [";
	    for (CapSet::const_iterator iter = item._deps_satisfied_by_this_install.begin();
		   iter != item._deps_satisfied_by_this_install.end(); iter++) {
		  if (iter != item._deps_satisfied_by_this_install.begin()) ret += ", ";
		  ret += (*iter).asString();
	    }
	    ret += "]";
	}
	if (!item._needed_by.empty()) {
	    ret += ", Needed by ";
	    ret += ResItem::toString(item._needed_by);
	}
	if (item._explicitly_requested) ret += ", Explicit !";
	ret += "]";
	return ret;
}


ostream &
QueueItemInstall::dumpOn( ostream & str ) const
{
	str << asString();
	return str;
}


ostream&
operator<<( ostream& os, const QueueItemInstall & item)
{
	return os << item.asString();
}

//---------------------------------------------------------------------------

QueueItemInstall::QueueItemInstall (World_Ptr world, ResItem_constPtr resItem)
	: QueueItem (QUEUE_ITEM_TYPE_INSTALL, world)
	, _resItem (resItem)
	, _channel_priority (0)
	, _other_penalty (0)
	, _explicitly_requested (false)
{
	ResItem_constPtr upgrades = world->findInstalledResItem (resItem);
	_DBG("RC_SPEW") << "QueueItemInstall::QueueItemInstall(" << resItem->asString() << ") upgrades "
		<< ((upgrades!=NULL) ? upgrades->asString() : "nothing") << endl;
	if (upgrades
	    && ! (upgrades->equals (resItem))) {
	    setUpgrades(upgrades);
	}
}
 

QueueItemInstall::~QueueItemInstall()
{
}

//---------------------------------------------------------------------------

bool
QueueItemInstall::isSatisfied (ResolverContext_Ptr context) const
{
	return context->resItemIsPresent (_resItem);
}


//---------------------------------------------------------------------------

// Handle system resItem's that conflict with us -> uninstall them

static bool
build_conflict_list (ResItem_constPtr resItem, const Capability & dep, void *data)
{
	CResItemList *rl = (CResItemList *)data;
	rl->push_front (resItem);
	return true;
}

bool
QueueItemInstall::process (ResolverContext_Ptr context, QueueItemList & qil)
{
	_DBG("RC_SPEW") <<  "QueueItemInstall::process(" << this->asString() << ")" << endl;

	ResItem_constPtr resItem = _resItem;
	string res_name = resItem->asString();
	string msg;
	ResItemStatus status = context->getStatus (resItem);

	/* If we are trying to upgrade resItem A with resItem B and they both have the
	   same version number, do nothing.  This shouldn't happen in general with
	   red-carpet, but can come up with the installer & autopull. */

	if (_upgrades
	    && _resItem->equals (_upgrades)) {
	    ResolverInfo_Ptr info;

	    _DBG("RC_SPEW") << "upgrades equal resItem, skipping" << endl;

	    // Translator: %s = name of package,patch,...
	    msg = str::form (_("Skipping %s: already installed"),
			     res_name.c_str());
	    info = new ResolverInfoMisc (_resItem, RESOLVER_INFO_PRIORITY_VERBOSE, msg);
	    context->addInfo (info);
	    goto finished;
	}

	// check if this install is still needed
	//   (maybe other resolver processing made this install obsolete

	if (!_needed_by.empty()) {
	    bool still_needed = false;

	    _DBG("RC_SPEW") <<  "still needed " << endl;

	    for (CResItemList::const_iterator iter = _needed_by.begin(); iter != _needed_by.end() && !still_needed; iter++) {
	        ResItemStatus status = context->getStatus (*iter);
	        _DBG("RC_SPEW") << "by: [status: " << ResolverContext::toString(status) << "] " << (*iter)->asString() << endl;
	        if (! resItem_status_is_to_be_uninstalled (status)) {
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
	    && resItem_status_is_to_be_uninstalled (context->getStatus (resItem))
	    && !_needed_by.empty()) {

	    QueueItemUninstall_Ptr uninstall_item;

	    for (CResItemList::const_iterator iter = _needed_by.begin(); iter != _needed_by.end(); iter++) {
	        uninstall_item = new QueueItemUninstall (world(), *iter, "uninstallable resolvable");
	        qil.push_front (uninstall_item);
	    }

	    goto finished;
	}

	// if this install upgrades an installed resolvable, explicitly uninstall this one
	//   in order to ensure that all dependencies are still met after the upgrade

	if (_upgrades == NULL) {

	    _DBG("RC_SPEW")  << "simple install of " <<  resItem->asString(true) << endl;

	    context->installResItem (resItem, context->verifying(), /* is_soft */ _other_penalty);

	} else {

	    QueueItemUninstall_Ptr uninstall_item;

	    _DBG("RC_SPEW") << "upgrade install of " << resItem->asString() << endl;

	    context->upgradeResItem (resItem, _upgrades, context->verifying(), /* is_soft */ _other_penalty);

	    uninstall_item = new QueueItemUninstall (world(), _upgrades, "upgrade");
	    uninstall_item->setUpgradedTo (resItem);

	    if (_explicitly_requested)
	        uninstall_item->setExplicitlyRequested ();

	    qil.push_front (uninstall_item);
	}

	/* Log which resItem need this install */

	if (!_needed_by.empty()) {
	    ResolverInfoNeededBy_Ptr info;

	    info = new ResolverInfoNeededBy (resItem);
	    info->addRelatedResItemList (_needed_by);
	    context->addInfo (info);
	}

	// we're done if this isn't currently uninstalled or incomplete

	if (! (status == RESOLVABLE_STATUS_UNINSTALLED
	       || status == RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_UNLINK
	       || resItem_status_is_incomplete (status))) {
	    goto finished;
	}

	if (_upgrades != NULL) {
	    // Translator: 1.%s and 2.%s = name of package
	    msg = str::form (_("Upgrading %s => %s"),
			     _upgrades->asString().c_str(),
			     res_name.c_str());
	} else {
	    // Translator: %s = packagename
	    msg = str::form (_("Installing %s"),
			     res_name.c_str());
	}

	context->addInfoString (resItem, RESOLVER_INFO_PRIORITY_VERBOSE, msg);

	logInfo (context);

	/* Construct require items for each of the resItem's requires that is still unsatisfied. */
	{
	CapSet deps;

	deps = resItem->requires();

	for (CapSet::const_iterator iter = deps.begin(); iter != deps.end(); iter++) {
	    const Capability dep = *iter;
	    _DBG("RC_SPEW") << "this requires " << dep.asString() << endl;
	    if (!context->requirementIsMet (dep)) {
	        _DBG("RC_SPEW") << "this requires " << dep.asString() << endl;
	        QueueItemRequire_Ptr req_item = new QueueItemRequire (world(), dep);
	        req_item->addResItem (resItem);
	        qil.push_front (req_item);
	    }
	}

	/* Construct conflict items for each of the resItem's conflicts. */

	deps = resItem->conflicts();
	for (CapSet::const_iterator iter = deps.begin(); iter != deps.end(); iter++) {
	    const Capability dep = *iter;
	    _DBG("RC_SPEW") << "this conflicts with '" << dep.asString() << "'" << endl;
	    QueueItemConflict_Ptr conflict_item = new QueueItemConflict (world(), dep, resItem);
	    qil.push_front (conflict_item);
	}

	/* Construct conflict items for each of the resItem's obsoletes. */

	deps = resItem->obsoletes();
	for (CapSet::const_iterator iter = deps.begin(); iter != deps.end(); iter++) {
	    const Capability dep = *iter;
	    _DBG("RC_SPEW") << "this obsoletes " <<  dep.asString() << endl;
	    QueueItemConflict_Ptr conflict_item = new QueueItemConflict (world(), dep, resItem);
	    conflict_item->setActuallyAnObsolete();
	    qil.push_front (conflict_item);
	}

	/* Construct uninstall items for system resItem's that conflict with us. */

	CResItemList conflicts;
	deps = resItem->provides();
	for (CapSet::const_iterator iter = deps.begin(); iter != deps.end(); iter++) {
	    const Capability dep = *iter;
	    world()->foreachConflictingResItem (dep, build_conflict_list, &conflicts);
	}

	for (CResItemList::const_iterator iter = conflicts.begin(); iter != conflicts.end(); iter++) {
	    ResItem_constPtr conflicting_resItem = *iter;
	    ResolverInfo_Ptr log_info;
	    QueueItemUninstall_Ptr uninstall_item;

	    /* Check to see if we conflict with ourself and don't create
	     * an uninstall item for it if we do.  This is Debian's way of
	     * saying that one and only one resItem with this provide may
	     * exist on the system at a time.
	     */
	    if (conflicting_resItem->equals (resItem)) {
	        continue;
	    }

	    _DBG("RC_SPEW") << "because: '" << conflicting_resItem->asString(true) << "'" << endl;

	    uninstall_item = new QueueItemUninstall (world(), conflicting_resItem, "conflict");
	    uninstall_item->setDueToConflict ();
	    log_info = new ResolverInfoConflictsWith (conflicting_resItem, resItem);
	    uninstall_item->addInfo (log_info);
	    qil.push_front (uninstall_item);
	}
	}

 finished:

	return true;
}


QueueItem_Ptr
QueueItemInstall::copy (void) const
{
	QueueItemInstall_Ptr new_install = new QueueItemInstall (world(), _resItem);
	((QueueItem_Ptr)new_install)->copy((QueueItem_constPtr)this);

	new_install->_upgrades = _upgrades;
	new_install->_deps_satisfied_by_this_install = CapSet(_deps_satisfied_by_this_install.begin(), _deps_satisfied_by_this_install.end());
	new_install->_needed_by = CResItemList (_needed_by.begin(), _needed_by.end());
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
	return ResItem::compare (_resItem, install->_resItem);
}

//---------------------------------------------------------------------------

void
QueueItemInstall::addDependency (const Capability & dep)
{
	_deps_satisfied_by_this_install.insert (dep);
}


void
QueueItemInstall::addNeededBy (ResItem_constPtr resItem)
{
	_needed_by.push_front (resItem);
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

