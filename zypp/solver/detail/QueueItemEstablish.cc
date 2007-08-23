/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* QueueItemEstablish.cc
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

#include "zypp/solver/detail/Types.h"

#include "zypp/solver/detail/QueueItemEstablish.h"
#include "zypp/solver/detail/QueueItemInstall.h"
#include "zypp/solver/detail/QueueItemRequire.h"
#include "zypp/solver/detail/QueueItemConflict.h"
#include "zypp/solver/detail/QueueItem.h"
#include "zypp/solver/detail/Helper.h"
#include "zypp/solver/detail/ResolverContext.h"
#include "zypp/solver/detail/ResolverInfoConflictsWith.h"
#include "zypp/solver/detail/ResolverInfoNeededBy.h"
#include "zypp/solver/detail/ResolverInfoMisc.h"

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

IMPL_PTR_TYPE(QueueItemEstablish);

//---------------------------------------------------------------------------

std::ostream &
QueueItemEstablish::dumpOn( std::ostream & os ) const
{
    os <<"[Establish: ";
    os << _item;
    if (_explicitly_requested) os << ", Explicit !";
    os << "]";
    return os;
}

//---------------------------------------------------------------------------

QueueItemEstablish::QueueItemEstablish (const ResPool & pool, PoolItem_Ref item, bool soft)
    : QueueItem (QUEUE_ITEM_TYPE_ESTABLISH, pool)
    , _item(item)
    , _soft(soft)
    , _channel_priority (0)
    , _other_penalty (0)
    , _explicitly_requested (false)
{
    _XDEBUG("QueueItemEstablish::QueueItemEstablish (" << item << ")");

}


QueueItemEstablish::~QueueItemEstablish()
{
}

//---------------------------------------------------------------------------

bool
QueueItemEstablish::isSatisfied (ResolverContext_Ptr context) const
{
    return context->isPresent (_item);
}


//---------------------------------------------------------------------------


