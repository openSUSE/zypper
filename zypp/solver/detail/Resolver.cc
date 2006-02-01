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

#include "zypp/CapSet.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"

#include "zypp/base/Algorithm.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"

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

std::ostream &
Resolver::dumpOn( std::ostream & os ) const
{
    return os << "<resolver/>";
}

//---------------------------------------------------------------------------

Resolver::Resolver (const ResPool & pool)
    : _pool (pool)
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

ResPool
Resolver::pool (void) const
{
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

    _extra_caps.clear();
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
Resolver::addSubscribedSource (Source_Ref source)
{
    _subscribed.insert(source);
}

void
Resolver::addPoolItemToInstall (PoolItem_Ref item)
{
    _items_to_install.push_front (item);
}


void
Resolver::addPoolItemsToInstallFromList (PoolItemList & rl)
{
    for (PoolItemList::const_iterator iter = rl.begin(); iter != rl.end(); iter++) {
	addPoolItemToInstall (*iter);
    }
}


void
Resolver::addPoolItemToRemove (PoolItem_Ref item)
{
    _items_to_remove.push_front (item);
}


void
Resolver::addPoolItemsToRemoveFromList (PoolItemList & rl)
{
    for (PoolItemList::const_iterator iter = rl.begin(); iter != rl.end(); iter++) {
	addPoolItemToRemove (*iter);
    }
}


void
Resolver::addPoolItemToEstablish (PoolItem_Ref item)
{
    _items_to_establish.push_front (item);
}


void
Resolver::addPoolItemsToEstablishFromList (PoolItemList & rl)
{
    for (PoolItemList::const_iterator iter = rl.begin(); iter != rl.end(); iter++) {
	addPoolItemToEstablish (*iter);
    }
}


void
Resolver::addPoolItemToVerify (PoolItem_Ref item)
{
#if 0
  /** Order PoolItems based on name and edition only. */
  struct {
    /** 'less then' based on name and edition */
    bool operator()( PoolItem_Ref lhs, PoolItem_Ref rhs ) const
    {
      int res = lhs->name().compare( rhs->name() );
      if ( res )
        return res == -1; // lhs < rhs ?
      // here: lhs == rhs, so compare edition:
      return lhs->edition() < rhs->edition();
    }
  } order;
#endif

    _items_to_verify.push_front (item);

#warning Should order by name (and probably edition since with zypp we could have multiple editions installed in parallel)
//    _items_to_verify.sort (order);			//(GCompareFunc) rc_item_compare_name);
}


void
Resolver::addExtraCapability (const Capability & capability)
{
    _extra_caps.insert (capability);
}


void
Resolver::addExtraConflict (const Capability & capability)
{
    _extra_conflicts.insert (capability);
}

void
Resolver::addIgnoreConflict (const PoolItem_Ref item,
		   const Capability & capability)
{
    _ignoreConflicts[item] = capability;
}
void
Resolver::addIgnoreRequires (const PoolItem_Ref item,
			     const Capability & capability)
{
    _ignoreRequires[item] = capability;
}



//---------------------------------------------------------------------------

struct VerifySystem : public resfilter::PoolItemFilterFunctor
{
    Resolver & resolver;

    VerifySystem (Resolver & r)
	: resolver (r)
    { }

    bool operator()( PoolItem_Ref provider )
    {
	resolver.addPoolItemToVerify (provider);
	return true;
    }
};


void
Resolver::verifySystem (void)
{
    DBG <<  "Resolver::verifySystem()" << endl;

    VerifySystem info (*this);

    invokeOnEach( pool().byKindBegin( ResTraits<Package>::kind ),
		  pool().byKindEnd( ResTraits<Package>::kind ),
		  resfilter::ByInstalled ( ),
		  functor::functorRef<bool,PoolItem>(info) );


    _verifying = true;

    resolveDependencies ();

    return;
}


//---------------------------------------------------------------------------

// copy marked item from solution back to pool

static void
solution_to_pool (PoolItem_Ref item, const ResStatus & status, void *data)
{
    if (status.isToBeInstalled()) {
	item.status().setToBeInstalled(ResStatus::SOLVER);
    }
    else if (status.isToBeUninstalled()) {
	item.status().setToBeUninstalled(ResStatus::SOLVER);
    }
    else if (status.isIncomplete()
	     || status.isNeeded()) {
	item.status().setIncomplete();
    }
    else if (status.isUnneeded()) {
	item.status().setUnneeded();
    }
    else if (status.isSatisfied()) {
	item.status().setSatisfied();
    }
    return;
}


//---------------------------------------------------------------------------


// establish state

struct EstablishState : public resfilter::OnCapMatchCallbackFunctor
{
    Resolver & resolver;

    EstablishState (Resolver & r)
	: resolver (r)
    { }

    bool operator()( PoolItem_Ref provider )
    {
	resolver.addPoolItemToEstablish (provider);
	return true;
    }
};


void
Resolver::establishState (ResolverContext_Ptr context)
{
    DBG << "Resolver::establishState ()" << endl;
    typedef list<Resolvable::Kind> KindList; 
    static KindList ordered;
    if (ordered.empty()) {
	ordered.push_back (ResTraits<zypp::Patch>::kind);
	ordered.push_back (ResTraits<zypp::Pattern>::kind);
	ordered.push_back (ResTraits<zypp::Product>::kind);
    }

    if (context == NULL)
	context = new ResolverContext(_pool);

    for (KindList::const_iterator iter = ordered.begin(); iter != ordered.end(); iter++) {
	const Resolvable::Kind kind = *iter;

	DBG << "establishing state for kind " << kind.asString() << endl;

	//world()->foreachResItemByKind (kind, trial_establish_cb, this);

	EstablishState info (*this);

	invokeOnEach( pool().byKindBegin( kind ),
		      pool().byKindEnd( kind ),
		      functor::functorRef<bool,PoolItem>(info) );

	// process the queue
	resolveDependencies (context);

	reset();
    }

    _best_context = context;

    return;
}


void
Resolver::establishPool ()
{
    establishState ();						// establish !
    ResolverContext_Ptr solution = bestContext();

    if (solution) {						// copy solution back to pool
	solution->foreachMarked (solution_to_pool, NULL);
    }
    else {
	ERR << "establishState did not return a bestContext" << endl;
    }

    return;
}

//---------------------------------------------------------------------------

bool
Resolver::resolveDependencies (const ResolverContext_Ptr context)
{

    time_t t_start, t_now;

    DBG << "Resolver::resolveDependencies()" << endl;

#warning local items disabled
#if 0
    bool have_local_items = false;

    /* Walk through are list of to-be-installed packages and see if any of them are local. */

    for (PoolItemList::const_iterator iter = _items_to_install.begin(); iter != _items_to_install.end(); iter++) {
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

    ResolverQueue_Ptr initial_queue = new ResolverQueue(_pool, context);

    /* If this is a verify, we do a "soft resolution" */

    initial_queue->context()->setVerifying (_verifying);

    /* Add extra items. */

    for (QueueItemList::const_iterator iter = _initial_items.begin(); iter != _initial_items.end(); iter++) {
	initial_queue->addItem (*iter);
    }
    _initial_items.clear();

    for (PoolItemList::const_iterator iter = _items_to_install.begin(); iter != _items_to_install.end(); iter++) {
	PoolItem_Ref r = *iter;

#warning local items disabled
#if 0
	/* Add local packages to our dummy channel. */
	if (r->local()) {
	    assert (local_channel != NULL);
	    ResItem_Ptr r1 = const_pointer_cast<ResItem>(r);
	    r1->setChannel (local_channel);
	    local_world->addPoolItem_Ref (r);
	}
#endif
	initial_queue->addPoolItemToInstall (r);
    }

    for (PoolItemList::const_iterator iter = _items_to_remove.begin(); iter != _items_to_remove.end(); iter++) {
	initial_queue->addPoolItemToRemove (*iter, true /* remove-only mode */);
    }

    for (PoolItemList::const_iterator iter = _items_to_verify.begin(); iter != _items_to_verify.end(); iter++) {
	initial_queue->addPoolItemToVerify (*iter);
    }

    for (PoolItemList::const_iterator iter = _items_to_establish.begin(); iter != _items_to_establish.end(); iter++) {
	initial_queue->addPoolItemToEstablish (*iter);
    }

    for (CapSet::const_iterator iter = _extra_caps.begin(); iter != _extra_caps.end(); iter++) {
	initial_queue->addExtraCapability (*iter);
    }

    for (CapSet::const_iterator iter = _extra_conflicts.begin(); iter != _extra_conflicts.end(); iter++) {
	initial_queue->addExtraConflict (*iter);
    }

    DBG << "Initial Queue: [" << initial_queue << "]" << endl;

    _best_context = NULL;

    if (initial_queue->isEmpty()) {
	INT << "Empty Queue, nothing to resolve" << endl;

	return true;
    }

    _pending_queues.push_front (initial_queue);

    time (&t_start);

    while (!_pending_queues.empty()) {

	DBG << "Pend " << (long) _pending_queues.size()
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

	    DBG << "Invalid Queue\n" << endl;;
	    _invalid_queues.push_front (queue);

	} else if (queue->isEmpty ()) {

	    DBG <<"Empty Queue\n" << endl;

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

	    DBG << "PRUNED!" << endl;

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
	DBG << "Pend " << (long) _pending_queues.size()
			<< " / Cmpl " << (long) _complete_queues.size()
			<< " / Prun " << (long) _pruned_queues.size()
			<< " / Defr " << (long) _deferred_queues.size()
			<< " / Invl " << (long) _invalid_queues.size() << endl;

    return _best_context && _best_context->isValid();
}


//----------------------------------------------------------------------------
// resolvePool

struct CollectTransact : public resfilter::PoolItemFilterFunctor
{
    Resolver & resolver;

    CollectTransact (Resolver & r)
	: resolver (r)
    { }

    bool operator()( PoolItem_Ref item )		// only transacts() items go here
    {
	ResStatus status = item.status();
	if (status.isBySolver()) {			// clear any solver transactions
	    item.status().setNoTransact(ResStatus::SOLVER);
	}
	if (status.isUninstalled()) {			// transact && uninstalled
	    resolver.addPoolItemToInstall(item);	// -> install! 
	}
	if (status.isInstalled()) {			// transact && installed
	    resolver.addPoolItemToRemove(item);		// -> remove !
	}
	return true;
    }
};


//  This function loops over the pool and grabs
//  all item.status().transacts() and item.status().byUser()
//  It clears all previous bySolver() states also
//
//  Every toBeInstalled is passed to zypp::solver:detail::Resolver.addPoolItemToInstall()
//  Every toBeUninstalled is passed to zypp::solver:detail::Resolver.addPoolItemToRemove()
//
//  Then zypp::solver:detail::Resolver.resolveDependencies() is called.
//
//  zypp::solver:detail::Resolver then returns a ResolverContext via bestContext() which
//  describes the best solution. If bestContext() is NULL, no solution was found.
//
//  ResolverContext has a foreachMarked() iterator function which loops over all
//  items of the solutions. These must be written back to the pool.

bool
Resolver::resolvePool ()
{

    CollectTransact info (*this);

    invokeOnEach ( _pool.begin(), _pool.end(),
		   resfilter::ByTransact( ),			// collect transacts from Pool to resolver queue
		   functor::functorRef<bool,PoolItem>(info) );

    bool have_solution = resolveDependencies ();		// resolve !

    if (have_solution) {					// copy solution back to pool
	ResolverContext_Ptr solution = bestContext();
	solution->foreachMarked (solution_to_pool, NULL);
    }

    return have_solution;
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

