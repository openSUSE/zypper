/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* StoreWorld.h
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

#ifndef _StoreWorld_h
#define _StoreWorld_h

#include <iosfwd>
#include <string>
#include <list>
#include <map>

#include <zypp/solver/detail/StoreWorldPtr.h>
#include <zypp/solver/detail/ResolvableAndDependency.h>
#include <zypp/solver/detail/PackmanPtr.h>
#include <zypp/solver/detail/World.h>
#include <zypp/solver/detail/Resolvable.h>
#include <zypp/solver/detail/Match.h>

///////////////////////////////////////////////////////////////////
namespace ZYPP {
//////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : StoreWorld

class StoreWorld : public World {
    REP_BODY(StoreWorld);

  private:

    int _freeze_count;

    ResolvableTable _resolvables_by_name;
    ResolvableAndDependencyTable _provides_by_name;
    ResolvableAndDependencyTable _requires_by_name;
    ResolvableAndDependencyTable _conflicts_by_name;

    PackmanPtr _packman;
    Kind _resolvable_kind;

    ChannelList _channels;

  public:

    StoreWorld (WorldType type = STORE_WORLD);
    virtual ~StoreWorld();

    // ---------------------------------- I/O

    static std::string toString (const StoreWorld & storeworld);

    virtual std::ostream & dumpOn(std::ostream & str ) const;

    friend std::ostream& operator<<(std::ostream&, const StoreWorld & storeworld);

    std::string asString (void ) const;

    // ---------------------------------- accessors

    virtual ChannelList channels () const { return _channels; }

    // ---------------------------------- methods

    // Add/remove resolvables

    bool addResolvable (constResolvablePtr resolvable);
    void addResolvablesFromList (const CResolvableList & slist);
    void removeResolvable (constResolvablePtr resolvable);
    void removeResolvables (constChannelPtr channel);
    void clear ();

    // Iterate over resolvables

    virtual int foreachResolvable (ChannelPtr channel, CResolvableFn fn, void *data);
    virtual int foreachResolvableByName (const std::string & name, ChannelPtr channel, CResolvableFn fn, void *data);
    virtual int foreachResolvableByMatch (constMatchPtr match, CResolvableFn fn, void *data);

    // Iterate across provides or requirement

    virtual int foreachProvidingResolvable (constDependencyPtr dep, ResolvableAndSpecFn fn, void *data);
    virtual int foreachRequiringResolvable (constDependencyPtr dep, ResolvableAndDepFn fn, void *data);
    virtual int foreachConflictingResolvable (constDependencyPtr dep, ResolvableAndDepFn fn, void *data);

    // Channels

    void addChannel (ChannelPtr channel);
    void removeChannel (constChannelPtr channel);

    virtual bool containsChannel (constChannelPtr channel) const;

    virtual ChannelPtr getChannelByName (const char *channel_name) const;
    virtual ChannelPtr getChannelByAlias (const char *alias) const;
    virtual ChannelPtr getChannelById (const char *channel_id) const;

    virtual int foreachChannel (ChannelFn fn, void *data) const;

    // Single resolvable queries

    virtual constResolvablePtr findInstalledResolvable (constResolvablePtr resolvable);
    virtual constResolvablePtr findResolvable (constChannelPtr channel, const char *name) const;
    virtual constResolvablePtr findResolvableWithConstraint (constChannelPtr channel, const char *name, constDependencyPtr constraint, bool is_and) const;
    virtual ChannelPtr guessResolvableChannel (constResolvablePtr resolvable) const;

};
    
///////////////////////////////////////////////////////////////////
}; // namespace ZYPP
///////////////////////////////////////////////////////////////////

#endif // _StoreWorld_h
