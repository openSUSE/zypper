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

#include "zypp/CapFactory.h"
#include "zypp/CapSet.h"
#include "zypp/CapMatch.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Exception.h"

#include "zypp/base/Algorithm.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"

#include "zypp/solver/detail/Types.h"

#include "zypp/solver/detail/QueueItemConflict.h"
#include "zypp/solver/detail/QueueItemBranch.h"
#include "zypp/solver/detail/QueueItemInstall.h"
#include "zypp/solver/detail/QueueItemUninstall.h"
#include "zypp/solver/detail/QueueItem.h"
#include "zypp/solver/detail/ResolverContext.h"
#include "zypp/solver/detail/ResolverInfoConflictsWith.h"
#include "zypp/solver/detail/ResolverInfoMisc.h"
#include "zypp/solver/detail/ResolverInfoObsoletes.h"

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

std::ostream &
QueueItemConflict::dumpOn( std::ostream & os ) const
{
    os << "[" << (_soft?"Soft":"") << "Conflict: ";
    os << _capability;
    os << ", Triggered by ";
    os << _conflicting_item;
    if (_actually_an_obsolete) os << ", Obsolete !";
    os << "]";
    return os;
}

//---------------------------------------------------------------------------

QueueItemConflict::QueueItemConflict (const ResPool & pool, const Capability & cap, PoolItem_Ref item, bool soft)
    : QueueItem (QUEUE_ITEM_TYPE_CONFLICT, pool)
    , _capability (cap)
    , _conflicting_item (item)
    , _soft (soft)
    , _actually_an_obsolete (false)
{
    _XDEBUG("QueueItemConflict::QueueItemConflict(" << cap << ", " << item << (soft?", soft":"") << ")");
}


QueueItemConflict::~QueueItemConflict()
{
}

//---------------------------------------------------------------------------

#if PHI

// on conflict, try to find upgrade candidates for the installed item triggering the conflict
// there are cases where upgrading prevents the conflict
// rc tends to uninstall the item
// phi tends to upgrade the item
// testcases: exercise-02conflict-08-test.xml, exercise-02conflict-09-test.xml

struct UpgradeCandidate : public resfilter::OnCapMatchCallbackFunctor
{
    PoolItem_Ref item; // the conflicting resolvable, used to filter upgrades with an identical resolvable
    ResolverContext_Ptr context;
    PoolItemList upgrades;

    UpgradeCandidate (PoolItem_Ref pi, ResolverContext_Ptr ctx)
	: item (pi)
	, context (ctx)
    { }

    bool operator() (const PoolItem & candidate, const Capability & cap)
    {

//MIL << "UpgradeCandidate? " << candidate << ":[" << context->getStatus (candidate) << "]" << (item->edition().compare(candidate->edition())) << "<" << item->arch() << "," << candidate->arch() << ">" << endl;
// FIXME put this in the resfilter chain
	ResStatus status = context->getStatus (candidate);
	if ((item->edition().compare(candidate->edition()) < 0)		// look at real upgrades
	    && item->arch() == candidate->arch()			// keep the architecture
	    && (status.wasUninstalled()
		|| status.isToBeUninstalled())				// FIXME: just for exercise-02conflict-03-test.xml
									// the original solver found the uninstalled foo-2.0.1 first, this solver
									// finds the uninstallable first. In the end, we had a duplicate solution
									// now we have no solution. Both results are right.
	   && (!status.isImpossible()) )
	{
//MIL << "UpgradeCandidate! " << candidate << endl;
	    upgrades.push_back (candidate);
	}
	return true;
    }
};

#endif  // PHI


//---------------------------------------------------------------------------------------

struct ConflictProcess : public resfilter::OnCapMatchCallbackFunctor
{
    ResPool pool;
    PoolItem_Ref conflict_issuer;			// the item which issues 'conflicts:'
    const Capability conflict_capability;		// the capability mentioned in the 'conflicts'
    ResolverContext_Ptr context;
    QueueItemList & new_items;
    bool actually_an_obsolete;

