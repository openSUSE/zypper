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
#include "zypp/solver/detail/Helper.h"

#include "zypp/CapSet.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"

#include "zypp/base/Algorithm.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"
#include "zypp/ZYppFactory.h"
#include "zypp/SystemResObject.h"


/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace zypp_detail
  { /////////////////////////////////////////////////////////////////
    Arch defaultArchitecture();
    /////////////////////////////////////////////////////////////////
  } // namespace zypp_detail
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

using namespace std;

IMPL_PTR_TYPE(Resolver);

static const unsigned MAX_SECOND_RUNS( 3 );
static const unsigned TIMOUT_SECOND_RUN( 30 );

//---------------------------------------------------------------------------

std::ostream &
Resolver::dumpOn( std::ostream & os ) const
{
    return os << "<resolver/>";
}

// Generating a system resolvable in the pool in order to trigger
// modaliases and hals
void assertSystemResObjectInPool()
{
  ResPool pool( getZYpp()->pool() );
  if ( pool.byKindBegin<SystemResObject>()
       == pool.byKindEnd<SystemResObject>() )
    {
      // SystemResObject is missing in the pool ==> insert
      ResStore store;
      store.insert( SystemResObject::instance() );
      getZYpp()->addResolvables( store, true ); // true = is installed
    }

  // set lock
  if ( ! pool.byKindBegin<SystemResObject>()
         ->status().setLock( true, ResStatus::USER ) )
    {
      WAR << "Unable to set SystemResObject to lock" << endl;
    }
}

//---------------------------------------------------------------------------

Resolver::Resolver (const ResPool & pool)
    : _pool (pool)
    , _timeout_seconds (0)
    , _maxSolverPasses (0)
    , _verifying (false)
    , _testing (false)
    , _tryAllPossibilities (false)
    , _scippedPossibilities (false)
    , _valid_solution_count (0)
    , _best_context (NULL)
    , _timed_out (false)
    , _architecture( zypp_detail::defaultArchitecture() )
    , _forceResolve (false)
    , _upgradeMode (false)
{}


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
    
    _tryAllPossibilities = false;
    _scippedPossibilities = false;
    
}


ResolverContext_Ptr
Resolver::context (void) const
{
    if (_best_context) return _best_context;
    if (_invalid_queues.empty()) return NULL;
    ResolverQueue_Ptr invalid = _invalid_queues.front();
    return invalid->context();
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
    bool found = false;
    for (PoolItemList::const_iterator iter = _items_to_remove.begin();
	 iter != _items_to_remove.end(); iter++) {
	if (*iter == item) {
	    _items_to_remove.remove(*iter);
	    found = true;
	    break;
	}
    }
    if (!found)
	_items_to_install.push_back (item);
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
    bool found = false;
    for (PoolItemList::const_iterator iter = _items_to_install.begin();
	 iter != _items_to_install.end(); iter++) {
	if (*iter == item) {
	    _items_to_install.remove(*iter);
	    found = true;
	    break;
	}
    }
    if (!found)
	_items_to_remove.push_back (item);
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
    _items_to_establish.push_back (item);
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

    _items_to_verify.push_back (item);

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
    _ignoreConflicts.insert(make_pair(item, capability));
}


void
Resolver::addIgnoreRequires (const PoolItem_Ref item,
			     const Capability & capability)
{
    _ignoreRequires.insert(make_pair(item, capability));
}

void
Resolver::addIgnoreObsoletes (const PoolItem_Ref item,
			      const Capability & capability)
{
    _ignoreObsoletes.insert(make_pair(item, capability));
}

void
Resolver::addIgnoreInstalledItem (const PoolItem_Ref item)
{
    _ignoreInstalledItem.push_back (item);
}

