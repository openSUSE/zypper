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

ostream&
operator<<( ostream& os, const QueueItemUninstall & item)
{
    os << "[Uninstall: ";

    os << item._item->asString();
    os << " (";
    switch (item._reason) {
	case CONFLICT:	  os << "conflicts"; break;
	case OBSOLETE:	  os << "obsoletes"; break;
	case UNSATISFIED: os << "unsatisfied dependency"; break;
	case BACKOUT:	  os << "uninstallable"; break;
	case UPGRADE:	  os << "upgrade"; break;
	case DUPLICATE:	  os << "duplicate"; break;
	case EXPLICIT:	  os << "explicit"; break;
    }
    os << ")";
    if (item._dep_leading_to_uninstall != Capability()) {
	os << ", Triggered By ";
	os << item._dep_leading_to_uninstall.asString();
    }
    if (item._upgraded_to != NULL) {
	os << ", Upgraded To ";
	os << item._upgraded_to->asString();
    }
    if (item._explicitly_requested) os << ", Explicit";
    if (item._remove_only) os << ", Remove Only";
    if (item._due_to_conflict) os << ", Due To Conflict";
    if (item._due_to_obsolete) os << ", Due To Obsolete";
    if (item._unlink) os << ", Unlink";
    os << "]";
    return os;
}

//---------------------------------------------------------------------------

QueueItemUninstall::QueueItemUninstall (const ResPool *pool, PoolItem_Ref item, UninstallReason reason)
    : QueueItem (QUEUE_ITEM_TYPE_UNINSTALL, pool)
    , _item (item)
    , _reason (reason)
    , _cap_leading_to_uninstall (Capability())
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

struct UnlinkCheck: public resfilter::OnCapMatchCallbackFunctor
{
    ResolverContext_Ptr context;
    bool cancel_unlink;

    // An item is to be uninstalled, it provides a capability
    //   which requirer needs (as match)
    //   if requirer is installed or to-be-installed:
    //     check if anyone else provides it to the requirer
    //	   or if the uninstall breaks the requirer
    //	     in this case, we have to cancel the uninstallation

    bool operator()( PoolItem_Ref requirer, const Capability & match ) const
    {
      // Untill we can pass the functor by reference to algorithms.
      return const_cast<RequireProcess&>(*this).fake( requirer, match );
    }
    bool fake( PoolItem_Ref requirer, const Capability & match )
    {
	if (cancel_unlink)				// already cancelled
	    return true;

	if (! context->isPresent (requirer))		// item is not (to-be-)installed
	    return true;

	if (context->requirementIsMet (match))		// another resolvable provided match
	    return true;

	cancel_unlink = true;				// cancel, as this would break dependencies

	return true;
    }
}

//---------------------------------------------------------------------------


struct UninstallProcess: public resfilter::OnCapMatchCallbackFunctor
{
    const ResPool *pool;
    ResolverContext_Ptr context;
    PoolItem_Ref uninstalled_item;
    PoolItem_Ref upgraded_item;
    QueueItemList & qil;
    bool remove_only;

    UninstallProcessInfo (const ResPool *p, ResolverContext_Ptr ct, PoolItem_Ref u1, PoolItem_Ref u2, QueueItemList & l, bool ro)
	: pool (p)
	, context (ct)
	, uninstalled_item (u1)
	, upgraded_item (u2)
	, qil (l)
	, remove_only (ro)


    // the uninstall of uninstalled_item breaks the dependency 'match' of resolvable 'requirer'

    bool operator()( PoolItem_Ref requirer, const Capability & match ) const
    {
      // Untill we can pass the functor by reference to algorithms.
      return const_cast<RequireProcess&>(*this).fake( requirer, match );
    }
    bool fake( PoolItem_Ref requirer, const Capability & match )
    {
	if (! context->isPresent (requirer))				// its not installed -> dont care
	    return true;

	if (context->requirementIsMet (match, false))			// its provided by another installed resolvable -> dont care
	    return true;

	if (item_status_is_satisfied (requirer)) {			// it is just satisfied, check freshens
#warning If an uninstall incompletes a satisfied, the uninstall should be cancelled
	    QueueItemEstablish_Ptr establish_item = new QueueItemEstablish (pool, requirer);	// re-check if its still needed
	    qil->push_front (establish_item);
	    return true;
	}

	QueueItemRequire_Ptr require_item = new QueueItemRequire (pool, match);	// issue a new require to fulfill this dependency
	require_item->addResItem (requirer);
	if (remove_only) {
	    require_item->setRemoveOnly ();
	}
	require_item->setUpgradedResItem (upgraded_item);
	require_item->setLostResItem (uninstalled_item);				// this is what we lost, dont re-install it

	qil->push_front (require_item);

	return true;
    }
}


