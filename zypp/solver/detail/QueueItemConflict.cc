/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* QueueItemConflict.cc
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

#include <zypp/solver/detail/QueueItemConflict.h>
#include <zypp/solver/detail/QueueItemBranch.h>
#include <zypp/solver/detail/QueueItemInstall.h>
#include <zypp/solver/detail/QueueItemUninstall.h>
#include <zypp/solver/detail/QueueItem.h>
#include <zypp/solver/detail/Version.h>
#include <zypp/solver/detail/World.h>
#include <zypp/solver/detail/ResolverContext.h>
#include <zypp/solver/detail/ResolverInfoConflictsWith.h>
#include <zypp/solver/detail/ResolverInfoMisc.h>
#include <zypp/solver/detail/ResolverInfoObsoletes.h>

///////////////////////////////////////////////////////////////////
namespace ZYPP {
//////////////////////////////////////////////////////////////////

using namespace std;

IMPL_DERIVED_POINTER(QueueItemConflict,QueueItem);

//---------------------------------------------------------------------------

string
QueueItemConflict::asString ( void ) const
{
    return toString (*this);
}


string
QueueItemConflict::toString ( const QueueItemConflict & item)
{
    string res = "[Conflict: ";
    res += item._dep->asString();
    res += ", Triggered by ";
    res += item._conflicting_resItem->asString();
    if (item._actually_an_obsolete) res += ", Obsolete !";
    res += "]";
    return res;
}


ostream &
QueueItemConflict::dumpOn( ostream & str ) const
{
    str << asString();
    return str;
}


ostream&
operator<<( ostream& os, const QueueItemConflict & item)
{
    return os << item.asString();
}

//---------------------------------------------------------------------------

QueueItemConflict::QueueItemConflict (WorldPtr world, constDependencyPtr dep, constResItemPtr resItem)
    : QueueItem (QUEUE_ITEM_TYPE_CONFLICT, world)
    , _dep (dep)
    , _conflicting_resItem (resItem)
    , _actually_an_obsolete (false)
{
}


QueueItemConflict::~QueueItemConflict()
{
}

//---------------------------------------------------------------------------

#if PHI

// on conflict, try to find upgrade candidates for the installed resItem triggering the conflict
// there are cases where upgrading prevents the conflict
// rc tends to uninstall the resItem
// phi tends to upgrade the resItem
// testcases: exercise-02conflict-08-test.xml, exercise-02conflict-09-test.xml

typedef struct {
    ResolverContextPtr context;
    CResItemList upgrades;
} UpgradeCandidateInfo;


static bool
upgrade_candidates_cb (constResItemPtr resItem, constSpecPtr spec, void *data)
{
//fprintf (stderr, "upgrade_candidates_cb(%s,%s)\n", resItem->asString().c_str(), spec->asString().c_str());
    UpgradeCandidateInfo *info = (UpgradeCandidateInfo *)data;
    if (info->context->getStatus (resItem) == RESOLVABLE_STATUS_UNINSTALLED) {
	info->upgrades.push_back (resItem);
    }
    return true;
}

#endif  // PHI


typedef struct {
    WorldPtr world;
    constResItemPtr conflicting_resItem;
    constDependencyPtr dep;
    ResolverContextPtr context;
    QueueItemList & new_items;

    string pkg_str;
    string dep_str;

    bool actually_an_obsolete;
} ConflictProcessInfo;


static bool
conflict_process_cb (constResItemPtr resItem, constSpecPtr spec, void *data)
{
    ConflictProcessInfo *info = (ConflictProcessInfo *)data;
    ResItemStatus status;
    string pkg_str, spec_str, msg;
    ResolverInfoPtr log_info;

    if (getenv ("RC_SPEW")) fprintf (stderr, "conflict_process_cb (resItem[%s], spec[%s], info [%s]\n", resItem->asString().c_str(), spec->asString().c_str(), info->conflicting_resItem->asString().c_str());
    if (getenv ("RC_SPEW")) fprintf (stderr, "conflict_process_cb (resItem equals spec: %s, info->dep [%s]\n", resItem->equals(spec) ? "YES" : "NO", info->dep->asString().c_str());

    /* We conflict with ourself.  For the purpose of installing ourself, we
     * just ignore it, but it's Debian's way of saying that one and only one
     * resItem with this provide may exist on the system at a time. */

    if (info->conflicting_resItem
	&& resItem->equals (info->conflicting_resItem)) {
	return true;
    }

    /* FIXME: This should probably be a GVersion capability. */
    /* Obsoletes don't apply to virtual provides, only the resItems
     * themselves.  A provide is "virtual" if it's not the same spec
     * as the resItem that's providing it.  This, of course, only
     * applies to RPM, since it's the only one with obsoletes right
     * now. */

    if (info->actually_an_obsolete
	&& !(resItem->equals(spec)))
    {
	return true;
    }

    pkg_str = resItem->asString();
    spec_str = spec->asString();

    status = info->context->getStatus (resItem);

    if (getenv ("RC_SPEW")) fprintf (stderr, "conflict_process_cb (resItem[%s]<%s>\n", resItem->asString().c_str(), ResolverContext::toString(status).c_str());

    switch (status) {
	
    case RESOLVABLE_STATUS_INSTALLED:
    case RESOLVABLE_STATUS_TO_BE_INSTALLED_SOFT: {
	QueueItemUninstallPtr uninstall;
	ResolverInfoPtr log_info;

#if PHI
	// maybe an upgrade can resolve the conflict ?
	//        check if other resItem is available which upgrades

	// find non-installed packages which provide the conflicting name

	UpgradeCandidateInfo upgrade_info;
	upgrade_info.context = info->context;

	DependencyPtr maybe_upgrade_dep = new Dependency (resItem->name(), Relation::Any, Kind::Package, new Channel (CHANNEL_TYPE_NONSYSTEM), -1);
	info->world->foreachProvidingResItem (maybe_upgrade_dep, upgrade_candidates_cb, (void *)&upgrade_info);

#endif

	uninstall = new QueueItemUninstall (info->world, resItem, "conflict");
	uninstall->setDependency (info->dep);

	if (info->actually_an_obsolete) {
	    uninstall->setDueToObsolete ();
	    log_info = new ResolverInfoObsoletes (resItem, info->conflicting_resItem);
	} else {
	    uninstall->setDueToConflict ();
	    log_info = new ResolverInfoConflictsWith (resItem, info->conflicting_resItem);
	}

	uninstall->addInfo (log_info);

#if PHI
	if (upgrade_info.upgrades.empty ()) {
#endif

	    info->new_items.push_back (uninstall);

#if PHI
	}
	else {
	    // there are upgrade candidates for the conflicting resItem
	    // branch to: 1. uninstall, 2. upgrade (for each upgrading resItem)

	    QueueItemBranchPtr branch = new QueueItemBranch (info->world);

	    branch->addItem (uninstall);			// try uninstall

	    for (CResItemList::const_iterator iter = upgrade_info.upgrades.begin(); iter != upgrade_info.upgrades.end(); iter++) {
		QueueItemInstallPtr upgrade = new QueueItemInstall (info->world, *iter);
		upgrade->setUpgrades (resItem);
		branch->addItem (upgrade);			// try upgrade
	    }
	    info->new_items.push_back (branch);
	}
#endif

	break;
    }

    case RESOLVABLE_STATUS_TO_BE_INSTALLED: {
	msg = string ("A conflict over ") + info->dep_str + " (" + spec_str + ") requires the removal of the to-be-installed resItem " + pkg_str;

	ResolverInfoMiscPtr misc_info = new ResolverInfoMisc (resItem,RESOLVER_INFO_PRIORITY_VERBOSE, msg);

	misc_info->flagAsError ();
	if (info->conflicting_resItem) {
	    misc_info->addRelatedResItem (info->conflicting_resItem);
	}
	info->context->addInfo (misc_info);

	break;
    }

    case RESOLVABLE_STATUS_UNINSTALLED: {
	info->context->setStatus (resItem, RESOLVABLE_STATUS_TO_BE_UNINSTALLED);
	msg = string ("Marking ") + pkg_str + " as uninstallable due to conflicts over " + info->dep_str + " (" + spec_str + ")";
	if (!(info->pkg_str.empty())) {
	    msg += " from ";
	    msg += info->pkg_str;
	}

	ResolverInfoMiscPtr misc_info = new ResolverInfoMisc (NULL, RESOLVER_INFO_PRIORITY_VERBOSE, msg);

	misc_info->addRelatedResItem (resItem);
	if (info->conflicting_resItem) {
	    misc_info->addRelatedResItem(info->conflicting_resItem);
	}
	info->context->addInfo (misc_info);

	break;
    }

    case RESOLVABLE_STATUS_TO_BE_UNINSTALLED:
    case RESOLVABLE_STATUS_TO_BE_UNINSTALLED_DUE_TO_OBSOLETE:
	/* This is the easy case -- we do nothing. */
	break;

    default:
	abort();
    }

    return true;
}


bool
QueueItemConflict::process (ResolverContextPtr context, QueueItemList & new_items)
{
    if (getenv ("RC_SPEW")) fprintf (stderr, "QueueItemConflict::process(%s)\n", this->asString().c_str());

    ConflictProcessInfo info = { world(), _conflicting_resItem, _dep, context, new_items, "", "", _actually_an_obsolete };

    if (_conflicting_resItem) {
	info.pkg_str = _conflicting_resItem->asString();
    }

    info.dep_str = _dep->relation().asString() + " " + ((constSpecPtr)_dep)->asString();

    world()->foreachProvidingResItem (_dep, conflict_process_cb, (void *)&info);
					
// FIXME: free self

    return true;
}


//---------------------------------------------------------------------------

QueueItemPtr
QueueItemConflict::copy (void) const
{
    QueueItemConflictPtr new_conflict = new QueueItemConflict (world(), _dep, _conflicting_resItem);
    ((QueueItemPtr)new_conflict)->copy((constQueueItemPtr)this);

    // _actually_an_obsolete is not being copied !

    return new_conflict;
}


int
QueueItemConflict::cmp (constQueueItemPtr item) const
{
    int cmp = this->compare (item);		// assures equal type
    if (cmp != 0)
	return cmp;

    constQueueItemConflictPtr conflict = item;
    cmp = GVersion.compare ((constSpecPtr) _dep, ((constSpecPtr)(conflict->dependency())));
    if (cmp)
	return cmp;

    return CMP ((int) _dep->relation().op(), (int) (conflict->dependency()->relation().op()));
}


///////////////////////////////////////////////////////////////////
}; // namespace ZYPP
///////////////////////////////////////////////////////////////////