void
Resolver::addIgnoreArchitectureItem (const PoolItem_Ref item)
{
    _ignoreArchitectureItem.push_back (item);
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


bool
Resolver::verifySystem (bool considerNewHardware)
{
    _DEBUG ("Resolver::verifySystem() " << (considerNewHardware ? "consider new hardware":""));

    VerifySystem info (*this);

    invokeOnEach( pool().byKindBegin( ResTraits<Package>::kind ),
		  pool().byKindEnd( ResTraits<Package>::kind ),
		  resfilter::ByInstalled ( ),
		  functor::functorRef<bool,PoolItem>(info) );


    _verifying = true;

    if (considerNewHardware) {
	// evaluate all Freshens/Supplements and solve
	return freshenPool(false) && bestContext() && bestContext()->isValid();
    }
    else {
	return resolveDependencies (); // do solve only
    }
}


//---------------------------------------------------------------------------

// copy marked item from solution back to pool
// if data != NULL, set as APPL_LOW (from establishPool())

static void
solution_to_pool (PoolItem_Ref item, const ResStatus & status, void *data)
{
    bool r;

    if (status.isToBeInstalled()) {
	r = item.status().setToBeInstalled( (data != NULL) ? ResStatus::APPL_LOW : ResStatus::SOLVER );
	_XDEBUG("solution_to_pool(" << item << ", " << status << ") install !" << r);
    }
    else if (status.isToBeUninstalledDueToUpgrade()) {
	r = item.status().setToBeUninstalledDueToUpgrade( (data != NULL) ? ResStatus::APPL_LOW : ResStatus::SOLVER );
	_XDEBUG("solution_to_pool(" << item << ", " << status << ") upgrade !" << r);
    }
    else if (status.isToBeUninstalled()) {
	r = item.status().setToBeUninstalled( (data != NULL) ? ResStatus::APPL_LOW : ResStatus::SOLVER );
	_XDEBUG("solution_to_pool(" << item << ", " << status << ") remove !" << r);
    }
    else if (status.isIncomplete()
	     || status.isNeeded()) {
	r = item.status().setIncomplete();
	_XDEBUG("solution_to_pool(" << item << ", " << status << ") incomplete !" << r);
    }
    else if (status.isUnneeded()) {
	r = item.status().setUnneeded();
	_XDEBUG("solution_to_pool(" << item << ", " << status << ") unneeded !" << r);
    }
    else if (status.isSatisfied()) {
	r = item.status().setSatisfied();
	_XDEBUG("solution_to_pool(" << item << ", " << status << ") satisfied !" << r);
    } else {
	_XDEBUG("solution_to_pool(" << item << ", " << status << ") unchanged !");
    }
    return;
}


//---------------------------------------------------------------------------
// establish state

struct EstablishState
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
Resolver::establishState( ResolverContext_Ptr context )
{
    _DEBUG( "Resolver::establishState ()" );
    typedef list<Resolvable::Kind> KindList;
    static KindList ordered;
    if (ordered.empty()) {
	ordered.push_back (ResTraits<zypp::Atom>::kind);
	ordered.push_back (ResTraits<zypp::Message>::kind);
	ordered.push_back (ResTraits<zypp::Script>::kind);
	ordered.push_back (ResTraits<zypp::Patch>::kind);
	ordered.push_back (ResTraits<zypp::Pattern>::kind);
	ordered.push_back (ResTraits<zypp::Product>::kind);
    }

    if (context == NULL)
	context = new ResolverContext(_pool, _architecture);

    context->setEstablishing (true);
    context->setIgnoreCababilities (_ignoreConflicts,
				    _ignoreRequires,
				    _ignoreObsoletes,
				    _ignoreInstalledItem,
				    _ignoreArchitectureItem);
    context->setForceResolve( _forceResolve );
    context->setUpgradeMode( _upgradeMode );

    for (KindList::const_iterator iter = ordered.begin(); iter != ordered.end(); iter++) {
	const Resolvable::Kind kind = *iter;

	_XDEBUG( "establishing state for kind " << kind.asString() );

	//world()->foreachResItemByKind (kind, trial_establish_cb, this);

	EstablishState info (*this);

	invokeOnEach( pool().byKindBegin( kind ),
		      pool().byKindEnd( kind ),
		      functor::functorRef<bool,PoolItem>(info) );

	// process the queue
	resolveDependencies( context );

	reset();
    }

    context->setEstablishing (false);

    _best_context = context;

    return;
}


bool
Resolver::establishPool ()
{
    MIL << "Resolver::establishPool()" << endl;

    establishState ();						// establish !
    ResolverContext_Ptr solution = bestContext();

    if (solution) {						// copy solution back to pool
	solution->foreachMarked (solution_to_pool, (void *)1);	// as APPL_LOW
    }
    else {
	ERR << "establishState did not return a bestContext" << endl;
	return false;
    }

    return true;
}


//---------------------------------------------------------------------------
// freshen state

typedef map<string, PoolItem_Ref> FreshenMap;

// add item to itemmap
//  check for item with same name and only keep
//  best architecture, best version

static void
addToFreshen( PoolItem_Ref item, FreshenMap & itemmap )
{
    FreshenMap::iterator it = itemmap.find( item->name() );
    if (it != itemmap.end()) {					// item with same name found
	int cmp = it->second->arch().compare( item->arch() );
	if (cmp < 0) {						// new item has better arch
	    it->second = item;
	}
	else if (cmp == 0) {					// new item has equal arch
	    if (it->second->edition().compare( item->edition() ) < 0) {
		it->second = item;				// new item has better edition
	    }
	}
    }
    else {
	itemmap[item->name()] = item;
    }
    return;
}


struct FreshenState
{
    FreshenMap itemmap;

    FreshenState()
    { }

    bool operator()( PoolItem_Ref item)
    {
	CapSet freshens( item->dep( Dep::FRESHENS ) );
	if (!freshens.empty()) {
	    addToFreshen( item, itemmap );
	}
	else {					// if no freshens, look at supplements
	    // Also regarding supplements e.g. in order to recognize
	    // modalias dependencies. Bug #163140
	    CapSet supplements( item->dep( Dep::SUPPLEMENTS ) );
	    if (!supplements.empty()) {
		addToFreshen( item, itemmap );
	    }
	}
	return true;
    }
};


void
Resolver::freshenState( ResolverContext_Ptr context,
			bool resetAfterSolve )
{
    _DEBUG( "Resolver::freshenState ()" );

    if (context == NULL)
	context = new ResolverContext( _pool, _architecture );

    context->setEstablishing( true );
    context->setIgnoreCababilities( _ignoreConflicts,
				    _ignoreRequires,
				    _ignoreObsoletes,
				    _ignoreInstalledItem,
				    _ignoreArchitectureItem );
    context->setForceResolve( _forceResolve );
    context->setUpgradeMode( _upgradeMode );

    FreshenState info;

    // collect items to be established

    invokeOnEach( pool().byKindBegin( ResTraits<zypp::Package>::kind ),
		      pool().byKindEnd( ResTraits<zypp::Package>::kind ),
		      functor::functorRef<bool,PoolItem>(info) );

    // schedule all collected items for establish

    for (FreshenMap::iterator it = info.itemmap.begin(); it != info.itemmap.end(); ++it) {
	addPoolItemToEstablish( it->second );
    }

    // process the queue
    resolveDependencies( context );
    
    if (resetAfterSolve) {
	reset();
	context->setEstablishing( false );
	_best_context = context;
    }

    return;
}


bool
Resolver::freshenPool (bool resetAfterSolve)
{
    MIL << "Resolver::freshenPool()" << endl;

    freshenState (NULL, resetAfterSolve);	// establish all packages with freshens; (NULL)= no initial context
    ResolverContext_Ptr solution = bestContext();

    if (solution) {						// copy solution back to pool
	solution->foreachMarked (solution_to_pool, (void *)1);	// as APPL_LOW
    }
    else {
	ERR << "freshenState did not return a bestContext" << endl;
	return false;
    }

    return true;
}

//---------------------------------------------------------------------------

bool
Resolver::resolveDependencies (const ResolverContext_Ptr context)
{

    time_t t_start, t_now;

    MIL << "Resolver::resolveDependencies()" << endl;

    _pending_queues.clear();
    _pruned_queues.clear();
    _complete_queues.clear();
    _deferred_queues.clear();
    _invalid_queues.clear();
    _valid_solution_count = 0;
    _best_context = NULL;

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

    ResolverQueue_Ptr initial_queue = new ResolverQueue(_pool, _architecture, context);

    // adding "external" provides, the the requirements will be ignored
    IgnoreMap ignoreRequires = _ignoreRequires;
    ResPool::AdditionalCapSet additionalCapSet = pool().additionaProvide();
    for (ResPool::AdditionalCapSet::const_iterator it = additionalCapSet.begin();
	 it != additionalCapSet.end(); it++) {
	CapSet cset = it->second;
	for (CapSet::const_iterator cit = cset.begin(); cit != cset.end(); ++cit) {
	    ignoreRequires.insert(make_pair(PoolItem_Ref(), *cit));
	}
    }
    
    // Initialize all ignoring dependencies
    initial_queue->context()->setIgnoreCababilities (_ignoreConflicts,
						     ignoreRequires,
						     _ignoreObsoletes,
						     _ignoreInstalledItem,
						     _ignoreArchitectureItem);
    initial_queue->context()->setForceResolve( _forceResolve );
    initial_queue->context()->setUpgradeMode( _upgradeMode );
    initial_queue->context()->setTryAllPossibilities( _tryAllPossibilities );
    initial_queue->context()->setScippedPossibilities( _scippedPossibilities );

    /* If this is a verify, we do a "soft resolution" */

    initial_queue->context()->setVerifying( _verifying );

    /* Add extra items. */

    for (QueueItemList::const_iterator iter = _initial_items.begin(); iter != _initial_items.end(); iter++) {
	initial_queue->addItem (*iter);
    }

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
	if (!_upgradeMode)
	    initial_queue->addPoolItemToRemove (*iter, true /* remove-only mode */);
	else
	    //   Checking old dependencies for packages which will be updated.
	    //   E.g. foo provides a dependecy which foo-new does not provides anymore.
	    //   So check, if there is a packages installed which requires foo.
	    //   Testcase exercise-bug150844-test.xml
	    //   Testcase Bug156439-test.xml
	    initial_queue->addPoolItemToRemove (*iter, false /* no remove-only mode */);
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
    
    // adding "external" requires
    additionalCapSet = pool().additionalRequire();
    for (ResPool::AdditionalCapSet::const_iterator it = additionalCapSet.begin();
	 it != additionalCapSet.end(); it++) {
	CapSet cset = it->second;
	for (CapSet::const_iterator cit = cset.begin(); cit != cset.end(); ++cit) {
	    initial_queue->addExtraCapability (*cit);	    
	}
    }

    for (CapSet::const_iterator iter = _extra_conflicts.begin(); iter != _extra_conflicts.end(); iter++) {
	initial_queue->addExtraConflict (*iter);
    }

    // adding "external" conflicts
    additionalCapSet = pool().additionaConflict();
    for (ResPool::AdditionalCapSet::const_iterator it = additionalCapSet.begin();
	 it != additionalCapSet.end(); it++) {
	CapSet cset = it->second;
	for (CapSet::const_iterator cit = cset.begin(); cit != cset.end(); ++cit) {
	    initial_queue->addExtraConflict (*cit);	    
	}
    }

    // Adding System resolvable
    assertSystemResObjectInPool();

    _XDEBUG( "Initial Queue: [" << *initial_queue << "]" );

    if (initial_queue->isEmpty()) {
	INT << "Empty Queue, nothing to resolve" << endl;

	return true;
    }

    _best_context = NULL;

    _pending_queues.push_front (initial_queue);

    time (&t_start);

    while (!_pending_queues.empty()) {

	_DEBUG( "Pend " << (long) _pending_queues.size()
		<< " / Cmpl " << (long) _complete_queues.size()
		<< " / Prun " << (long) _pruned_queues.size()
		<< " / Defr " << (long) _deferred_queues.size()
		<< " / Invl " << (long) _invalid_queues.size() );

	if (_timeout_seconds > 0) {
	    time (&t_now);
	    if (difftime (t_now, t_start) > _timeout_seconds) {
		_timed_out = true;
		MIL << "Timeout " << _timeout_seconds << " seconds reached"
		    << " -> exit" << endl;
		break;
	    }
	}
	if (_maxSolverPasses > 0) {
	    if (_maxSolverPasses <= _complete_queues.size() +
		_pruned_queues.size() +
		_deferred_queues.size() +
		_invalid_queues.size()) {
		_timed_out = true;
		MIL << "Max solver runs ( " << _maxSolverPasses
		    << " ) reached -> exit" << endl;
		break;
	    }
	}		    
	      
	ResolverQueue_Ptr queue = _pending_queues.front();
	_pending_queues.pop_front();
	ResolverContext_Ptr context = queue->context();

	queue->process();

	if (queue->isInvalid ()) {

	    _XDEBUG( "Invalid Queue\n" );
	    _invalid_queues.push_back(queue);

	} else if (queue->isEmpty ()) {

	    _XDEBUG( "Empty Queue\n" );

	    _complete_queues.push_back(queue);

	    ++_valid_solution_count;

	    /* Compare this solution to our previous favorite.  In the case of a tie,
	       the first solution wins --- yeah, I know this is lame, but it shouldn't
	       be an issue too much of the time. */

	    if (_best_context == NULL
		|| _best_context->compare (context) < 0)
	    {
		_best_context = context;
	    }

	} else if (_best_context != NULL
		   && _best_context->partialCompare (context) > 0) {

	    /* If we aren't currently as good as our previous best complete solution,
	       this solution gets pruned. */

	    _XDEBUG( "PRUNED!" );

	    _pruned_queues.push_back(queue);

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
	    _pending_queues.push_back(_deferred_queues.front());
	}
    }
    _DEBUG("Pend " << (long) _pending_queues.size()
	   << " / Cmpl " << (long) _complete_queues.size()
	   << " / Prun " << (long) _pruned_queues.size()
	   << " / Defr " << (long) _deferred_queues.size()
	   << " / Invl " << (long) _invalid_queues.size() );
    
    
    if ( !(_best_context && _best_context->isValid()) // no valid solution
	 && !_tryAllPossibilities ) { // a second run with ALL possibilities has not been tried

	for (ResolverQueueList::iterator iter = _invalid_queues.begin();
	     iter != _invalid_queues.end(); iter++) {
	    // evaluate if there are other possibilities which have not been regarded
	    ResolverQueue_Ptr invalid =	*iter;    	    
	    if (invalid->context()->scippedPossibilities()) {
		_scippedPossibilities = true;
		break;
	    }
	}
	
	if (_scippedPossibilities) { // possible other solutions scipped	 
	    // lets try a second run with ALL possibilities
	    _tryAllPossibilities = true;
	    MIL << "================================================================"
		<< endl;
	    MIL << "No valid solution, lets try a second run with ALL possibilities"
		<< endl;
	    if (_maxSolverPasses <= 0) 
		_maxSolverPasses = MAX_SECOND_RUNS;	    
	    if (_timeout_seconds <= 0) 
		_timeout_seconds = TIMOUT_SECOND_RUN;

	    MIL << "But no longer than " << MAX_SECOND_RUNS << " runs or "
		<< TIMOUT_SECOND_RUN << " seconds" << endl;
	    MIL << "================================================================"
		<< endl;
	    // saving invalid queue
	    ResolverQueueList   save_queues = _invalid_queues;
	    resolveDependencies ();
	    if (!(_best_context && _best_context->isValid()))
		_invalid_queues = save_queues; // take the old
	}
    }

    return _best_context && _best_context->isValid();
}


//----------------------------------------------------------------------------
// undo

struct UndoTransact : public resfilter::PoolItemFilterFunctor
{
    UndoTransact ()
    { }

    bool operator()( PoolItem_Ref item )		// only transacts() items go here
    {
	item.status().resetTransact( ResStatus::APPL_LOW );// clear any solver/establish transactions
	return true;
    }
};

void
Resolver::undo(void)
{
    UndoTransact info;
    MIL << "*** undo ***" << endl;
    invokeOnEach ( _pool.begin(), _pool.end(),
		   resfilter::ByTransact( ),			// collect transacts from Pool to resolver queue
		   functor::functorRef<bool,PoolItem>(info) );
    // These conflict should be ignored of the concering item
    _ignoreConflicts.clear();
    // These requires should be ignored of the concering item
    _ignoreRequires.clear();
    // These obsoletes should be ignored of the concering item
    _ignoreObsoletes.clear();
    // Ignore architecture of the item
    _ignoreArchitecture.clear();
    // Ignore the status "installed" of the item
    _ignoreInstalledItem.clear();
    // Ignore the architecture of the item
    _ignoreArchitectureItem.clear();


    return;
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
	_XDEBUG( "CollectTransact(" << item << ")" );
	bool by_solver = (status.isBySolver() || status.isByApplLow());

	if (by_solver) {
	    item.status().resetTransact( ResStatus::APPL_LOW );// clear any solver/establish transactions
	    return true;				// back out here, dont re-queue former solver result
	}

	if (status.isToBeInstalled()) {
	    resolver.addPoolItemToInstall(item);	// -> install!
	}
	if (status.isToBeUninstalled()) {
	    resolver.addPoolItemToRemove(item);		// -> remove !
	}
	if (status.isIncomplete()) {			// incomplete (re-install needed)
	    PoolItem_Ref reinstall = Helper::findReinstallItem (resolver.pool(), item);
	    if (reinstall) {
		MIL << "Reinstall " << reinstall << " for incomplete " << item << endl;
		resolver.addPoolItemToInstall(reinstall);	// -> install!
	    }
	    else {
		WAR << "Can't find " << item << " for re-installation" << endl;
	    }
	}
	return true;
    }
};


