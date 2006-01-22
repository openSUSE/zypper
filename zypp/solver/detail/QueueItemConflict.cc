/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "zypp/solver/temporary/World.h"

#include "zypp/solver/detail/QueueItemConflict.h"
#include "zypp/solver/detail/QueueItemBranch.h"
#include "zypp/solver/detail/QueueItemInstall.h"
#include "zypp/solver/detail/QueueItemUninstall.h"
#include "zypp/solver/detail/QueueItem.h"
#include "zypp/solver/detail/ResolverContext.h"
#include "zypp/solver/detail/ResolverInfoConflictsWith.h"
#include "zypp/solver/detail/ResolverInfoMisc.h"
#include "zypp/solver/detail/ResolverInfoObsoletes.h"
#include "zypp/CapFactory.h"
#include "zypp/CapSet.h"
#include "zypp/CapMatch.h"
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

IMPL_PTR_TYPE(QueueItemConflict);

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
    res += item._dep.asString();
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

QueueItemConflict::QueueItemConflict (World_Ptr world, const Capability & dep, ResItem_constPtr resItem)
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
    ResolverContext_Ptr context;
    ResItem_constPtr resItem; // the conflicting resolvable, used to filter upgrades with an identical resolvable
    CResItemList upgrades;
} UpgradeCandidateInfo;


static bool
upgrade_candidates_cb (ResItem_constPtr resItem, const Capability & cap, void *data)
{
    UpgradeCandidateInfo *info = (UpgradeCandidateInfo *)data;
    if ( !(info->resItem->equals (resItem)) 		// dont upgrade with ourselves
	&& info->context->getStatus (resItem) == RESOLVABLE_STATUS_UNINSTALLED) {
	info->upgrades.push_back (resItem);
    }
    return true;
}

#endif  // PHI


typedef struct {
    World_Ptr world;
    ResItem_constPtr conflict_issuer;		// the item which issues 'conflicts:'
    const Capability conflict_capability;	// the capability mentioned in the 'conflicts'
    ResolverContext_Ptr context;
    QueueItemList & new_items;

    bool actually_an_obsolete;
} ConflictProcessInfo;


