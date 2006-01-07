/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* World.h
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

#ifndef ZYPP_SOLVER_DETAIL_WORLD_H
#define ZYPP_SOLVER_DETAIL_WORLD_H

#include <iosfwd>
#include <list>
#include <string>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/solver/detail/WorldPtr.h"
#include "zypp/solver/detail/MultiWorldPtr.h"
#include "zypp/solver/detail/ResItem.h"
#include "zypp/solver/detail/Channel.h"
#include "zypp/solver/detail/Match.h"
#include "zypp/solver/detail/Pending.h"
#include "zypp/solver/detail/Package.h"
#include "zypp/solver/detail/PackageUpdate.h"
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

      typedef std::list <World_constPtr> WorldList;

      class NameConflictInfo;

      //////////////////////////////////////////////////////////////////

      typedef enum {
          PLAIN_WORLD = 0,
          STORE_WORLD,
          MULTI_WORLD,
          SERVICE_WORLD,
          UNDUMP_WORLD,
          LOCALDIR_WORLD,
          SYSTEM_WORLD
      } WorldType;

      typedef bool		(*WorldFn)	  (World_constPtr world, void *user_data);
      typedef Pending_Ptr	(*WorldRefreshFn) (World_constPtr world);

      #if 0
      typedef bool		(*WorldSyncFn)    (World_constPtr world, Channel_constPtr channel);
      typedef void		(*WorldSpewFn)	  (World_constPtr world, FILE *out);
      typedef World_constPtr	(*WorldDupFn)	  (World_constPtr world);

      typedef bool		(*WorldCanTransactResItemFn) (World_constPtr world, ResItem_constPtr resItem);
      typedef bool		(*WorldTransactFn) (World_constPtr world, const ResItemList & install_resItems, const ResItemList & remove_resItems, int flags);

      typedef bool		(*WorldGetSubscribedFn) (const World *world, Channel_constPtr channel);
      typedef void		(*WorldSetSubscribedFn) (World *world, Channel_Ptr channel, bool subs_status);

      typedef int		(*WorldForeachChannelFn) (const World *world, ChannelFn callback, void *user_data);
      typedef int		(*WorldForeachLockFn)    (World_constPtr world, MatchFn callback, void *user_data);

      typedef void		(*WorldAddLockFn) (World_constPtr world, Match_constPtr lock);
      typedef void		(*WorldRemoveLockFn) (World_constPtr world, Match_constPtr lock);
      typedef void		(*WorldClearLockFn) (World_constPtr world);

      typedef int		(*WorldForeachResItemFn) (World_constPtr world, const char *name, Channel_constPtr channel, ResItemFn callback, void *user_data);
      typedef int		(*WorldForeachPackageDepFn) (World_constPtr world, const Capability & dep, ResItemAndDepFn callback, void *user_data);

      typedef void		(*WorldSerializeFn) (World_constPtr world, XmlNode_constPtr root);
      typedef void		(*WorldUnserializeFn) (World_constPtr world, XmlNode_constPtr node);

      #endif

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : World

      class World : public base::ReferenceCounted, private base::NonCopyable {


        private:
          static World_Ptr GlobalWorld;

          WorldType _type;

          /* The sequence numbers gets incremented every
             time the RCWorld is changed. */

          unsigned int _seq_no_resItems;
          unsigned int _seq_no_channels;
          unsigned int _seq_no_subscriptions;
          unsigned int _seq_no_locks;

          /* Every world needs to be able to store locks, so we provide a
             place for that.  Of course, derived classes are allowed to
             provide their own exotic lock semantics by providing their
             own *_lock_fn methods. */
          MatchList _lock_store;

          bool _refresh_pending;

          /* a bad hack to keep us from emitting signals while finalizing */
          bool _no_changed_signals;

          /* For unserialized worlds currently. If a world is read only,
             you can not refresh or transact on it. */
          bool _read_only;

          MatchList _locks;

        public:

          World (WorldType type = PLAIN_WORLD);
          virtual ~World();

          // ---------------------------------- I/O

          static std::string toString (const World & section);

          static std::string toString (WorldType type);

          virtual std::ostream & dumpOn(std::ostream & str ) const;

          friend std::ostream& operator<<(std::ostream&, const World & section);

          std::string asString (void ) const;

          // ---------------------------------- accessors

          WorldType type() const { return _type; }
          bool isPlainWorld () const { return _type == PLAIN_WORLD; }
          bool isUndumpWorld () const { return _type == UNDUMP_WORLD; }
          bool isMultiWorld () const { return _type == MULTI_WORLD; }
          bool isServiceWorld () const { return _type == SERVICE_WORLD; }

          unsigned int resItemSequenceNumber (void) const { return _seq_no_resItems; }
          unsigned int channelSequenceNumber (void) const { return _seq_no_channels; }
          unsigned int subscriptionSequenceNumber (void) const { return _seq_no_subscriptions; }
          unsigned int lockSequenceNumber (void) const { return _seq_no_locks; }

          void touchResItemSequenceNumber (void) { _seq_no_resItems++; }
          void touchChannelSequenceNumber (void) { _seq_no_channels++; }
          void touchSubscriptionSequenceNumber (void) { _seq_no_subscriptions++; }
          void touchLockSequenceNumber (void) { _seq_no_locks++; }

          MatchList locks (void) const { return _lock_store; }

          // ---------------------------------- methods

          static void setGlobalWorld (MultiWorld_Ptr world);
          static MultiWorld_Ptr globalWorld (void);

          bool sync (void) const;
          virtual bool syncConditional (Channel_constPtr channel) const;
          Pending_Ptr refresh (void);
          bool hasRefresh (void);
          bool isRefreshing (void);

      	/* These functions are for World-implementers only!  Don't call them! */
          void refreshBegin (void);
          void refreshComplete (void);

          virtual int foreachChannel (ChannelFn fn, void *user_data) const = 0;

          virtual void setSubscription (Channel_Ptr channel, bool is_subscribed);
          virtual bool isSubscribed (Channel_constPtr channel) const;

          virtual ChannelList channels () const = 0;
          virtual bool containsChannel (Channel_constPtr channel) const = 0;

          virtual Channel_Ptr getChannelByName (const char *channel_name) const = 0;
          virtual Channel_Ptr getChannelByAlias (const char *alias) const = 0;
          virtual Channel_Ptr getChannelById (const char *channel_id) const = 0;

          // ResItem Locks

          virtual int foreachLock (MatchFn fn, void *data) const;

          void addLock (Match_constPtr lock);
          void removeLock (Match_constPtr lock);
          void clearLocks ();

          bool resItemIsLocked (ResItem_constPtr resItem);

          // Single resItem queries

          virtual ResItem_constPtr findInstalledResItem (ResItem_constPtr resItem) = 0;
          virtual ResItem_constPtr findResItem (Channel_constPtr channel, const char *name) const = 0;
          virtual ResItem_constPtr findResItemWithConstraint (Channel_constPtr channel, const char *name, const Capability &  constraint, bool is_and) const = 0;
          virtual Channel_Ptr guessResItemChannel (ResItem_constPtr resItem) const = 0;

          // Iterate across resItems

          virtual int foreachResItem (Channel_Ptr channel, CResItemFn fn, void *data) = 0;
          virtual int foreachResItemByName (const std::string & name, Channel_Ptr channel, CResItemFn fn, void *user_data) = 0;
          virtual int foreachResItemByMatch (Match_constPtr match, CResItemFn fn, void *user_data) = 0;

          // Iterate across provides or requirement

          virtual int foreachProvidingResItem (const Capability & dep, ResItemAndDepFn fn, void *user_data) = 0;
          virtual int foreachRequiringResItem (const Capability & dep, ResItemAndDepFn fn, void *user_data) = 0;
          virtual int foreachConflictingResItem (const Capability & dep, ResItemAndDepFn fn, void *user_data) = 0;

          // upgrades

          int foreachUpgrade (ResItem_constPtr resItem, Channel_Ptr channel, CResItemFn fn, void *data);
          PackageUpdateList getUpgrades (ResItem_constPtr resItem, Channel_constPtr channel);
          ResItem_constPtr getBestUpgrade (ResItem_constPtr resItem, bool subscribed_only);
          int foreachSystemUpgrade (bool subscribed_only, ResItemPairFn fn, void *data);

          // provider

          bool getSingleProvider (const Capability & dep, Channel_constPtr channel, ResItem_constPtr *resItem);

          // Transacting

          bool  canTransactResItem (ResItem_constPtr resItem);
          bool  transact (const ResItemList & installResItems, const ResItemList & remove_resItems, int flags);

          // XML serialization

          void serialize (XmlNode_Ptr parent);
          void toFile (const char *filename);

          // Duplicating (primarily for atomic refreshes)
          World_Ptr dup (void);

          // only used for bindings
          void setRefreshFunction (WorldRefreshFn refresh_fn);

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

#endif // ZYPP_SOLVER_DETAIL_WORLD_H