static void
show_pool( ResPool pool )
{
    int count = 1;
    static bool full_pool_shown = true;

    _XDEBUG( "---------------------------------------" );
    for (ResPool::const_iterator it = pool.begin(); it != pool.end(); ++it, ++count) {

	if (!full_pool_shown					// show item if not shown all before
	    || it->status().transacts()				// or transacts
	    || !it->status().isUndetermined())			// or established status
	{
	    _DEBUG( count << ": " << *it );
	}
    }
    _XDEBUG( "---------------------------------------" );
    full_pool_shown = true;
}

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

    // cleanup before next run
    reset();

#if 1

    MIL << "Resolver::resolvePool()" << endl;
    _XDEBUG( "Pool before resolve" );
    show_pool( _pool );

#endif
    invokeOnEach ( _pool.begin(), _pool.end(),
		   resfilter::ByTransact( ),			// collect transacts from Pool to resolver queue
		   functor::functorRef<bool,PoolItem>(info) );

    bool have_solution = resolveDependencies ();		// resolve !

    if (have_solution) {					// copy solution back to pool
	MIL << "Have solution, copying back to pool" << endl;
	ResolverContext_Ptr solution = bestContext();
	solution->foreachMarked (solution_to_pool, NULL);
#if 1
	_XDEBUG( "Pool after resolve" );
	show_pool( _pool );
#endif
    }
    else {
	MIL << "!!! Have NO solution !!! Additional solver information:" << endl;
	int counter = 0;
	for (ResolverQueueList::iterator iter = _invalid_queues.begin();
	     iter != _invalid_queues.end(); iter++) {
	    counter++;
	    MIL << "-----------------------------------------------------------------" << endl;
	    MIL << counter++ << ". failed queue:" << endl;
	    ResolverQueue_Ptr invalid =	*iter;    
	    invalid->context()->spewInfo ();
	    MIL << *invalid->context() << endl;
	    MIL << "-----------------------------------------------------------------" << endl;		
	}
    }
    return have_solution;
}


