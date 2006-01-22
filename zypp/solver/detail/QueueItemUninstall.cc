/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* QueueItemUninstall.cc
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

#include "zypp/solver/temporary/World.h"

#include "zypp/solver/detail/QueueItemUninstall.h"
#include "zypp/solver/detail/QueueItemEstablish.h"
#include "zypp/solver/detail/QueueItemRequire.h"
#include "zypp/solver/detail/QueueItem.h"
#include "zypp/solver/detail/ResolverContext.h"
#include "zypp/solver/detail/ResolverInfoMisc.h"
#include "zypp/solver/detail/ResolverInfoMissingReq.h"
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

IMPL_PTR_TYPE(QueueItemUninstall);

//---------------------------------------------------------------------------

string
QueueItemUninstall::asString ( void ) const
{
    return toString (*this);
}


string
QueueItemUninstall::toString ( const QueueItemUninstall & item)
{
    string ret = "[Uninstall: ";

    ret += item._resItem->asString();
    ret += " (";
    switch (item._reason) {
	case CONFLICT:	  ret += "conflicts"; break;
	case OBSOLETE:	  ret += "obsoletes"; break;
	case UNSATISFIED: ret += "unsatisfied dependency"; break;
	case BACKOUT:	  ret += "uninstallable"; break;
	case UPGRADE:	  ret += "upgrade"; break;
	case DUPLICATE:	  ret += "duplicate"; break;
	case EXPLICIT:	  ret += "explicit"; break;
    }
    ret += ")";
    if (item._dep_leading_to_uninstall != Capability()) {
	ret += ", Triggered By ";
	ret += item._dep_leading_to_uninstall.asString();
    }
    if (item._upgraded_to != NULL) {
	ret += ", Upgraded To ";
	ret += item._upgraded_to->asString();
    }
    if (item._explicitly_requested) ret += ", Explicit";
    if (item._remove_only) ret += ", Remove Only";
    if (item._due_to_conflict) ret += ", Due To Conflict";
    if (item._due_to_obsolete) ret += ", Due To Obsolete";
    if (item._unlink) ret += ", Unlink";
    ret += "]";

    return ret;
}


ostream &
QueueItemUninstall::dumpOn( ostream & str ) const
{
    str << asString();
    return str;
}


ostream&
operator<<( ostream& os, const QueueItemUninstall & item)
{
    return os << item.asString();
}

//---------------------------------------------------------------------------

QueueItemUninstall::QueueItemUninstall (World_Ptr world, ResItem_constPtr resItem, UninstallReason reason)
    : QueueItem (QUEUE_ITEM_TYPE_UNINSTALL, world)
    , _resItem (resItem)
    , _reason (reason)
    , _dep_leading_to_uninstall (Capability())
    , _upgraded_to (NULL)
    , _explicitly_requested (false)
    , _remove_only (false)
    , _due_to_conflict (false)
    , _due_to_obsolete (false)
    , _unlink (false)
{
}


QueueItemUninstall::~QueueItemUninstall()
{
}

//---------------------------------------------------------------------------

void
QueueItemUninstall::setUnlink ()
{
    _unlink = true;
    /* Reduce the priority so that unlink items will tend to get
       processed later.  We want to process unlinks as late as possible...
       this will make our "is this item in use" check more accurate. */
    setPriority (0);

    return;
}

//---------------------------------------------------------------------------

typedef struct {
    ResolverContext_Ptr context;
    Capability capability;
    bool cancel_unlink;
} UnlinkCheckInfo;

// A resolvable providing dep is to be uninstalled, resItem requires dep
//  check if we have to cancel the uninstallation

static bool
unlink_check_cb (ResItem_constPtr resItem, const Capability & dep, void *data)
{
    UnlinkCheckInfo *info = (UnlinkCheckInfo *)data;

    if (info->cancel_unlink)				// already cancelled
	return true;

    if (! info->context->resItemIsPresent (resItem))	// resItem is not installed
	return true;

    if (info->context->requirementIsMet (dep))		// another resolvable provided dep
	return true;

    info->capability = dep;
    info->cancel_unlink = true;				// cancel, as this would break dependencies

    return true;
}

typedef struct {
    World_Ptr world;
    ResolverContext_Ptr context;
    ResItem_constPtr uninstalled_resItem;
    ResItem_constPtr upgraded_resItem;
    QueueItemList *qil;
    bool remove_only;
} UninstallProcessInfo;


// the uninstall of info->uninstalled_resItem breaks the dependency 'dep' of resolvable 'resItem'

static bool
uninstall_process_cb (ResItem_constPtr resItem, const Capability & dep, void *data)
{
    UninstallProcessInfo *info = (UninstallProcessInfo *)data;

    if (! info->context->resItemIsPresent (resItem))					// its not installed -> dont care
	return true;

    if (info->context->requirementIsMet (dep, false))					// its provided by another installed resolvable -> dont care
	return true;

    if (resItem_status_is_satisfied (info->context->getStatus (resItem))) {		// it is just satisfied, check freshens
	QueueItemEstablish_Ptr establish_item = new QueueItemEstablish (info->world, resItem);	// re-check if its still needed
	info->qil->push_front (establish_item);
	return true;
    }

    QueueItemRequire_Ptr require_item = new QueueItemRequire (info->world, dep);	// issue a new require to fulfill this dependency
    require_item->addResItem (resItem);
    if (info->remove_only) {
        require_item->setRemoveOnly ();
    }
    require_item->setUpgradedResItem (info->upgraded_resItem);
    require_item->setLostResItem (info->uninstalled_resItem);				// this is what we lost, dont re-install it

    info->qil->push_front (require_item);

    return true;
}


