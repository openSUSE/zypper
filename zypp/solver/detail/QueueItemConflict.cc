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
    res += item._conflicting_resolvable->asString();
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

QueueItemConflict::QueueItemConflict (WorldPtr world, constDependencyPtr dep, constResolvablePtr resolvable)
    : QueueItem (QUEUE_ITEM_TYPE_CONFLICT, world)
    , _dep (dep)
    , _conflicting_resolvable (resolvable)
    , _actually_an_obsolete (false)
{
}


QueueItemConflict::~QueueItemConflict()
{
}

//---------------------------------------------------------------------------

#if PHI

// on conflict, try to find upgrade candidates for the installed resolvable triggering the conflict
// there are cases where upgrading prevents the conflict
// rc tends to uninstall the resolvable
// phi tends to upgrade the resolvable
// testcases: exercise-02conflict-08-test.xml, exercise-02conflict-09-test.xml

typedef struct {
    ResolverContextPtr context;
    CResolvableList upgrades;
} UpgradeCandidateInfo;


static bool
upgrade_candidates_cb (constResolvablePtr resolvable, constSpecPtr spec, void *data)
{
//fprintf (stderr, "upgrade_candidates_cb(%s,%s)\n", resolvable->asString().c_str(), spec->asString().c_str());
    UpgradeCandidateInfo *info = (UpgradeCandidateInfo *)data;
    if (info->context->getStatus (resolvable) == RESOLVABLE_STATUS_UNINSTALLED) {
	info->upgrades.push_back (resolvable);
    }
    return true;
}

#endif  // PHI


typedef struct {
    WorldPtr world;
    constResolvablePtr conflicting_resolvable;
    constDependencyPtr dep;
    ResolverContextPtr context;
    QueueItemList & new_items;

    string pkg_str;
    string dep_str;

    bool actually_an_obsolete;
} ConflictProcessInfo;


