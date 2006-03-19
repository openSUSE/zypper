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
	if (context->requirementIsMet (cap)) {
	    _XDEBUG("this freshens " << cap);
	    break;
	}
    }

    ResStatus status = context->getStatus(_item);

    // if we have freshens but none of the freshen deps were met, mark the _item as unneeded
    // else we look at its requires to set it to satisfied or incomplete

    if (status.staysUninstalled()
	&& freshens.size() > 0				// have freshens !
	&& iter == freshens.end())
    {
	_DEBUG(_item << " freshens nothing -> unneeded");
	if (_item->kind() != ResTraits<Package>::kind)
	    context->unneeded (_item, _other_penalty);
    }
    else {							// installed or no freshens or triggered freshens

	CapSet supplements = _item->dep(Dep::SUPPLEMENTS);
	if (supplements.size() != 0) {					// if we have supplements, they must also trigger
	    CapSet::const_iterator iter;
	    for (iter = supplements.begin(); iter != supplements.end(); iter++) {
		const Capability cap = *iter;
		if (context->requirementIsMet (cap)) {
		    _XDEBUG("this supplements " << cap);
		    break;
		}
	    }
	    if (iter == supplements.end()) {
		_DEBUG(_item << " none of the supplements match -> unneeded");
		if (_item->kind() != ResTraits<Package>::kind)
		    context->unneeded (_item, _other_penalty);
		return true;
	    }
	}


	CapSet requires = _item->dep(Dep::REQUIRES);			// check requirements
	Capability missing;
	for (iter = requires.begin(); iter != requires.end(); iter++) {
	    missing = *iter;
	    if (!context->requirementIsMet (missing)) {
		break;
	    }
	}
	if (iter == requires.end()) {					// all are met
	    if (_item->kind() == ResTraits<Package>::kind) {
		if (status.staysUninstalled()) {
		    _DEBUG("Uninstalled " << _item << " has all requirements -> install");
		    QueueItemInstall_Ptr install_item = new QueueItemInstall( pool(), _item );
		    qil.push_front( install_item );
		}
	    }
	    else {
		_DEBUG("all requirements of " << _item << " met -> satisfied");
		context->satisfy (_item, _other_penalty);
	    }
	}
	else {
	    // If the item stays installed, blame the user
	    if ((_item->kind() != ResTraits<Package>::kind
		 && _item->kind() != ResTraits<Atom>::kind)
		|| status.staysInstalled()
		|| context->establishing())
	    {
		_DEBUG("Non-Package/Installed/Establishing " << _item << " has unfulfilled requirement " << *iter << " -> incomplete");
		context->incomplete( _item, _other_penalty );
	    }
	    else if (status.staysUninstalled())			// not installed -> schedule for installation
	    {
		_DEBUG("Uninstalled " << _item << " has unfulfilled requirement " << *iter << " -> install");
		QueueItemInstall_Ptr install_item = new QueueItemInstall( pool(), _item );
		qil.push_front( install_item );
	    }
	    else {
		_DEBUG("Transacted " << _item << " has unfulfilled requirement " << *iter << " -> leave");
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