static void
get_info_foreach_cb (ResolverInfo_Ptr info, void *data)
{
    list<string> *stringList = (list<string> *)data;
    stringList->push_back (info->message());
}


// returns a string list of ResolverInfo of the LAST not valid solution
std::list<std::string> Resolver::problemDescription( void ) const
{
    list<string> retList;
    if (_invalid_queues.empty()) return retList;
    ResolverQueue_Ptr invalid = _invalid_queues.front();
    invalid->context()->foreachInfo (PoolItem_Ref(), -1, get_info_foreach_cb, (void *)&retList);;
    return retList;
}


//-----------------------------------------------------------------------------

//
// UI Helper
// do a single (install/uninstall) transaction
//  if a previously uninstalled item was just set to-be-installed, return it as 'added'

static bool
transactItems( PoolItem_Ref installed, PoolItem_Ref uninstalled, bool install, bool soft, PoolItem_Ref & added )
{
	if (install) {
	    if (compareByNVRA (installed.resolvable(), uninstalled.resolvable()) != 0) { // do not update itself Bug 174290
		if (uninstalled
		    && !uninstalled.status().isLocked())
		{
		    bool adding;	// check if we're succeeding with transaction
		    if (soft)
			adding = uninstalled.status().setSoftTransact( true, ResStatus::APPL_LOW );
		    else
			adding = uninstalled.status().setTransact( true, ResStatus::APPL_LOW );
		    if (adding)	// if succeeded, return it as 'just added'
			added = uninstalled;
		}
		if (installed
		    && !installed.status().isLocked())
		{
		    installed.status().resetTransact( ResStatus::APPL_LOW );
		}
	    }
	} else {
	    // uninstall
	    if (uninstalled
		&& !uninstalled.status().isLocked())
	    {
		uninstalled.status().resetTransact( ResStatus::APPL_LOW );
	    }
	    if (installed
		&& !installed.status().isLocked())
	    {
		if (soft)
		    installed.status().setSoftTransact( true, ResStatus::APPL_LOW );
		else
		    installed.status().setTransact( true, ResStatus::APPL_LOW );
	    }
	}
	if (!uninstalled
	    && !installed)
	{
	    return false;
	}
    return true;
}


