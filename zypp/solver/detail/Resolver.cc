/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* Resolver.cc
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

#include "zypp/solver/detail/Resolver.h"
#include "zypp/base/Logger.h"

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

IMPL_PTR_TYPE(Resolver);

//---------------------------------------------------------------------------

ostream&
operator<<( ostream& os, const Resolver & resolver)
{
    return os << "<resolver/>";
}

//---------------------------------------------------------------------------

Resolver::Resolver (const ResPool *pool)
    , _pool (pool)
    , _timeout_seconds (0)
    , _verifying (false)
    , _valid_solution_count (0)
    , _best_context (NULL)
    , _timed_out (false)
{
}


Resolver::~Resolver()
{
}

//---------------------------------------------------------------------------

const ResPool *
Resolver::pool (void) const
{
//    if (_pool == NULL)
//	return World::globalWorld();

    return _pool;
}

void
Resolver::reset (void)
{
    _timeout_seconds = 0;
    _verifying = false;

    _initial_items.clear();

    _items_to_install.clear();
    _items_to_remove.clear();
    _items_to_verify.clear();
    _items_to_establish.clear();

    _extra_deps.clear();
    _extra_conflicts.clear();

    _pending_queues.clear();
    _pruned_queues.clear();
    _complete_queues.clear();
    _deferred_queues.clear();
    _invalid_queues.clear();

    _valid_solution_count = 0;

    _best_context = NULL;
    _timed_out = false;
}


//---------------------------------------------------------------------------

void
Resolver::addSubscribedChannel (Channel_constPtr channel)
{
    ERR << "Resolver::addSubscribedChannel() not implemented" << endl;
}


void
Resolver::addResItemToInstall (PoolItem & item)
{
    _items_to_install.push_front (item);
}


void
Resolver::addPoolItemsToInstallFromList (CPoolItemList & rl)
{
    for (CPoolItemList::const_iterator iter = rl.begin(); iter != rl.end(); iter++) {
	addPoolItemToInstall (*iter);
    }
}


void
Resolver::addPoolItemToRemove (PoolItem & item)
{
    _items_to_remove.push_front (item);
}


void
Resolver::addPoolItemsToRemoveFromList (CPoolItemList & rl)
{
    for (CPoolItemList::const_iterator iter = rl.begin(); iter != rl.end(); iter++) {
	addPoolItemToRemove (*iter);
    }
}


void
Resolver::addPoolItemToEstablish (PoolItem & item)
{
    _items_to_establish.push_front (item);
}


void
Resolver::addPoolItemsToEstablishFromList (CPoolItemList & rl)
{
    for (CPoolItemList::const_iterator iter = rl.begin(); iter != rl.end(); iter++) {
	addPoolItemToEstablish (*iter);
    }
}


void
Resolver::addPoolItemToVerify (PoolItem & item)
{
    _items_to_verify.push_front (item);
    _items_to_verify.sort ();			//(GCompareFunc) rc_item_compare_name);
}


void
Resolver::addExtraDependency (const Capability & dependency)
{
    _extra_deps.insert (dependency);
}


void
Resolver::addExtraConflict (const Capability & dependency)
{
    _extra_conflicts.insert (dependency);
}


//---------------------------------------------------------------------------

static bool
verify_system_cb (PoolItem & item, void *data)
{
    Resolver *resolver  = (Resolver *)data;

    resolver->addPoolItemToVerify (item);

    return true;
}


