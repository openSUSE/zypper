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

#ifndef _World_h
#define _World_h

#include <iosfwd>
#include <list>
#include <string.h>

#include <zypp/solver/detail/WorldPtr.h>
#include <zypp/solver/detail/MultiWorldPtr.h>
#include <zypp/solver/detail/ResItem.h>
#include <zypp/solver/detail/Channel.h>
#include <zypp/solver/detail/Match.h>
#include <zypp/solver/detail/Pending.h>
#include <zypp/solver/detail/Packman.h>
#include <zypp/solver/detail/Package.h>
#include <zypp/solver/detail/PackageUpdate.h>
#include <zypp/Capability.h>

/////////////////////////////////////////////////////////////////////////
namespace zypp 
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////
          
      typedef std::list <constWorldPtr> WorldList;
      
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
      
      typedef bool		(*WorldFn)	  (constWorldPtr world, void *user_data);
      typedef PendingPtr	(*WorldRefreshFn) (constWorldPtr world);
      
      #if 0
      typedef bool		(*WorldSyncFn)    (constWorldPtr world, constChannelPtr channel);
      typedef PackmanPtr	(*WorldPackmanFn) (constWorldPtr world, const Resolvable::Kind & kind);
      typedef void		(*WorldSpewFn)	  (constWorldPtr world, FILE *out);
      typedef constWorldPtr	(*WorldDupFn)	  (constWorldPtr world);
      
      typedef bool		(*WorldCanTransactResItemFn) (constWorldPtr world, constResItemPtr resItem);
      typedef bool		(*WorldTransactFn) (constWorldPtr world, const ResItemList & install_resItems, const ResItemList & remove_resItems, int flags);
      
      typedef bool		(*WorldGetSubscribedFn) (const World *world, constChannelPtr channel);
      typedef void		(*WorldSetSubscribedFn) (World *world, ChannelPtr channel, bool subs_status);
      
      typedef int		(*WorldForeachChannelFn) (const World *world, ChannelFn callback, void *user_data);
      typedef int		(*WorldForeachLockFn)    (constWorldPtr world, MatchFn callback, void *user_data);
      
      typedef void		(*WorldAddLockFn) (constWorldPtr world, constMatchPtr lock);
      typedef void		(*WorldRemoveLockFn) (constWorldPtr world, constMatchPtr lock);
      typedef void		(*WorldClearLockFn) (constWorldPtr world);
      
      typedef int		(*WorldForeachResItemFn) (constWorldPtr world, const char *name, constChannelPtr channel, ResItemFn callback, void *user_data);
      typedef int		(*WorldForeachPackageDepFn) (constWorldPtr world, const Capability & dep, ResItemAndDepFn callback, void *user_data);
      
      typedef void		(*WorldSerializeFn) (constWorldPtr world, constXmlNodePtr root);
      typedef void		(*WorldUnserializeFn) (constWorldPtr world, constXmlNodePtr node);
      
      #endif
      
      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : World
      
      class World : public CountedRep {
          REP_BODY(World);
      
        private:
          static WorldPtr GlobalWorld;
      
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
      
          static void setGlobalWorld (MultiWorldPtr world) { GlobalWorld = world; }
          static MultiWorldPtr globalWorld (void) { return GlobalWorld; }
      
      	//RCPackman *get_packman      (GType);
      
          bool sync (void) const;
          virtual bool syncConditional (constChannelPtr channel) const;
          PendingPtr refresh (void);
          bool hasRefresh (void);
          bool isRefreshing (void);
      
      	/* These functions are for World-implementers only!  Don't call them! */
          void refreshBegin (void);
          void refreshComplete (void);
      
          virtual int foreachChannel (ChannelFn fn, void *user_data) const = 0;
      
          virtual void setSubscription (ChannelPtr channel, bool is_subscribed);
          virtual bool isSubscribed (constChannelPtr channel) const;
      
          virtual ChannelList channels () const = 0;
          virtual bool containsChannel (constChannelPtr channel) const = 0;
      
          virtual ChannelPtr getChannelByName (const char *channel_name) const = 0;
          virtual ChannelPtr getChannelByAlias (const char *alias) const = 0;
          virtual ChannelPtr getChannelById (const char *channel_id) const = 0;
      
          // ResItem Locks
      
          virtual int foreachLock (MatchFn fn, void *data) const;
      
          void addLock (constMatchPtr lock);
          void removeLock (constMatchPtr lock);
          void clearLocks ();
      
          bool resItemIsLocked (constResItemPtr resItem);
      
          // Single resItem queries
      
          virtual constResItemPtr findInstalledResItem (constResItemPtr resItem) = 0;
          virtual constResItemPtr findResItem (constChannelPtr channel, const char *name) const = 0;
          virtual constResItemPtr findResItemWithConstraint (constChannelPtr channel, const char *name, const Capability &  constraint, bool is_and) const = 0;
          virtual ChannelPtr guessResItemChannel (constResItemPtr resItem) const = 0;
      
          // Iterate across resItems
      
          virtual int foreachResItem (ChannelPtr channel, CResItemFn fn, void *data) = 0;
          virtual int foreachResItemByName (const std::string & name, ChannelPtr channel, CResItemFn fn, void *user_data) = 0;
          virtual int foreachResItemByMatch (constMatchPtr match, CResItemFn fn, void *user_data) = 0;
      
          // Iterate across provides or requirement
      
          virtual int foreachProvidingResItem (const Capability & dep, ResItemAndDepFn fn, void *user_data) = 0;
          virtual int foreachRequiringResItem (const Capability & dep, ResItemAndDepFn fn, void *user_data) = 0;
          virtual int foreachConflictingResItem (const Capability & dep, ResItemAndDepFn fn, void *user_data) = 0;
      
          // upgrades
      
          int foreachUpgrade (constResItemPtr resItem, ChannelPtr channel, CResItemFn fn, void *data);
          PackageUpdateList getUpgrades (constResItemPtr resItem, constChannelPtr channel);
          constResItemPtr getBestUpgrade (constResItemPtr resItem, bool subscribed_only);
          int foreachSystemUpgrade (bool subscribed_only, ResItemPairFn fn, void *data);
      
          // provider
      
          bool getSingleProvider (const Capability & dep, constChannelPtr channel, constResItemPtr *resItem);
      
          // Transacting
      
          bool  canTransactResItem (constResItemPtr resItem);
          bool  transact (const ResItemList & installResItems, const ResItemList & remove_resItems, int flags);
      
          // XML serialization
      
          void serialize (XmlNodePtr parent);
          void toFile (const char *filename);
      
          // Duplicating (primarily for atomic refreshes)
          WorldPtr dup (void);
      
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

#endif // _World_h
