/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "config.h"

#include <zypp/solver/detail/QueueItemInstall.h>
#include <zypp/solver/detail/QueueItemUninstall.h>
#include <zypp/solver/detail/QueueItemRequire.h>
#include <zypp/solver/detail/QueueItemConflict.h>
#include <zypp/solver/detail/QueueItem.h>
#include <zypp/solver/detail/ResolverContext.h>
#include <zypp/solver/detail/ResolverInfoConflictsWith.h>
#include <zypp/solver/detail/ResolverInfoMisc.h>
#include <zypp/solver/detail/ResolverInfoNeededBy.h>
#include <zypp/solver/detail/ResItem.h>
#include <zypp/solver/detail/Version.h>
#include <zypp/solver/detail/World.h>
#include <zypp/solver/detail/ResItemAndDependency.h>

///////////////////////////////////////////////////////////////////
namespace zypp {
//////////////////////////////////////////////////////////////////

using namespace std;

IMPL_DERIVED_POINTER(QueueItemInstall,QueueItem);

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
	ret += ", Satisfies ";
	ret += Dependency::toString(item._deps_satisfied_by_this_install);
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

QueueItemInstall::QueueItemInstall (WorldPtr world, constResItemPtr resItem)
    : QueueItem (QUEUE_ITEM_TYPE_INSTALL, world)
    , _resItem (resItem)
    , _channel_priority (0)
    , _other_penalty (0)
    , _explicitly_requested (false)
{
    constResItemPtr upgrades = world->findInstalledResItem (resItem);
    if (getenv("RC_SPEW")) fprintf (stderr, "QueueItemInstall::QueueItemInstall(%s) upgrades %s\n", resItem->asString().c_str(), upgrades!=NULL?upgrades->asString().c_str():"nothing");
    if (upgrades
	&& ! (((constSpecPtr)upgrades)->equals (resItem))) {
        setUpgrades(upgrades);
    }
}


QueueItemInstall::~QueueItemInstall()
{
}

//---------------------------------------------------------------------------

bool
QueueItemInstall::isSatisfied (ResolverContextPtr context) const
{
    return context->resItemIsPresent (_resItem);
}


//---------------------------------------------------------------------------

// Handle system resItem's that conflict with us -> uninstall them

static bool
build_conflict_list (constResItemPtr resItem, constDependencyPtr dep, void *data)
{
    CResItemList *rl = (CResItemList *)data;
    rl->push_front (resItem);
    return true;
}

bool
QueueItemInstall::process (ResolverContextPtr context, QueueItemList & qil)
{
    if (getenv ("RC_SPEW")) fprintf (stderr, "QueueItemInstall::process(%s)\n", this->asString().c_str());

    constResItemPtr resItem = _resItem;
    string pkg_name = resItem->asString();
    string msg;
    ResItemStatus status = context->getStatus (resItem);

    CDependencyList deps;
    CResItemList conflicts;

    /* If we are trying to upgrade resItem A with resItem B and they both have the
       same version number, do nothing.  This shouldn't happen in general with
       red-carpet, but can come up with the installer & autopull. */

    if (_upgrades
	&& ((constSpecPtr)_resItem)->equals (_upgrades)) {
	ResolverInfoPtr info;

	if (getenv ("RC_SPEW")) fprintf (stderr, "upgrades equal resItem, skipping\n");

	msg = string("Skipping ") + pkg_name + (": already installed");
	info = new ResolverInfoMisc (_resItem, RESOLVER_INFO_PRIORITY_VERBOSE, msg);
	context->addInfo (info);
	goto finished;
    }

    if (!_needed_by.empty()) {
	bool still_needed = false;

	if (getenv ("RC_SPEW")) fprintf (stderr, "still needed ");

	for (CResItemList::const_iterator iter = _needed_by.begin(); iter != _needed_by.end() && !still_needed; iter++) {
	    ResItemStatus status = context->getStatus (*iter);
	    if (getenv ("RC_SPEW")) fprintf (stderr, "by: [status: %s] %s\n", ResolverContext::toString(status).c_str(), (*iter)->asString().c_str());
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

	QueueItemUninstallPtr uninstall_item;

	for (CResItemList::const_iterator iter = _needed_by.begin(); iter != _needed_by.end(); iter++) {
	    uninstall_item = new QueueItemUninstall (world(), *iter, "uninstallable resolvable");
	    qil.push_front (uninstall_item);
	}
	
	goto finished;
    }

    if (_upgrades == NULL) {

	if (getenv ("RC_SPEW")) fprintf (stderr, "simple install of %s\n", resItem->asString(true).c_str());

	context->installResItem (resItem, context->verifying(), /* is_soft */ _other_penalty);

    } else {

	QueueItemUninstallPtr uninstall_item;

	if (getenv ("RC_SPEW")) fprintf (stderr, "upgrade install of %s\n", resItem->asString().c_str());

	context->upgradeResItem (resItem, _upgrades, context->verifying(), /* is_soft */ _other_penalty);

	uninstall_item = new QueueItemUninstall (world(), _upgrades, "upgrade");
	uninstall_item->setUpgradedTo (resItem);

	if (_explicitly_requested)
	    uninstall_item->setExplicitlyRequested ();

	qil.push_front (uninstall_item);
    }

    /* Log which resItem need this install */

    if (!_needed_by.empty()) {
	ResolverInfoNeededByPtr info;

	info = new ResolverInfoNeededBy (resItem);
	info->addRelatedResItemList (_needed_by);
	context->addInfo (info);
    }

    if (! (status == RESOLVABLE_STATUS_UNINSTALLED
	   || status == RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_UNLINK)) {
	goto finished;
    }

    if (_upgrades != NULL) {
	msg = string ("Upgrading ") + _upgrades->asString() + " => " + pkg_name;
    } else {
	msg = string ("Installing ") + pkg_name;
    }

    context->addInfoString (resItem, RESOLVER_INFO_PRIORITY_VERBOSE, msg);

    logInfo (context);

    /* Construct require items for each of the resItem's requires that is still unsatisfied. */

    deps = resItem->requires();
    for (CDependencyList::const_iterator iter = deps.begin(); iter != deps.end(); iter++) {
	constDependencyPtr dep = *iter;
	if (!context->requirementIsMet (dep, false)) {
	    if (getenv("RC_SPEW")) fprintf (stderr, "this requires %s\n", dep->asString().c_str());
	    QueueItemRequirePtr req_item = new QueueItemRequire (world(), dep);
	    req_item->addResItem (resItem);
	    qil.push_front (req_item);
	}
    }

    /* Construct conflict items for each of the resItem's conflicts. */

    deps = resItem->conflicts();
    for (CDependencyList::const_iterator iter = deps.begin(); iter != deps.end(); iter++) {
	constDependencyPtr dep = *iter;
	if (getenv("RC_SPEW")) fprintf (stderr, "this conflicts with '%s'\n", dep->asString().c_str());
	QueueItemConflictPtr conflict_item = new QueueItemConflict (world(), dep, resItem);
	qil.push_front (conflict_item);
    }

    /* Construct conflict items for each of the resItem's obsoletes. */

    deps = resItem->obsoletes();
    for (CDependencyList::const_iterator iter = deps.begin(); iter != deps.end(); iter++) {
	constDependencyPtr dep = *iter;
	if (getenv("RC_SPEW")) fprintf (stderr, "this obsoletes %s\n", dep->asString().c_str());
	QueueItemConflictPtr conflict_item = new QueueItemConflict (world(), dep, resItem);
	conflict_item->setActuallyAnObsolete();
	qil.push_front (conflict_item);
    }

    /* Construct uninstall items for system resItem's that conflict with us. */

    deps = resItem->provides();
    for (CDependencyList::const_iterator iter = deps.begin(); iter != deps.end(); iter++) {
	constDependencyPtr dep = *iter;
	world()->foreachConflictingResItem (dep, build_conflict_list, &conflicts);
    }

    for (CResItemList::const_iterator iter = conflicts.begin(); iter != conflicts.end(); iter++) {
	constResItemPtr conflicting_resItem = *iter;
	ResolverInfoPtr log_info;
	QueueItemUninstallPtr uninstall_item;

	/* Check to see if we conflict with ourself and don't create
	 * an uninstall item for it if we do.  This is Debian's way of
	 * saying that one and only one resItem with this provide may
	 * exist on the system at a time.
	 */
	if (((constSpecPtr)conflicting_resItem)->equals (resItem)) {
	    continue;
	}

	if (getenv("RC_SPEW")) fprintf (stderr, "because: '%s'\n", conflicting_resItem->asString(true).c_str());

	uninstall_item = new QueueItemUninstall (world(), conflicting_resItem, "conflict");
	uninstall_item->setDueToConflict ();
	log_info = new ResolverInfoConflictsWith (conflicting_resItem, resItem);
	uninstall_item->addInfo (log_info);
	qil.push_front (uninstall_item);
    }
    
 finished:
//FIXME    rc_queue_item_free (item);
    
    return true;
}


QueueItemPtr
QueueItemInstall::copy (void) const
{
    QueueItemInstallPtr new_install = new QueueItemInstall (world(), _resItem);
    ((QueueItemPtr)new_install)->copy((constQueueItemPtr)this);

    new_install->_upgrades = _upgrades;
    new_install->_deps_satisfied_by_this_install = CDependencyList(_deps_satisfied_by_this_install.begin(), _deps_satisfied_by_this_install.end());
    new_install->_needed_by = CResItemList (_needed_by.begin(), _needed_by.end());
    new_install->_channel_priority = _channel_priority;
    new_install->_other_penalty = _other_penalty;
    new_install->_explicitly_requested = _explicitly_requested;

    return new_install;
}


int
QueueItemInstall::cmp (constQueueItemPtr item) const
{
    int cmp = this->compare (item);
    if (cmp != 0)
	return cmp;
    constQueueItemInstallPtr install = item;
    return GVersion.compare (_resItem, install->_resItem);
}

//---------------------------------------------------------------------------

void 
QueueItemInstall::addDependency (constDependencyPtr dep)
{
    _deps_satisfied_by_this_install.push_front (dep);
}


void 
QueueItemInstall::addNeededBy (constResItemPtr resItem)
{
    _needed_by.push_front (resItem);
}

///////////////////////////////////////////////////////////////////
}; // namespace zypp
///////////////////////////////////////////////////////////////////

