/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
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

#ifndef ZYPP_SOLVER_TEMPORARY_MULTIWORLD_H
#define ZYPP_SOLVER_TEMPORARY_MULTIWORLD_H

#include <iosfwd>
#include <string>

#include "zypp/solver/temporary/MultiWorldPtr.h"
#include "zypp/solver/temporary/ServiceWorldPtr.h"
#include "zypp/solver/temporary/World.h"

#include "zypp/solver/detail/Pending.h"
#include "zypp/Capability.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp 
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

class SubWorldInfo;
class NameConflictInfo;
class ForeachByTypeInfo;

typedef std::list <SubWorldInfo *> SubWorldInfoList;

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : MultiWorld

class MultiWorld : public World {
    

        private:
      
          SubWorldInfoList _subworlds;
      
          Pending_Ptr _multi_pending;
          PendingList _subworld_pendings;
      
          void (*_subworld_added)   (World_Ptr subworld);
          void (*_subworld_removed) (World_Ptr subworld);
      
        public:
      
          MultiWorld ();
          MultiWorld (XmlNode_Ptr node);
          MultiWorld (const char *filename);
          virtual ~MultiWorld();
      
          // ---------------------------------- I/O
      
          static std::string toString (const MultiWorld & section);
      
          virtual std::ostream & dumpOn(std::ostream & str ) const;
      
          friend std::ostream& operator<<(std::ostream&, const MultiWorld & section);
      
          std::string asString (void ) const;
      
          // ---------------------------------- accessors
      
          void addSubworld (World_Ptr subworld);
          void removeSubworld (World_Ptr subworld);
      
          // ---------------------------------- methods
      
          virtual ChannelList channels () const;
          virtual bool containsChannel (Channel_constPtr channel) const;
          virtual Channel_Ptr getChannelByName (const char *channel_name) const;
          virtual Channel_Ptr getChannelByAlias (const char *alias) const;
          virtual Channel_Ptr getChannelById (const char *channel_id) const;
          virtual Channel_Ptr guessResItemChannel (ResItem_constPtr resItem) const;
          virtual int foreachChannel (ChannelFn fn, void *data) const;
      
          int foreachSubworld (WorldFn callback, void *user_data);
          int foreachSubworldByType (WorldType type, WorldFn callback, NameConflictInfo *info);
          WorldList getSubworlds ();
          ServiceWorld_Ptr lookupService (const char *url);
          ServiceWorld_Ptr lookupServiceById (const char *id);
          bool mountService (const char *url, void *error);			// GError **error);
      
          // Single resItem queries
      
          virtual ResItem_constPtr findInstalledResItem (ResItem_constPtr resItem);
          virtual ResItem_constPtr findResItem (Channel_constPtr channel, const char *name) const;
          virtual ResItem_constPtr findResItemWithConstraint (Channel_constPtr channel, const char *name, const Capability & constraint, bool is_and) const;
      
          // Iterate over resItems
      
          virtual int foreachResItem (Channel_Ptr channel, CResItemFn fn, void *data);
          virtual int foreachResItemByName (const std::string & name, Channel_Ptr channel, CResItemFn fn, void *data);
          virtual int foreachResItemByMatch (Match_constPtr match, CResItemFn fn, void *data);
      
          // Iterate across provides or requirement
      
          virtual int foreachProvidingResItem (const Capability & dep, ResItemAndDepFn fn, void *data);
          virtual int foreachRequiringResItem (const Capability & dep, ResItemAndDepFn fn, void *data);
          virtual int foreachConflictingResItem (const Capability & dep, ResItemAndDepFn fn, void *data);
      
          // locks
      
          virtual int foreachLock (MatchFn fn, void *data) const;
      
      };
          
      ///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////

#endif // ZYPP_SOLVER_TEMPORARY_MULTIWORLD_H