// resItem provides cap
static bool
conflict_process_cb (ResItem_constPtr provider, const Capability & provides, void *data)
{
    ConflictProcessInfo *info = (ConflictProcessInfo *)data;
    ResItemStatus status;
    ResolverInfo_Ptr log_info;
    CapFactory factory;

    _DBG("RC_SPEW") << "conflict_process_cb (resolvable[" << provider->asString() <<"], provides[" << provides.asString() << "], conflicts with [" <<
	      info->conflict_issuer->asString() << " conflicts: " << info->conflict_capability.asString() << endl;

    /* We conflict with ourself.  For the purpose of installing ourself, we
     * just ignore it, but it's Debian's way of saying that one and only one
     * resItem with this provide may exist on the system at a time. */

    if (info->conflict_issuer
	&& provider->equals (info->conflict_issuer)) {
	return true;
    }

    /* FIXME: This should probably be a GVersion capability. */
    /* Obsoletes don't apply to virtual provides, only the resItems
     * themselves.  A provide is "virtual" if it's not the same spec
     * as the resItem that's providing it.  This, of course, only
     * applies to RPM, since it's the only one with obsoletes right
     * now. */
    Capability capTest =  factory.parse ( provider->kind(),
	                                  provider->name(),
	                                  Rel::EQ,
	                                  provider->edition());

    if (info->actually_an_obsolete
	&& capTest.matches (provides) != CapMatch::yes )
    {
	return true;
    }

    status = info->context->getStatus (provider);

    _DBG("RC_SPEW") << "conflict_process_cb (resolvable[" << provider->asString() << "]<" << ResolverContext::toString(status) << ">" << endl;

    switch (status) {

	case RESOLVABLE_STATUS_INSTALLED:
	case RESOLVABLE_STATUS_TO_BE_INSTALLED_SOFT: {
	    QueueItemUninstall_Ptr uninstall;
	    ResolverInfo_Ptr log_info;

#if PHI
	    // maybe an upgrade can resolve the conflict ?
	    //        check if other resItem is available which upgrades

	    // find non-installed packages which provide the conflicting name

	    UpgradeCandidateInfo upgrade_info;
	    upgrade_info.context = info->context;
	    upgrade_info.resItem = provider;

	    Capability maybe_upgrade_dep =  factory.parse ( provider->kind(),
	                                                    provider->name(),
	                                                    Rel::ANY,
	                                                    Edition::noedition );

	    info->world->foreachProvidingResItem (maybe_upgrade_dep, upgrade_candidates_cb, (void *)&upgrade_info);

#endif

	    uninstall = new QueueItemUninstall (info->world, provider, info->actually_an_obsolete ? QueueItemUninstall::OBSOLETE : QueueItemUninstall::CONFLICT);
	    uninstall->setDependency (info->conflict_capability);

	    if (info->actually_an_obsolete) {
	        uninstall->setDueToObsolete ();
	        log_info = new ResolverInfoObsoletes (provider, info->conflict_issuer);
	    } else {
	        uninstall->setDueToConflict ();
	        log_info = new ResolverInfoConflictsWith (provider, info->conflict_issuer);
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

	        QueueItemBranch_Ptr branch = new QueueItemBranch (info->world);

	        branch->addItem (uninstall);			// try uninstall

	        for (CResItemList::const_iterator iter = upgrade_info.upgrades.begin(); iter != upgrade_info.upgrades.end(); iter++) {
	            QueueItemInstall_Ptr upgrade = new QueueItemInstall (info->world, *iter);
	            upgrade->setUpgrades (provider);
	            branch->addItem (upgrade);			// try upgrade
	        }
	        info->new_items.push_back (branch);
	    }
#endif

	    break;
	}

	case RESOLVABLE_STATUS_TO_BE_INSTALLED: {
	    ResolverInfoMisc_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_CONFLICT_CANT_INSTALL, provider, RESOLVER_INFO_PRIORITY_VERBOSE, provides);

	    misc_info->flagAsError ();
	    if (info->conflict_issuer) {
	        misc_info->setOtherResItem (info->conflict_issuer);
		misc_info->setOtherCapability (info->conflict_capability);
	    }
	    info->context->addInfo (misc_info);

	    break;
	}

	case RESOLVABLE_STATUS_UNINSTALLED: {
	    info->context->setStatus (provider, RESOLVABLE_STATUS_TO_BE_UNINSTALLED);

	    ResolverInfoMisc_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_CONFLICT_UNINSTALLABLE, provider, RESOLVER_INFO_PRIORITY_VERBOSE, provides);

	    misc_info->setOtherResItem (info->conflict_issuer);
	    misc_info->setOtherCapability (info->conflict_capability);

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
QueueItemConflict::process (ResolverContext_Ptr context, QueueItemList & new_items)
{
    _DBG("RC_SPEW") << "QueueItemConflict::process(" << this->asString() << ")" << endl;

    ConflictProcessInfo info = {
	world(),
	_conflicting_resItem,		// conflict_issuer
	_dep,				// conflict_capability
	context,
	new_items,
	_actually_an_obsolete
    };

    world()->foreachProvidingResItem (_dep, conflict_process_cb, (void *)&info);

    return true;
}


//---------------------------------------------------------------------------

QueueItem_Ptr
QueueItemConflict::copy (void) const
{
    QueueItemConflict_Ptr new_conflict = new QueueItemConflict (world(), _dep, _conflicting_resItem);
    new_conflict->QueueItem::copy(this);

    // _actually_an_obsolete is not being copied !

    return new_conflict;
}


int
QueueItemConflict::cmp (QueueItem_constPtr item) const
{
    int cmp = this->compare (item);		// assures equal type
    if (cmp != 0)
	return cmp;

    QueueItemConflict_constPtr conflict = dynamic_pointer_cast<const QueueItemConflict>(item);
    if ( _dep != conflict->dependency())
	cmp = -1;

    return cmp;
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