static bool
conflict_process_cb (constResolvablePtr resolvable, constSpecPtr spec, void *data)
{
    ConflictProcessInfo *info = (ConflictProcessInfo *)data;
    ResolvableStatus status;
    string pkg_str, spec_str, msg;
    ResolverInfoPtr log_info;

    if (getenv ("RC_SPEW")) fprintf (stderr, "conflict_process_cb (resolvable[%s], spec[%s], info [%s]\n", resolvable->asString().c_str(), spec->asString().c_str(), info->conflicting_resolvable->asString().c_str());
    if (getenv ("RC_SPEW")) fprintf (stderr, "conflict_process_cb (resolvable equals spec: %s, info->dep [%s]\n", resolvable->equals(spec) ? "YES" : "NO", info->dep->asString().c_str());

    /* We conflict with ourself.  For the purpose of installing ourself, we
     * just ignore it, but it's Debian's way of saying that one and only one
     * resolvable with this provide may exist on the system at a time. */

    if (info->conflicting_resolvable
	&& resolvable->equals (info->conflicting_resolvable)) {
	return true;
    }

    /* FIXME: This should probably be a GVersion capability. */
    /* Obsoletes don't apply to virtual provides, only the resolvables
     * themselves.  A provide is "virtual" if it's not the same spec
     * as the resolvable that's providing it.  This, of course, only
     * applies to RPM, since it's the only one with obsoletes right
     * now. */

    if (info->actually_an_obsolete
	&& !(resolvable->equals(spec)))
    {
	return true;
    }

    pkg_str = resolvable->asString();
    spec_str = spec->asString();

    status = info->context->getStatus (resolvable);

    if (getenv ("RC_SPEW")) fprintf (stderr, "conflict_process_cb (resolvable[%s]<%s>\n", resolvable->asString().c_str(), ResolverContext::toString(status).c_str());

    switch (status) {
	
    case RESOLVABLE_STATUS_INSTALLED:
    case RESOLVABLE_STATUS_TO_BE_INSTALLED_SOFT: {
	QueueItemUninstallPtr uninstall;
	ResolverInfoPtr log_info;

#if PHI
	// maybe an upgrade can resolve the conflict ?
	//        check if other resolvable is available which upgrades

	// find non-installed packages which provide the conflicting name

	UpgradeCandidateInfo upgrade_info;
	upgrade_info.context = info->context;

	DependencyPtr maybe_upgrade_dep = new Dependency (resolvable->name(), Relation::Any, Kind::Package, new Channel (CHANNEL_TYPE_NONSYSTEM), -1);
	info->world->foreachProvidingResolvable (maybe_upgrade_dep, upgrade_candidates_cb, (void *)&upgrade_info);

#endif

	uninstall = new QueueItemUninstall (info->world, resolvable, "conflict");
	uninstall->setDependency (info->dep);

	if (info->actually_an_obsolete) {
	    uninstall->setDueToObsolete ();
	    log_info = new ResolverInfoObsoletes (resolvable, info->conflicting_resolvable);
	} else {
	    uninstall->setDueToConflict ();
	    log_info = new ResolverInfoConflictsWith (resolvable, info->conflicting_resolvable);
	}

	uninstall->addInfo (log_info);

#if PHI
	if (upgrade_info.upgrades.empty ()) {
#endif

	    info->new_items.push_back (uninstall);

#if PHI
	}
	else {
	    // there are upgrade candidates for the conflicting resolvable
	    // branch to: 1. uninstall, 2. upgrade (for each upgrading resolvable)

	    QueueItemBranchPtr branch = new QueueItemBranch (info->world);

	    branch->addItem (uninstall);			// try uninstall

	    for (CResolvableList::const_iterator iter = upgrade_info.upgrades.begin(); iter != upgrade_info.upgrades.end(); iter++) {
		QueueItemInstallPtr upgrade = new QueueItemInstall (info->world, *iter);
		upgrade->setUpgrades (resolvable);
		branch->addItem (upgrade);			// try upgrade
	    }
	    info->new_items.push_back (branch);
	}
#endif

	break;
    }

    case RESOLVABLE_STATUS_TO_BE_INSTALLED: {
	msg = string ("A conflict over ") + info->dep_str + " (" + spec_str + ") requires the removal of the to-be-installed resolvable " + pkg_str;

	ResolverInfoMiscPtr misc_info = new ResolverInfoMisc (resolvable,RESOLVER_INFO_PRIORITY_VERBOSE, msg);

	misc_info->flagAsError ();
	if (info->conflicting_resolvable) {
	    misc_info->addRelatedResolvable (info->conflicting_resolvable);
	}
	info->context->addInfo (misc_info);

	break;
    }

    case RESOLVABLE_STATUS_UNINSTALLED: {
	info->context->setStatus (resolvable, RESOLVABLE_STATUS_TO_BE_UNINSTALLED);
	msg = string ("Marking ") + pkg_str + " as uninstallable due to conflicts over " + info->dep_str + " (" + spec_str + ")";
	if (!(info->pkg_str.empty())) {
	    msg += " from ";
	    msg += info->pkg_str;
	}

	ResolverInfoMiscPtr misc_info = new ResolverInfoMisc (NULL, RESOLVER_INFO_PRIORITY_VERBOSE, msg);

	misc_info->addRelatedResolvable (resolvable);
	if (info->conflicting_resolvable) {
	    misc_info->addRelatedResolvable(info->conflicting_resolvable);
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

    ConflictProcessInfo info = { world(), _conflicting_resolvable, _dep, context, new_items, "", "", _actually_an_obsolete };

    if (_conflicting_resolvable) {
	info.pkg_str = _conflicting_resolvable->asString();
    }

    info.dep_str = _dep->relation().asString() + " " + ((constSpecPtr)_dep)->asString();

    world()->foreachProvidingResolvable (_dep, conflict_process_cb, (void *)&info);
					
// FIXME: free self

    return true;
}


//---------------------------------------------------------------------------

QueueItemPtr
QueueItemConflict::copy (void) const
{
    QueueItemConflictPtr new_conflict = new QueueItemConflict (world(), _dep, _conflicting_resolvable);
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