typedef struct { PoolItem_Ref installed; PoolItem_Ref uninstalled; } IandU;
typedef map<string, IandU> IandUMap;

// find best available providers for requested capability index
// (use capability index instead of name in order to find e.g. ccb vs. x11)

struct FindIandU
{
    IandUMap iandu;		// install, and best uninstalled

    FindIandU ()
    { }

    bool operator()( const CapAndItem & cai )
    {
	PoolItem item( cai.item );
	string idx = cai.cap.index();

	if ( item.status().staysInstalled() ) {
	    iandu[idx].installed = item;
	}
	else if ( item.status().isToBeInstalled() ) {			// prefer already to-be-installed
	    iandu[idx].uninstalled = item;
	}
	else if ( item.status().staysUninstalled() ) {			// only look at uninstalled
	    IandUMap::iterator it = iandu.find( idx );

	    if (it != iandu.end()
		&& it->second.uninstalled)
	    {								// uninstalled with same name found
		int cmp = it->second.uninstalled->arch().compare( item->arch() );
		if (cmp < 0) {						// new item has better arch
		    it->second.uninstalled = item;
		}
		else if (cmp == 0) {					// new item has equal arch
		    if (it->second.uninstalled->edition().compare( item->edition() ) < 0) {
			it->second.uninstalled = item;			// new item has better edition
		    }
		}
	    }
	    else {
		iandu[idx].uninstalled = item;
	    }
	}
	return true;
    }
};


