/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <zypp/solver/detail/Resolver.h>
#include <zypp/solver/detail/ResolverContext.h>
#include <zypp/solver/detail/ResolverQueue.h>
#include <zypp/solver/detail/ResItem.h>
#include <zypp/solver/detail/Version.h>
#include <zypp/solver/detail/World.h>
#include <zypp/solver/detail/StoreWorld.h>
#include <zypp/solver/detail/MultiWorld.h>

///////////////////////////////////////////////////////////////////
namespace zypp {
//////////////////////////////////////////////////////////////////

using namespace std;

IMPL_BASE_POINTER(Resolver);

//---------------------------------------------------------------------------

string
Resolver::asString ( void ) const
{
    return toString (*this);
}


string
Resolver::toString ( const Resolver & resolver )
{
    return "<resolver/>";
}


ostream &
Resolver::dumpOn( ostream & str ) const
{
    str << asString();
    return str;
}


ostream&
operator<<( ostream& os, const Resolver & resolver)
{
    return os << resolver.asString();
}

//---------------------------------------------------------------------------

Resolver::Resolver (WorldPtr world)
    : _current_channel (NULL)
    , _world (world)
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

WorldPtr
Resolver::world (void) const
{
    if (_world == NULL)
	return World::globalWorld();

    return _world;
}

//---------------------------------------------------------------------------

void
Resolver::addSubscribedChannel (constChannelPtr channel)
{
    fprintf (stderr, "Resolver::addSubscribedChannel() not implemented\n");
}

void
Resolver::addResItemToInstall (constResItemPtr resItem)
{
    _resItems_to_install.push_front (resItem);
}

void
Resolver::addResItemsToInstallFromList (CResItemList & rl)
{
    for (CResItemList::const_iterator iter = rl.begin(); iter != rl.end(); iter++) {
	addResItemToInstall (*iter);
    }
}

void
Resolver::addResItemToRemove (constResItemPtr resItem)
{
    _resItems_to_remove.push_front (resItem);
}

void
Resolver::addResItemsToRemoveFromList (CResItemList & rl)
{
    for (CResItemList::const_iterator iter = rl.begin(); iter != rl.end(); iter++) {
	addResItemToRemove (*iter);
    }
}

void
Resolver::addResItemToVerify (constResItemPtr resItem)
{
    _resItems_to_verify.push_front (resItem);
    _resItems_to_verify.sort ();			//(GCompareFunc) rc_resItem_compare_name);
}

void
Resolver::addExtraDependency (constDependencyPtr dependency)
{
    _extra_deps.push_front (dependency);
}

void
Resolver::addExtraConflict (constDependencyPtr dependency)
{
    _extra_conflicts.push_front (dependency);
}


//---------------------------------------------------------------------------

static bool
verify_system_cb (constResItemPtr resItem, void *data)
{
    Resolver *resolver  = (Resolver *)data;

    resolver->addResItemToVerify (resItem);

    return true;
}


void
Resolver::verifySystem (void)
{
    if (getenv ("RC_SPEW")) fprintf (stderr, "Resolver::verifySystem()\n");
    world()->foreachResItem (new Channel(CHANNEL_TYPE_SYSTEM), verify_system_cb, this);

    _verifying = true;

#if 0		// commented out in libredcarpet also
    /*
      Walk across the (sorted-by-name) list of installed packages and look for
      packages with the same name.  If they exist, construct a branch item
      containing multiple group items.  Each group item corresponds to removing
      all but one of the duplicates.
    */

    for (CResItemList::const_iterator i0 = _resItems_to_verify.begin(); i0 != _resItems_to_verify.end();) {
	CResItemList::const_iterator i1 = i0;
	i1++;
	CResItemList::const_iterator i2 = i1;
	for (; i1 != _resItems_to_verify.end()&& ! (*i0)->compareName (*i1); i1++) {
	    //empty
	}

	if (i1 != i2) {
	    QueueItemBranchPtr branch_item;

	    branch_item = new QueueItemBranch(world());

	    for (CResItemList::const_iterator i = i0; i != i1; i++) {

		QueueItemGroupPtr grp_item = new QueueItemGroup(world());

		for (CResItemList::const_iterator j = i0; j != i1; j++) {
		    constPackagePtr dup_pkg = *j;
		    QueueItemUninstallPtr uninstall_item;

		    if (i != j) {
			uninstall_item = new QueueItemUninstall (world(), dup_pkg, "duplicate install");
			grp_item->addItem (uninstall_item);
		    }

		}

		branch_item->adddIitem (grp_item);
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


void
Resolver::resolveDependencies (void)
{

    time_t t_start, t_now;
    bool extremely_noisy = getenv ("RC_SPEW") != NULL;
    bool have_local_resItems = false;

    if (extremely_noisy) fprintf (stderr, "Resolver::resolveDependencies()\n");

    /* Walk through are list of to-be-installed packages and see if any of them are local. */

    for (CResItemList::const_iterator iter = _resItems_to_install.begin(); iter != _resItems_to_install.end(); iter++) {
	if ((*iter)->local()) {
	    have_local_resItems = true;
	    break;
	}
    }

    WorldPtr the_world = world();
    StoreWorldPtr local_world = NULL;
    MultiWorldPtr local_multiworld = NULL;

    ChannelPtr local_channel = NULL;

    if (have_local_resItems) {
	local_multiworld = new MultiWorld();
	local_world = new StoreWorld();

	local_channel = new Channel ("", "Local ResItems", "@local", "");

	local_world->addChannel (local_channel);

	local_multiworld->addSubworld (local_world);
	local_multiworld->addSubworld (the_world);

	the_world = local_multiworld;
    }

    // create initial_queue

    ResolverQueuePtr initial_queue = new ResolverQueue();

    /* Stick the current/subscribed channel and world info into the context */

    initial_queue->context()->setWorld(the_world);

    initial_queue->context()->setCurrentChannel (_current_channel);

    /* If this is a verify, we do a "soft resolution" */

    initial_queue->context()->setVerifying (_verifying);

    /* Add extra items. */

    for (QueueItemList::const_iterator iter = _initial_items.begin(); iter != _initial_items.end(); iter++) {
	initial_queue->addItem (*iter);
    }
    _initial_items.clear();

    for (CResItemList::const_iterator iter = _resItems_to_install.begin(); iter != _resItems_to_install.end(); iter++) {
	constResItemPtr r = *iter;

	/* Add local packages to our dummy channel. */
	if (r->local()) {
	    assert (local_channel != NULL);
	    ResItemPtr r1 = ResItemPtr::cast_away_const (r);
	    r1->setChannel (local_channel);
	    local_world->addResItem (r);
	}

	initial_queue->addResItemToInstall (r);
    }

    for (CResItemList::const_iterator iter = _resItems_to_remove.begin(); iter != _resItems_to_remove.end(); iter++) {
	initial_queue->addResItemToRemove (*iter, true /* remove-only mode */);
    }

    for (CResItemList::const_iterator iter = _resItems_to_verify.begin(); iter != _resItems_to_verify.end(); iter++) {
	initial_queue->addResItemToVerify (*iter);
    }

    for (CDependencyList::const_iterator iter = _extra_deps.begin(); iter != _extra_deps.end(); iter++) {
	initial_queue->addExtraDependency (*iter);
    }

    for (CDependencyList::const_iterator iter = _extra_conflicts.begin(); iter != _extra_conflicts.end(); iter++) {
	initial_queue->addExtraConflict (*iter);
    }

    if (extremely_noisy) fprintf (stderr, "Initial Queue: [%s]\n", initial_queue->asString().c_str());

    _pending_queues.push_front (initial_queue);

    time (&t_start);

    while (!_pending_queues.empty()) {

	if (extremely_noisy) {
	    printf ("Pend %ld / Cmpl %ld / Prun %ld / Defr %ld / Invl %ld\n\n", (long) _pending_queues.size(), (long) _complete_queues.size(), (long) _pruned_queues.size(), (long) _deferred_queues.size(), (long) _invalid_queues.size());
	}

	if (_timeout_seconds > 0) {
	    time (&t_now);
	    if (difftime (t_now, t_start) > _timeout_seconds) {
		_timed_out = true;
		break;
	    }
	}

	ResolverQueuePtr queue = _pending_queues.front();
	_pending_queues.pop_front();
	ResolverContextPtr context = queue->context();

	queue->process();

	if (queue->isInvalid ()) {
	    if (extremely_noisy) printf ("Invalid Queue\n");
	    _invalid_queues.push_front (queue);

	} else if (queue->isEmpty ()) {
	    if (extremely_noisy) printf ("Empty Queue\n");

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

	    if (extremely_noisy) printf ("PRUNED!\n");

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

    if (extremely_noisy) {
	printf ("Pend %ld / Cmpl %ld / Prun %ld / Defr %ld / Invl %ld\n--------\n", (long) _pending_queues.size(), (long) _complete_queues.size(), (long) _pruned_queues.size(), (long) _deferred_queues.size(), (long) _invalid_queues.size());
    }

    return;
}

///////////////////////////////////////////////////////////////////
}; // namespace zypp
///////////////////////////////////////////////////////////////////

