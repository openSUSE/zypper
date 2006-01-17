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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "zypp/solver/temporary/ResItem.h"
#include "zypp/solver/temporary/World.h"

#include "zypp/solver/detail/QueueItemEstablish.h"
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

IMPL_PTR_TYPE(QueueItemEstablish);

//---------------------------------------------------------------------------

string
QueueItemEstablish::asString ( void ) const
{
    return toString (*this);
}


string
QueueItemEstablish::toString ( const QueueItemEstablish & item)
{
    string ret = "[Establish: ";
    ret += item._resItem->asString();
    if (item._explicitly_requested) ret += ", Explicit !";
    ret += "]";
    return ret;
}


ostream &
QueueItemEstablish::dumpOn( ostream & str ) const
{
    str << asString();
    return str;
}


ostream&
operator<<( ostream& os, const QueueItemEstablish & item)
{
    return os << item.asString();
}

//---------------------------------------------------------------------------

QueueItemEstablish::QueueItemEstablish (World_Ptr world, ResItem_constPtr res_item)
    : QueueItem (QUEUE_ITEM_TYPE_INSTALL, world)
    , _resItem (res_item)
    , _channel_priority (0)
    , _other_penalty (0)
    , _explicitly_requested (false)
{
}


QueueItemEstablish::~QueueItemEstablish()
{
}

//---------------------------------------------------------------------------

bool
QueueItemEstablish::isSatisfied (ResolverContext_Ptr context) const
{
    return context->resItemIsPresent (_resItem);
}


//---------------------------------------------------------------------------


bool
QueueItemEstablish::process (ResolverContext_Ptr context, QueueItemList & qil)
{
    _DBG("RC_SPEW") << "QueueItemEstablish::process(" << asString() << ")" << endl;

    _DBG("RC_SPEW") << "simple establish of " << _resItem->asString() << " with " << _resItem->freshens().size() << " freshens" << endl;

    string msg = string ("Establishing ") + _resItem->name();

    context->addInfoString (_resItem, RESOLVER_INFO_PRIORITY_VERBOSE, msg);

    logInfo (context);

    /* Loop through all freshen dependencies. If one is satisfied, queue the _resItem for installation.  */

    CapSet deps = _resItem->freshens();
    CapSet::const_iterator iter;
    for (iter = deps.begin(); iter != deps.end(); iter++) {
	const Capability dep = *iter;
	if (context->requirementIsMet (dep)) {
	    _DBG("RC_SPEW") << "this freshens " << dep.asString() << endl;
	    break;
	}
    }

    // if we have freshens but none of the freshen deps were met, mark the _resItem as unneeded
    // else we look at its requires to set it to satisfied or incomplete
    if (deps.size() > 0
	&& iter == deps.end()) {
	_DBG("RC_SPEW") << "this freshens nothing -> unneeded" << endl;
	context->unneededResItem (_resItem, _other_penalty);
    }
    else {
	deps = _resItem->requires();
	for (iter = deps.begin(); iter != deps.end(); iter++) {
	    const Capability dep = *iter;
	    if (!context->requirementIsMet (dep)) {
		break;
	    }
	}
	if (iter == deps.end()) {		// all are met
	    _DBG("RC_SPEW") << "all requirements met -> satisfied" << endl;
	    context->satisfyResItem (_resItem, _other_penalty);
	}
	else {
	    _DBG("RC_SPEW") << "unfulfilled requirements -> incomplete" << endl;
	    context->incompleteResItem (_resItem, _other_penalty);
	}
    }

    return true;
}


QueueItem_Ptr
QueueItemEstablish::copy (void) const
{
    QueueItemEstablish_Ptr new_install = new QueueItemEstablish (world(), _resItem);
    ((QueueItem_Ptr)new_install)->copy((QueueItem_constPtr)this);

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
    return ResItem::compare (_resItem, establish->_resItem);
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