    ConflictProcess (const ResPool & pl, PoolItem_Ref ci, const Capability & cc, ResolverContext_Ptr ct, QueueItemList & ni, bool ao)
	: pool (pl)
	, conflict_issuer (ci)
	, conflict_capability (cc)
	, context (ct)
	, new_items (ni)
	, actually_an_obsolete (ao)
    { }

    bool operator()( const PoolItem_Ref & provider, const Capability & provides )
    {
	ResStatus status;
	ResolverInfo_Ptr log_info;
	CapFactory factory;

	_XDEBUG("conflict_process_cb (resolvable[" << provider <<"], provides[" << provides << "], conflicts with [" <<
	      conflict_issuer << " conflicts: " << conflict_capability);

	/* We conflict with ourself.  For the purpose of installing ourself, we
	 * just ignore it, but it's Debian's way of saying that one and only one
	 * item with this provide may exist on the system at a time. */

	if (conflict_issuer
	    && compareByNVRA (provider.resolvable(), conflict_issuer.resolvable()) == 0)
	{
	    _XDEBUG("self-conflict");
	    return true;
	}

	/* FIXME: This should probably be a GVersion capability. */
	/* Obsoletes don't apply to virtual provides, only the items
	 * themselves.  A provide is "virtual" if it's not the same spec
	 * as the item that's providing it.  This, of course, only
	 * applies to RPM, since it's the only one with obsoletes right
	 * now. */
	Capability capTest =  factory.parse ( provider->kind(), provider->name(), Rel::EQ, provider->edition());

	if (actually_an_obsolete
	    && capTest.matches (provides) != CapMatch::yes )
	{
	    _XDEBUG("obsolete to virtual provide - ignoring");
	    return true;
	}

	status = context->getStatus(provider);

	_XDEBUG("ConflictProcess (provider[" << provider << "]<" << status << ">");

	if (status.staysInstalled()
	    || status.isToBeInstalledSoft())
	{
	    ResolverInfo_Ptr log_info;

#if PHI
	    _XDEBUG("Provider is installed - try upgrade");

	    // maybe an upgrade can resolve the conflict ?
	    //        check if other item is available which upgrades

	    // find non-installed packages which provide the conflicting name

	    UpgradeCandidate upgrade_info (provider, context);

	    Capability maybe_upgrade_cap =  factory.parse ( provider->kind(), provider->name(), Rel::ANY, Edition::noedition );
#if 0
	    // pool->foreachProvidingResItem (maybe_upgrade_dep, upgrade_candidates_cb, (void *)&upgrade_info);
	    Dep dep( Dep::PROVIDES );

	    invokeOnEach( pool.byCapabilityIndexBegin( maybe_upgrade_cap.index(), dep ),
			  pool.byCapabilityIndexEnd( maybe_upgrade_cap.index(), dep ),
			  resfilter::callOnCapMatchIn( dep, maybe_upgrade_cap, functor::functorRef<bool,PoolItem,Capability>(upgrade_info) ) );
#endif
	ResPool::const_indexiterator pend = pool.providesend(maybe_upgrade_cap.index());
	for (ResPool::const_indexiterator it = pool.providesbegin(maybe_upgrade_cap.index()); it != pend; ++it) {
	    if (it->second.second->arch() == Arch_src)
		continue;
	    if (maybe_upgrade_cap.matches (it->second.first) == CapMatch::yes) {
		if (!upgrade_info( it->second.second, it->second.first))
		    break;
	    }
	}

	    _XDEBUG("found " << upgrade_info.upgrades.size() << " upgrade candidates");
#endif

	    QueueItemUninstall_Ptr uninstall = new QueueItemUninstall (pool, provider, actually_an_obsolete ? QueueItemUninstall::OBSOLETE : QueueItemUninstall::CONFLICT);
	    uninstall->setCapability (conflict_capability);

	    if (actually_an_obsolete) {
		uninstall->setDueToObsolete ();
		log_info = new ResolverInfoObsoletes (provider, conflict_issuer);
	    } else {
		uninstall->setDueToConflict ();
		log_info = new ResolverInfoConflictsWith (provider, conflict_issuer);
	    }

	    uninstall->addInfo (log_info);

#if PHI
	    if (upgrade_info.upgrades.empty ()) {
#endif

		new_items.push_back (uninstall);

#if PHI
	    }
	    else {
		// there are upgrade candidates for the conflicting item
		// branch to: 1. uninstall, 2. upgrade (for each upgrading item)
		_DEBUG("Branching: uninstall vs. upgrade");
		QueueItemBranch_Ptr branch = new QueueItemBranch (pool);

		branch->addItem (uninstall);			// try uninstall

		for (PoolItemList::const_iterator iter = upgrade_info.upgrades.begin(); iter != upgrade_info.upgrades.end(); iter++) {
		    QueueItemInstall_Ptr upgrade = new QueueItemInstall (pool, *iter);
		    upgrade->setUpgrades (provider);
		    branch->addItem (upgrade);			// try upgrade
		}
		new_items.push_back (branch);
	    }
#endif

	}
	else if (status.isToBeInstalled()) {
	    ResolverInfoMisc_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_CONFLICT_CANT_INSTALL, provider, RESOLVER_INFO_PRIORITY_VERBOSE, provides);
	    if (conflict_issuer) {
		misc_info->setOtherPoolItem (conflict_issuer);
		misc_info->setOtherCapability (conflict_capability);
	    }
	    context->addError (misc_info);

	}
	else if (status.wasUninstalled()) {

	    context->setStatus (provider, ResStatus::impossible);

	    ResolverInfoMisc_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_CONFLICT_UNINSTALLABLE, provider, RESOLVER_INFO_PRIORITY_VERBOSE, provides);