//
// transact list of capabilities (requires or recommends)
//  return false if one couldn't be matched
//
// see Resolver::transactResObject
//

static bool
transactCaps( const ResPool & pool, const CapSet & caps, bool install, bool soft, std::set<PoolItem_Ref> & added_items )
{
    bool result = true;

    // loop over capabilities and find (best) matching provider

    for (CapSet::const_iterator cit = caps.begin(); cit != caps.end(); ++cit) {

	// find best providers of requested capability

	FindIandU callback;
	Dep dep( Dep::PROVIDES );
	invokeOnEach( pool.byCapabilityIndexBegin( cit->index(), dep ),
		      pool.byCapabilityIndexEnd( cit->index(), dep ),
		      resfilter::ByCapMatch( *cit ) ,
		      functor::functorRef<bool,CapAndItem>(callback) );

	// loop through providers and transact them accordingly

	for (IandUMap::const_iterator it = callback.iandu.begin(); it !=  callback.iandu.end(); ++it) {
	    PoolItem_Ref just_added;
	    just_added = PoolItem_Ref();
	    if (!transactItems( it->second.installed, it->second.uninstalled, install, soft, just_added )) {
		result = false;
	    }
	    else if (just_added) {
		// transactItems just selected an item of the same kind we're currently processing
		// add it to the list and handle it equivalent to the origin item
		added_items.insert( just_added );
	    }
	}

    }
    return result;
}


