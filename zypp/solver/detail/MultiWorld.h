/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* MultiWorld.h
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

#ifndef _MultiWorld_h
#define _MultiWorld_h

#include <iosfwd>
#include <string.h>

#include <zypp/solver/detail/MultiWorldPtr.h>
#include <zypp/solver/detail/ServiceWorldPtr.h>
#include <zypp/solver/detail/World.h>
#include <zypp/solver/detail/Pending.h>

///////////////////////////////////////////////////////////////////
namespace ZYPP {
//////////////////////////////////////////////////////////////////

class SubWorldInfo;
class NameConflictInfo;
class ForeachByTypeInfo;

typedef std::list <SubWorldInfo *> SubWorldInfoList;

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : MultiWorld

class MultiWorld : public World {
    REP_BODY(MultiWorld);

  private:

    SubWorldInfoList _subworlds;

    PendingPtr _multi_pending;
    PendingList _subworld_pendings;

    void (*_subworld_added)   (WorldPtr subworld);
    void (*_subworld_removed) (WorldPtr subworld);

  public:

    MultiWorld ();
    MultiWorld (XmlNodePtr node);
    MultiWorld (const char *filename);
    virtual ~MultiWorld();

    // ---------------------------------- I/O

    static std::string toString (const MultiWorld & section);

    virtual std::ostream & dumpOn(std::ostream & str ) const;

    friend std::ostream& operator<<(std::ostream&, const MultiWorld & section);

    std::string asString (void ) const;

    // ---------------------------------- accessors

    void addSubworld (WorldPtr subworld);
    void removeSubworld (WorldPtr subworld);

    // ---------------------------------- methods

    virtual ChannelList channels () const;
    virtual bool containsChannel (constChannelPtr channel) const;
    virtual ChannelPtr getChannelByName (const char *channel_name) const;
    virtual ChannelPtr getChannelByAlias (const char *alias) const;
    virtual ChannelPtr getChannelById (const char *channel_id) const;
    virtual ChannelPtr guessResolvableChannel (constResolvablePtr resolvable) const;
    virtual int foreachChannel (ChannelFn fn, void *data) const;

    int foreachSubworld (WorldFn callback, void *user_data);
    int foreachSubworldByType (WorldType type, WorldFn callback, NameConflictInfo *info);
    WorldList getSubworlds ();
    ServiceWorldPtr lookupService (const char *url);
    ServiceWorldPtr lookupServiceById (const char *id);
    bool mountService (const char *url, void *error);			// GError **error);

    // Single resolvable queries

    virtual constResolvablePtr findInstalledResolvable (constResolvablePtr resolvable);
    virtual constResolvablePtr findResolvable (constChannelPtr channel, const char *name) const;
    virtual constResolvablePtr findResolvableWithConstraint (constChannelPtr channel, const char *name, constDependencyPtr constraint, bool is_and) const;

    // Iterate over resolvables

    virtual int foreachResolvable (ChannelPtr channel, CResolvableFn fn, void *data);
    virtual int foreachResolvableByName (const std::string & name, ChannelPtr channel, CResolvableFn fn, void *data);
    virtual int foreachResolvableByMatch (constMatchPtr match, CResolvableFn fn, void *data);

    // Iterate across provides or requirement

    virtual int foreachProvidingResolvable (constDependencyPtr dep, ResolvableAndSpecFn fn, void *data);
    virtual int foreachRequiringResolvable (constDependencyPtr dep, ResolvableAndDepFn fn, void *data);
    virtual int foreachConflictingResolvable (constDependencyPtr dep, ResolvableAndDepFn fn, void *data);

    // locks

    virtual int foreachLock (MatchFn fn, void *data) const;

};
    
///////////////////////////////////////////////////////////////////
}; // namespace ZYPP
///////////////////////////////////////////////////////////////////

#endif // _MultiWorld_h