void
Resolver::verifySystem (void)
{
    _XXX("Resolver") <<  "Resolver::verifySystem()" << endl;
    world()->foreachResItem (new Channel(CHANNEL_TYPE_SYSTEM), verify_system_cb, this);

    _verifying = true;

#if 0		// commented out in libredcarpet also
    /*
      Walk across the (sorted-by-name) list of installed packages and look for
      packages with the same name.  If they exist, construct a branch item
      containing multiple group items.  Each group item corresponds to removing
      all but one of the duplicates.
    */

    for (CPoolItemList::const_iterator i0 = _items_to_verify.begin(); i0 != _items_to_verify.end();) {
	CPoolItemList::const_iterator i1 = i0;
	i1++;
	CPoolItemList::const_iterator i2 = i1;
	for (; i1 != _items_to_verify.end()&& ! (*i0)->compareName (*i1); i1++) {
	    //empty
	}

	if (i1 != i2) {
	    QueueItemBranch_Ptr branch_item;

	    branch_item = new QueueItemBranch(world());

	    for (CPoolItemList::const_iterator i = i0; i != i1; i++) {

		QueueItemGroup_Ptr grp_item = new QueueItemGroup(world());

		for (CPoolItemList::const_iterator j = i0; j != i1; j++) {
		    Package_constPtr dup_pkg = *j;
		    QueueItemUninstall_Ptr uninstall_item;

		    if (i != j) {
			uninstall_item = new QueueItemUninstall (world(), dup_pkg, QueueItemUninstall::DUPLICATE);
			grp_item->addItem (uninstall_item);
		    }

		}

		branch_item->addItem (grp_item);
	    }

	    _initial_items.push_back (branch_item);
	}

	i0 = i1;
    }
#endif

    /* OK, that was fun.  Now just resolve the dependencies. */
    resolveDependencies ();

    return;
}


//---------------------------------------------------------------------------

// establish state

static bool
trial_establish_cb (PoolItem & item, void *user_data)
{
    Resolver *resolver = (Resolver *)user_data;

    resolver->addPoolItemToEstablish (item);

    printf (">!> Establishing %s\n", item->asString().c_str());

    return false;
}


void
Resolver::establishState (ResolverContext_Ptr context)
{
    _DBG("Resolver") << "Resolver::establishState ()" << endl;
    typedef list<Resolvable::Kind> KindList; 
    static KindList ordered;
    if (ordered.empty()) {
	ordered.push_back (ResTraits<zypp::Patch>::kind);
	ordered.push_back (ResTraits<zypp::Pattern>::kind);
	ordered.push_back (ResTraits<zypp::Product>::kind);
    }

    if (context == NULL)
	context = new ResolverContext();

    for (KindList::const_iterator iter = ordered.begin(); iter != ordered.end(); iter++) {
	const Resolvable::Kind kind = *iter;

	_DBG("Resolver") << "establishing state for kind " << kind.asString() << endl;

	world()->foreachResItemByKind (kind, trial_establish_cb, this);

	// process the queue
	resolveDependencies (context);

	reset();
    }

    _best_context = context;

    return;
}

//---------------------------------------------------------------------------