bool
QueueItemUninstall::process (ResolverContext_Ptr context, QueueItemList & qil)
{
    ResItemStatus status;
    string pkg_str;

    pkg_str = _resItem->asString();

    status = context->getStatus (_resItem);

    _DBG("RC_SPEW") << "QueueItemUninstall::process(<" << ResolverContext::toString(status) << ">" << _resItem->asString() << ( _unlink ? "[unlink]" : "") << endl;

    /* In the case of an unlink, we only want to uninstall the resItem if it is
       being used by something else.  We can't really determine this with 100%
       accuracy, since some later queue item could cause something that requires
       the resItem to be uninstalled.  The alternative is to try to do something
       really clever... but I'm not clever enough to think of an algorithm that
	 (1) Would do the right thing.
	 (2) Is guaranteed to terminate. (!)
       so this will have to do.  In practice, I don't think that this is a serious
       problem. */

    if (_unlink) {
	/* If the resItem is to-be-installed, obviously it is being use! */
	if (status == RESOLVABLE_STATUS_TO_BE_INSTALLED) {

	    ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_UNINSTALL_TO_BE_INSTALLED, _resItem, RESOLVER_INFO_PRIORITY_VERBOSE);
	    context->addInfo (misc_info);
	    goto finished;

	} else if (status == RESOLVABLE_STATUS_INSTALLED) {
	    UnlinkCheckInfo info;

	    /* Flag the resItem as to-be-uninstalled so that it won't
	       satisfy any other resItem's deps during this check. */
	    context->setStatus (_resItem, RESOLVABLE_STATUS_TO_BE_UNINSTALLED);

	    info.context = context;
	    info.cancel_unlink = false;

	    // look at the provides of the to-be-uninstalled resolvable and
	    //   check if anyone (installed) needs it

	    CapSet provides = _resItem->provides();
	    for (CapSet::const_iterator iter = provides.begin(); iter != provides.end() && ! info.cancel_unlink; iter++) {
		world()->foreachRequiringResItem (*iter, unlink_check_cb, &info);
	    }

	    /* Set the status back to normal. */
	    context->setStatus (_resItem, status);

	    if (info.cancel_unlink) {
		ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_UNINSTALL_INSTALLED, _resItem, RESOLVER_INFO_PRIORITY_VERBOSE);
		context->addInfo (misc_info);
		goto finished;
	    }
	}

    }

    context->uninstallResItem (_resItem, _upgraded_to != NULL, _due_to_obsolete, _unlink);

    if (status == RESOLVABLE_STATUS_INSTALLED) {

	if (! _explicitly_requested
	    && world()->resItemIsLocked (_resItem)) {

	    ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_UNINSTALL_LOCKED, _resItem, RESOLVER_INFO_PRIORITY_VERBOSE);
	    context->addError (misc_info);
	    goto finished;
	}

	this->logInfo (context);

	if (_dep_leading_to_uninstall != Capability()		// non-empty _dep_leading_to_uninstall
	    && !_due_to_conflict
	    && !_due_to_obsolete)
	{
	    ResolverInfo_Ptr info = new ResolverInfoMissingReq (_resItem, _dep_leading_to_uninstall);
	    context->addInfo (info);
	}

	CapSet provides = _resItem->provides();
	for (CapSet::const_iterator iter = provides.begin(); iter != provides.end(); iter++) {
	    UninstallProcessInfo info;

	    info.world = world();
	    info.context = context;
	    info.uninstalled_resItem = _resItem;
	    info.upgraded_resItem = _upgraded_to;
	    info.qil = &qil;
	    info.remove_only = _remove_only;

	    world()->foreachRequiringResItem (*iter, uninstall_process_cb, &info);
	}
    }

 finished:

    return true;
}

//---------------------------------------------------------------------------

int
QueueItemUninstall::cmp (QueueItem_constPtr item) const
{
    int cmp = this->compare (item);		// assures equal type
    if (cmp != 0)
	return cmp;

    QueueItemUninstall_constPtr uninstall = dynamic_pointer_cast<const QueueItemUninstall>(item);
    return ResItem::compare (_resItem, uninstall->_resItem);
}


QueueItem_Ptr
QueueItemUninstall::copy (void) const
{
    QueueItemUninstall_Ptr new_uninstall = new QueueItemUninstall (world(), _resItem, _reason);
    new_uninstall->QueueItem::copy(this);


    new_uninstall->_resItem                = _resItem;
    new_uninstall->_dep_leading_to_uninstall  = _dep_leading_to_uninstall;
    new_uninstall->_upgraded_to               = _upgraded_to;

    new_uninstall->_explicitly_requested      = _explicitly_requested;
    new_uninstall->_remove_only               = _remove_only;
    new_uninstall->_due_to_conflict           = _due_to_conflict;
    new_uninstall->_due_to_obsolete           = _due_to_obsolete;
    new_uninstall->_unlink                    = _unlink;

    return new_uninstall;
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