bool
QueueItemUninstall::process (ResolverContext_Ptr context, QueueItemList & qil)
{
    ResItemStatus status;
    string pkg_str;

    pkg_str = _item->asString();

    status = context->getStatus (_item);

    _DBG("RC_SPEW") << "QueueItemUninstall::process(<" << ResolverContext::toString(status) << ">" << _item->asString() << ( _unlink ? "[unlink]" : "") << endl;

    /* In the case of an unlink, we only want to uninstall the item if it is
       being used by something else.  We can't really determine this with 100%
       accuracy, since some later queue item could cause something that requires
       the item to be uninstalled.  The alternative is to try to do something
       really clever... but I'm not clever enough to think of an algorithm that
	 (1) Would do the right thing.
	 (2) Is guaranteed to terminate. (!)
       so this will have to do.  In practice, I don't think that this is a serious
       problem. */

    if (_unlink) {
	/* If the item is to-be-installed, obviously it is being use! */
	if (status == RESOLVABLE_STATUS_TO_BE_INSTALLED) {

	    ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_UNINSTALL_TO_BE_INSTALLED, _item, RESOLVER_INFO_PRIORITY_VERBOSE);
	    context->addInfo (misc_info);
	    goto finished;

	} else if (status == RESOLVABLE_STATUS_INSTALLED) {
	    UnlinkCheckInfo info;

	    /* Flag the item as to-be-uninstalled so that it won't
	       satisfy any other item's deps during this check. */

	    _item.setStatus (RESOLVABLE_STATUS_TO_BE_UNINSTALLED);

	    info.context = context;
	    info.cancel_unlink = false;

	    // look at the provides of the to-be-uninstalled resolvable and
	    //   check if anyone (installed) needs it

	    CapSet provides = _item->provides();
	    for (CapSet::const_iterator iter = provides.begin(); iter != provides.end() && ! info.cancel_unlink; iter++) {

		//world()->foreachRequiringResItem (*iter, unlink_check_cb, &info);

		Dep dep( Dep::REQUIRES);

		invokeOnEach( pool()->byCapabilityIndexBegin( iter->index(), dep ),
			      pool()->byCapabilityIndexEnd( iter->index(), dep ),
			      resfilter::callOnCapMatchIn( dep, *iter, info ) );
	    }

	    /* Set the status back to normal. */
	    _item->setStatus (status);

	    if (info.cancel_unlink) {
		ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_UNINSTALL_INSTALLED, _item, RESOLVER_INFO_PRIORITY_VERBOSE);
		context->addInfo (misc_info);
		goto finished;
	    }
	}

    }

    context->uninstallResItem (_item, _upgraded_to != NULL, _due_to_obsolete, _unlink);

    if (status == RESOLVABLE_STATUS_INSTALLED) {

	if (! _explicitly_requested
	    && pool()->itemIsLocked (_item)) {

	    ResolverInfo_Ptr misc_info = new ResolverInfoMisc (RESOLVER_INFO_TYPE_UNINSTALL_LOCKED, _item, RESOLVER_INFO_PRIORITY_VERBOSE);
	    context->addError (misc_info);
	    goto finished;
	}

	this->logInfo (context);

	if (_cap_leading_to_uninstall != Capability()		// non-empty _cap_leading_to_uninstall
	    && !_due_to_conflict
	    && !_due_to_obsolete)
	{
	    ResolverInfo_Ptr info = new ResolverInfoMissingReq (_item, _cap_leading_to_uninstall);
	    context->addInfo (info);
	}


	// we're uninstalling an installed item
	//   loop over all its provides and check if any installed item requires
	//   one of these provides

	CapSet provides = _item->dep(Dep::PROVIDES);

	for (CapSet::const_iterator iter = provides.begin(); iter != provides.end(); iter++) {
	    UninstallProcess info ( pool(), context, _item, _upgraded_to, qil, _remove_only);

	    //world()->foreachRequiringResItem (*iter, uninstall_process_cb, &info);
	    Dep dep( Dep::REQUIRES);

	    invokeOnEach( pool()->byCapabilityIndexBegin( iter->index(), dep ),
			  pool()->byCapabilityIndexEnd( iter->index(), dep ),
			  resfilter::callOnCapMatchIn( dep, *iter, info ) );
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
    return ResItem::compare (_item, uninstall->_item);
}


QueueItem_Ptr
QueueItemUninstall::copy (void) const
{
    QueueItemUninstall_Ptr new_uninstall = new QueueItemUninstall (world(), _item, _reason);
    new_uninstall->QueueItem::copy(this);


    new_uninstall->_item              	      = _item;
    new_uninstall->_cap_leading_to_uninstall  = _cap_leading_to_uninstall;
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