bool
QueueItemEstablish::process (ResolverContext_Ptr context, QueueItemList & qil)
{
    _XDEBUG("QueueItemEstablish::process(" << *this << ")");

    ResStatus status = context->getStatus(_item);
    
    if (_item.status().isLocked()
        || status.isLocked()) {
        _XDEBUG("Item " << _item << " is locked. --> NO establish");
        return true;
    }
    if ( ! _item->arch().compatibleWith( context->architecture() ) ) {
	context->unneeded (_item, _other_penalty);
	_XDEBUG( _item << " has incompatible architecture, unneeded" );
	return true;
    }

    _item.status().setUndetermined();		// reset any previous establish state

    CapSet freshens = _item->dep(Dep::FRESHENS);

    _XDEBUG("simple establish of " << _item << " with " << freshens.size() << " freshens");

    ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_ESTABLISHING, _item, RESOLVER_INFO_PRIORITY_VERBOSE);
    context->addInfo (misc_info);
    logInfo (context);

    /* Loop through all freshen dependencies. If one is satisfied, queue the _item for installation.  */

    CapSet::const_iterator iter;
    for (iter = freshens.begin(); iter != freshens.end(); iter++) {
	const Capability cap = *iter;
        bool dummy1, dummy2;
	if (context->requirementIsMet (cap, _item, Dep::FRESHENS, &dummy1, &dummy2, true)) { //true =installInfoFlag
	    _XDEBUG("this freshens " << cap);
	    break;
	}
    }

    // if we have freshens but none of the freshen deps were met, mark the _item as unneeded
    // else we look at its supplements as an additional condition
    // (freshens AND supplements must be true. true means either empty or at least one match)
    //
    // else we look at its requires to set it to satisfied or incomplete

    if (freshens.size() > 0				// have freshens !
	&& iter == freshens.end())			// but none matched
    {
	_XDEBUG(_item << " freshens nothing -> unneeded");
	if (_item->kind() != ResTraits<Package>::kind)
	    context->unneeded (_item, _other_penalty);
    }
    else {							// installed or no freshens or triggered freshens

	CapSet supplements = _item->dep(Dep::SUPPLEMENTS);
	if (supplements.size() != 0) {					// if we have supplements, they must _also_ trigger
	    CapSet::const_iterator iter;
	    for (iter = supplements.begin(); iter != supplements.end(); iter++) {
		const Capability cap = *iter;
                bool dummy1, dummy2;                
		if (context->requirementIsMet (cap, _item, Dep::SUPPLEMENTS, &dummy1, &dummy2, true)) { //true =installInfoFlag
		    _XDEBUG("this supplements " << cap);
		    break;
		}
	    }
	    if (iter == supplements.end()) {
		_XDEBUG(_item << " none of the supplements match -> unneeded");
		if (_item->kind() != ResTraits<Package>::kind)
		    context->unneeded (_item, _other_penalty);
		return true;
	    }

	    PoolItem_Ref installed = Helper::findInstalledItem( pool(), _item );
	    if (!installed								// not installed
		&& Helper::isBestUninstalledItem( pool(), _item ))			// and no better available -> install
	    {
		// not yet installed, have supplements, and at least one triggers -> install
		_DEBUG("Uninstalled " << _item << " supplements " << *iter << " -> install");
		QueueItemInstall_Ptr install_item = new QueueItemInstall( pool(), _item, true );
		qil.push_front( install_item );
		return true;
	    }
	}

	// the conditions (freshens, supplements) are true (either empty or at least one match)
	// for a package, these are sufficient to trigger its installation. (packages only get
	//  'established' if they have freshens or supplements).
	//  While this is fine for e.g. fonts etc. its problematic for kernel driver packages.
	//  We must not install kernel driver packages which (via their dependencies) will install
	//  additional kernels. So for kernel driver packages, checking their requires is indeed
	//  the right thing. (see #178721)

	// So the current code checks all requirements and only triggers an install if
	//  - all requirements are already fulfilled
	//  - the package is not already installed (so we do only fresh installs, no upgrades here)

	// for other kind of resolvables, we now look at their requirements and set their
	//  'state modifier' accordingly.


	CapSet requires = _item->dep(Dep::REQUIRES);			// check requirements
	Capability missing;
	bool all_unneeded = true;					// check if all are met because of unneeded
	for (iter = requires.begin(); iter != requires.end(); iter++) {
	    missing = *iter;
	    bool unneeded;
	    if (!context->requirementIsMet (missing, _item, Dep::REQUIRES, &unneeded)) {
		all_unneeded = false;
		break;
	    }
	    if (!unneeded) all_unneeded = false;
	}
	if (iter == requires.end()) {					// all are met
	    if (all_unneeded
		&& _item->kind() == ResTraits<Patch>::kind)		// unneeded is transitive only for patches (#171590)
	    {
		_XDEBUG("all requirements of " << _item << " unneeded -> unneeded");
		context->unneeded( _item, _other_penalty );
	    }
	    else if (_item->kind() == ResTraits<Package>::kind)		// install package if not installed yet.
	    {
		PoolItem_Ref installed = Helper::findInstalledItem( pool(), _item );
		if (!installed								// not installed
		    && Helper::isBestUninstalledItem( pool(), _item ))			// and no better available -> install
		{
		    // freshens and at least one triggers -> install
		    _DEBUG("Uninstalled " << _item << " freshens -> install");
		    QueueItemInstall_Ptr install_item = new QueueItemInstall( pool(), _item, true );
		    qil.push_front( install_item );
		    return true;
		}
	    }
	    else
	    {
		_XDEBUG("all requirements of " << _item << " met -> satisfied");
		context->satisfy( _item, _other_penalty );
	    }
	}
	else {								// some requirements are unfulfilled
	    // If the item stays installed, blame the user
	    if ((_item->kind() == ResTraits<Patch>::kind	// bug 198379, set incomplete for all patches, installed or uninstalled
		 || _item->kind() == ResTraits<Atom>::kind)	// Bug 190272,  - same for atoms
		|| status.staysInstalled())
	    {
		_XDEBUG("Atom/Patch/Installed/Establishing " << _item << " has unfulfilled requirement " << *iter << " -> incomplete");
		context->incomplete( _item, _other_penalty );
	    }
	    else {
		_XDEBUG("Transacted " << _item << " has unfulfilled requirement " << *iter << " -> leave");
		// do nothing, because its either toBeInstalled or toBeUninstalled
	    }
	}
    }

    return true;
}


QueueItem_Ptr
QueueItemEstablish::copy (void) const
{
    QueueItemEstablish_Ptr new_install = new QueueItemEstablish (pool(), _item, _soft);
    new_install->QueueItem::copy(this);

    new_install->_channel_priority = _channel_priority;
    new_install->_other_penalty = _other_penalty;
    new_install->_explicitly_requested = _explicitly_requested;

    return new_install;
}


int
QueueItemEstablish::cmp (QueueItem_constPtr item) const
{
    int cmp = this->compare (item);
    if (cmp != 0)
	return cmp;
    QueueItemEstablish_constPtr establish = dynamic_pointer_cast<const QueueItemEstablish>(item);
    return compareByNVR(_item.resolvable(), establish->_item.resolvable());
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
