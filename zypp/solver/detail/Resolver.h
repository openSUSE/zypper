/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* Resolver.h
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

#ifndef _Resolver_h
#define _Resolver_h

#include <iosfwd>
#include <list>
#include <string.h>

#include <zypp/solver/detail/ResolverPtr.h>
#include <zypp/solver/detail/ResolverQueue.h>
#include <zypp/solver/detail/Resolvable.h>
#include <zypp/solver/detail/Dependency.h>
#include <zypp/solver/detail/Channel.h>

///////////////////////////////////////////////////////////////////
namespace ZYPP {


//////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : Resolver

class Resolver : public CountedRep {
    REP_BODY(Resolver);

  private:
    constChannelPtr _current_channel;
    
    WorldPtr _world;

    int _timeout_seconds;
    bool _verifying;

    QueueItemList _initial_items;
    CResolvableList _resolvables_to_install;
    CResolvableList _resolvables_to_remove;
    CResolvableList _resolvables_to_verify;

    CDependencyList _extra_deps;
    CDependencyList _extra_conflicts;

    ResolverQueueList _pending_queues;
    ResolverQueueList _pruned_queues;
    ResolverQueueList _complete_queues;
    ResolverQueueList _deferred_queues;
    ResolverQueueList _invalid_queues;
    
    int _valid_solution_count;

    ResolverContextPtr _best_context;
    bool _timed_out;

  public:

    Resolver (WorldPtr world = NULL);
    virtual ~Resolver();

    // ---------------------------------- I/O

    static std::string toString (const Resolver & resolver);

    virtual std::ostream & dumpOn(std::ostream & str ) const;

    friend std::ostream& operator<<(std::ostream&, const Resolver & resolver);

    std::string asString (void ) const;

    // ---------------------------------- accessors

    QueueItemList initialItems () const { return _initial_items; }

    ResolverQueueList pendingQueues () const { return _pending_queues; }
    ResolverQueueList prunedQueues () const { return _pruned_queues; }
    ResolverQueueList completeQueues () const { return _complete_queues; }
    ResolverQueueList deferredQueues () const { return _deferred_queues; }
    ResolverQueueList invalidQueues () const { return _invalid_queues; }

    ResolverContextPtr bestContext (void) const { return _best_context; }

    // ---------------------------------- methods

    void setTimeout (int seconds) { _timeout_seconds = seconds; }

    WorldPtr world (void) const;			// returns global world if _world == NULL
    void setWorld (WorldPtr world) { _world = world; }

    void setCurrentChannel (constChannelPtr channel) { _current_channel = channel; }
    void addSubscribedChannel (constChannelPtr channel);

    void addResolvableToInstall (constResolvablePtr resolvable);
    void addResolvablesToInstallFromList (CResolvableList & rl);

    void addResolvableToRemove (constResolvablePtr resolvable);
    void addResolvablesToRemoveFromList (CResolvableList & rl);

    void addResolvableToVerify (constResolvablePtr resolvable);

    void addExtraDependency (constDependencyPtr dependency);
    void addExtraConflict (constDependencyPtr dependency);

    void verifySystem (void);
    void resolveDependencies (void);

};
    
///////////////////////////////////////////////////////////////////
}; // namespace ZYPP
///////////////////////////////////////////////////////////////////

#endif // _Resolver_h