bool
Resolver::resolveDependencies (const ResolverContext_Ptr context)
{

    time_t t_start, t_now;
    bool have_local_items = false;

    _XXX("Resolver") << "Resolver::resolveDependencies()" << endl;

#warning local items disabled
#if 0
    /* Walk through are list of to-be-installed packages and see if any of them are local. */

    for (CPoolItemList::const_iterator iter = _items_to_install.begin(); iter != _items_to_install.end(); iter++) {
	if ((*iter)->local()) {
	    have_local_items = true;
	    break;
	}
    }

    World_Ptr the_world = world();
    StoreWorld_Ptr local_world = NULL;
    MultiWorld_Ptr local_multiworld = NULL;

    Channel_Ptr local_channel = NULL;

    if (have_local_items) {
	local_multiworld = new MultiWorld();
	local_world = new StoreWorld();

	local_channel = new Channel ("", "Local ResItems", "@local", "");

	local_world->addChannel (local_channel);

	local_multiworld->addSubworld (local_world);
	local_multiworld->addSubworld (the_world);

	the_world = local_multiworld;
    }
#endif

    // create initial_queue

    ResolverQueue_Ptr initial_queue = new ResolverQueue(context);

    /* Stick the current/subscribed channel and world info into the context */

    initial_queue->context()->setWorld(the_world);

    /* If this is a verify, we do a "soft resolution" */

    initial_queue->context()->setVerifying (_verifying);

    /* Add extra items. */

    for (QueueItemList::const_iterator iter = _initial_items.begin(); iter != _initial_items.end(); iter++) {
	initial_queue->addItem (*iter);
    }
    _initial_items.clear();

    for (CPoolItemList::const_iterator iter = _items_to_install.begin(); iter != _items_to_install.end(); iter++) {
	PoolItem & r = *iter;

	/* Add local packages to our dummy channel. */
	if (r->local()) {
	    assert (local_channel != NULL);
	    ResItem_Ptr r1 = const_pointer_cast<ResItem>(r);
	    r1->setChannel (local_channel);
	    local_world->addPoolItem (r);
	}

	initial_queue->addPoolItemToInstall (r);
    }

    for (CPoolItemList::const_iterator iter = _items_to_remove.begin(); iter != _items_to_remove.end(); iter++) {
	initial_queue->addPoolItemToRemove (*iter, true /* remove-only mode */);
    }

    for (CPoolItemList::const_iterator iter = _items_to_verify.begin(); iter != _items_to_verify.end(); iter++) {
	initial_queue->addPoolItemToVerify (*iter);
    }

    for (CPoolItemList::const_iterator iter = _items_to_establish.begin(); iter != _items_to_establish.end(); iter++) {
	initial_queue->addPoolItemToEstablish (*iter);
    }

    for (CapSet::const_iterator iter = _extra_deps.begin(); iter != _extra_deps.end(); iter++) {
	initial_queue->addExtraDependency (*iter);
    }

    for (CapSet::const_iterator iter = _extra_conflicts.begin(); iter != _extra_conflicts.end(); iter++) {
	initial_queue->addExtraConflict (*iter);
    }

    _XXX("Resolver") << "Initial Queue: [" << initial_queue->asString() << "]" << endl;

    _best_context = NULL;

    if (initial_queue->isEmpty()) {
	INT << "Empty Queue, nothing to resolve" << endl;

	return true;
    }

    _pending_queues.push_front (initial_queue);

    time (&t_start);

    while (!_pending_queues.empty()) {

	_XXX("Resolver") << "Pend " << (long) _pending_queues.size()
			      << " / Cmpl " << (long) _complete_queues.size()
			      << " / Prun " << (long) _pruned_queues.size()
			      << " / Defr " << (long) _deferred_queues.size()
			      << " / Invl " << (long) _invalid_queues.size() << endl;

	      if (_timeout_seconds > 0) {
		    time (&t_now);
		    if (difftime (t_now, t_start) > _timeout_seconds) {
			_timed_out = true;
		    break;
		}
	    }

	    ResolverQueue_Ptr queue = _pending_queues.front();
	    _pending_queues.pop_front();
	    ResolverContext_Ptr context = queue->context();

	    queue->process();

	if (queue->isInvalid ()) {

	    _XXX("Resolver") << "Invalid Queue\n" << endl;;
	    _invalid_queues.push_front (queue);

	} else if (queue->isEmpty ()) {

	    _XXX("Resolver") <<"Empty Queue\n" << endl;

	    _complete_queues.push_front (queue);

	    ++_valid_solution_count;

	    /* Compare this solution to our previous favorite.  In the case of a tie,
	       the first solution wins --- yeah, I know this is lame, but it shouldn't
	       be an issue too much of the time. */

	    if (_best_context == NULL
		|| _best_context->compare (context) < 0) {

		_best_context = context;
	    }

	} else if (_best_context != NULL
		   && _best_context->partialCompare (context) > 0) {

	    /* If we aren't currently as good as our previous best complete solution,
	       this solution gets pruned. */

	    _XXX("Resolver") << "PRUNED!" << endl;

	    _pruned_queues.push_front(queue);

	} else {

	    /* If our queue is isn't empty and isn't invalid, that can only mean
	       one thing: we are down to nothing but branches. */

	    queue->splitFirstBranch (_pending_queues, _deferred_queues);
	}

	/* If we have run out of pending queues w/o finding any solutions,
	   and if we have deferred queues, make the first deferred queue
	   pending. */

	if (_pending_queues.empty()
	    && _complete_queues.empty()
	    && !_deferred_queues.empty()) {
	    _pending_queues.push_front (_deferred_queues.front());
	}
	  }
	_XXX("Resolver") << "Pend " << (long) _pending_queues.size()
			<< " / Cmpl " << (long) _complete_queues.size()
			<< " / Prun " << (long) _pruned_queues.size()
			<< " / Defr " << (long) _deferred_queues.size()
			<< " / Invl " << (long) _invalid_queues.size() << endl;

    return _best_context && _best_context->isValid();
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