struct TransactSupplements : public resfilter::PoolItemFilterFunctor
{
    const Resolvable::Kind &_kind;
    bool valid;

    TransactSupplements( const Resolvable::Kind & kind )
	: _kind( kind )
	, valid( false )
    { }

    bool operator()( PoolItem_Ref item )
    {
//	MIL << "TransactSupplements(" << item << ")" << endl;
	if (item->kind() == _kind
	    && (item.status().staysInstalled()
		|| item.status().isToBeInstalled()))
	{
	    valid = true;
	    return false;		// end search here
	}
	return true;
    }
};

//
// transact due to a language dependency
// -> look through the pool and run transactResObject() accordingly

struct TransactLanguage : public resfilter::PoolItemFilterFunctor
{
    Resolver & _resolver;
    ResObject::constPtr _langObj;
    bool _install;

    TransactLanguage( Resolver & r, ResObject::constPtr langObj, bool install )
	: _resolver( r )
	, _langObj( langObj )
	, _install( install )
    { }

    /* item has a freshens on _langObj
	_langObj just transacted to _install (true == to-be-installed, false == to-be-uninstalled)
    */
    bool operator()( const CapAndItem & cai )
    {
	/* check for supplements, if the item has supplements these also must match  */

	PoolItem_Ref item( cai.item );
//	MIL << "TransactLanguage " << item << ", install " << _install << endl;
	CapSet supplements( item->dep( Dep::SUPPLEMENTS ) );
	if (!supplements.empty()) {
//	    MIL << "has supplements" << endl;
	    bool valid = false;
	    for (CapSet::const_iterator it = supplements.begin(); it != supplements.end(); ++it) {
//		MIL << "Checking " << *it << endl;
		TransactSupplements callback( it->refers() );
		invokeOnEach( _resolver.pool().byNameBegin( it->index() ),
			      _resolver.pool().byNameEnd( it->index() ),
			      functor::functorRef<bool,PoolItem>( callback ) );
		if (callback.valid) {
		    valid = true;		// found a supplements match
		    break;
		}
	    }
	    if (!valid) {
//		MIL << "All supplements false" << endl;
		return true;			// no supplements matched, we're done
	    }
	}

	PoolItem_Ref dummy;
	if (_install) {
	    if (item.status().staysUninstalled()) {
		transactItems( PoolItem_Ref(), item, _install, true, dummy );
	    }
	}
	else {
	    if (item.status().staysInstalled()) {
		transactItems( item, PoolItem_Ref(), _install, true, dummy );
	    }
	}
	return true;
    }
};