	    misc_info->setOtherPoolItem (conflict_issuer);
	    misc_info->setOtherCapability (conflict_capability);

	    context->addInfo (misc_info);

	}
	else if ((status.isToBeUninstalled() && !status.isToBeUninstalledDueToUnlink())
		|| status.isImpossible()
		|| status.isToBeUninstalledDueToObsolete()) {

	    /* This is the easy case -- we do nothing. */
	}
	else {
	    ZYPP_THROW (Exception ("Unhandled status in ConflictProcess"));
	}

	return true;

    } // operator ()

}; // struct ConflictProcess


bool
QueueItemConflict::process (ResolverContext_Ptr context, QueueItemList & new_items)
{
    _DEBUG("QueueItemConflict::process(" << *this << ")");

    // checking for ignoring dependencies
    IgnoreMap ignoreMap = context->getIgnoreConflicts();    
    for (IgnoreMap::iterator it = ignoreMap.begin();
	 it != ignoreMap.end(); it++) {
	if (it->first == _conflicting_item
	    && it->second == _capability) {
	    _XDEBUG("Found ignoring requires " << _capability << " for " << _conflicting_item);
	    return true;
	} else {
	    _XDEBUG("Ignoring requires " << it->second << " for " <<  it->first << " does not fit");	    
	}
    }

    ConflictProcess info (pool(), _conflicting_item, _capability, context, new_items, _actually_an_obsolete);

    // world()->foreachProvidingPoolItem (_capability, conflict_process_cb, (void *)&info);
#if 0
    Dep dep( Dep::PROVIDES );
    invokeOnEach( pool().byCapabilityIndexBegin( _capability.index(), dep ),
		  pool().byCapabilityIndexEnd( _capability.index(), dep ),
		  resfilter::callOnCapMatchIn( dep, _capability, functor::functorRef<bool,PoolItem,Capability>(info) ) );
#endif
	ResPool::const_indexiterator pend = pool().providesend(_capability.index());
	for (ResPool::const_indexiterator it = pool().providesbegin(_capability.index()); it != pend; ++it) {
	    if (_capability.matches (it->second.first) == CapMatch::yes) {
		if (!info( it->second.second, it->second.first))
		    break;
	    }
	}

    return true;
}


//---------------------------------------------------------------------------

QueueItem_Ptr
QueueItemConflict::copy (void) const
{
    QueueItemConflict_Ptr new_conflict = new QueueItemConflict (pool(), _capability, _conflicting_item);
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
    if ( _capability != conflict->capability())
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