//
// transact a single object
// -> do a 'single step' resolving either installing or removing
//    required and recommended PoolItems

bool
Resolver::transactResObject( ResObject::constPtr robj, bool install,
			     bool recursive)
{
    static std::set<PoolItem_Ref> alreadyTransactedObjects;

    if (!recursive) {
	alreadyTransactedObjects.clear(); // first call
    }
    
    if (robj == NULL) {
	ERR << "NULL ResObject" << endl;
    }
    _XDEBUG ( "transactResObject(" << *robj << ", " << (install?"install":"remove") << ")");

    if (robj->kind() == ResTraits<Language>::kind) {
	TransactLanguage callback( *this, robj, install );
	Dep dep( Dep::FRESHENS );
	invokeOnEach( pool().byCapabilityIndexBegin( robj->name(), dep ),
		      pool().byCapabilityIndexEnd( robj->name(), dep ),
		      functor::functorRef<bool,CapAndItem>( callback ) );

    }
    std::set<PoolItem_Ref> added;

    // loop over 'recommends' and 'requires' of this item and collect additional
    //  items of the same kind on the way

    transactCaps( _pool, robj->dep( Dep::RECOMMENDS ), install, true, added );
    transactCaps( _pool, robj->dep( Dep::REQUIRES ), install, false, added );

    // if any additional items were collected, call this functions again recursively
    //   This is guaranteed to finish since additional items are those which were not selected before
    //   and this function is only called for already selected items. So added really only contains
    //   'new' items.

    for (std::set<PoolItem_Ref>::const_iterator it = added.begin(); it != added.end(); ++it) {
	if ((*it)->kind() == robj->kind()) {
	    std::set<PoolItem_Ref>::const_iterator itCmp = alreadyTransactedObjects.find (*it);
	    if (itCmp == alreadyTransactedObjects.end())
	    {
		// not already transacted
		alreadyTransactedObjects.insert (*it);
		transactResObject( it->resolvable(), install,
				   true //recursive
				   );
	    }
	}
    }

    // not used anyway
    return true;
}

//
// helper to transact all objects of a specific kind
//  see Resolver::transactResKind
// item is to-be-installed (install == true) or to-be-uninstalled
// -> run transactResObject() accordingly

struct TransactKind : public resfilter::PoolItemFilterFunctor
{
    Resolver & _resolver;
    bool install;		// true if to-be-installed, else to-be-uninstalled
    bool result;

    TransactKind( Resolver & r )
	: _resolver( r )
	, result( true )
    { }

    bool operator()( PoolItem_Ref item )
    {
	result = _resolver.transactResObject( item.resolvable(), install );
	return true;
    }
};


bool
Resolver::transactResKind( Resolvable::Kind kind )
{
    TransactKind callback (*this);
    _XDEBUG( "transactResKind(" << kind << ")" );

    // check all uninstalls
    callback.install = false;
    invokeOnEach( pool().byKindBegin( kind ),
		  pool().byKindEnd( kind ),
		  functor::chain( resfilter::ByTransact(), resfilter::ByInstalled ()),
		  functor::functorRef<bool,PoolItem>( callback ) );

    // check all installs
    callback.install = true;
    invokeOnEach( pool().byKindBegin( kind ),
		  pool().byKindEnd( kind ),
		  functor::chain( resfilter::ByTransact(), resfilter::ByUninstalled ()),
		  functor::functorRef<bool,PoolItem>( callback ) );

    return callback.result;
}


struct TransactReset : public resfilter::PoolItemFilterFunctor
{
    ResStatus::TransactByValue _causer;
    TransactReset( ResStatus::TransactByValue causer )
	: _causer( causer )
    { }

    bool operator()( PoolItem_Ref item )		// only transacts() items go here
    {
	item.status().resetTransact( _causer );
	return true;
    }
};


void
Resolver::transactReset( ResStatus::TransactByValue causer )
{
    TransactReset info( causer );
    MIL << "transactReset(" << causer << ")" << endl;
    invokeOnEach ( _pool.begin(), _pool.end(),
		   resfilter::ByTransact( ),
		   functor::functorRef<bool,PoolItem>(info) );
    return;
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

