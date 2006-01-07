/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* World.cc
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

#include <map>

#include "zypp/solver/temporary/MultiWorld.h"
#include "zypp/solver/temporary/Subscription.h"
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

      IMPL_PTR_TYPE(World);

      World_Ptr World::GlobalWorld = NULL;

      void World::setGlobalWorld( MultiWorld_Ptr world )
      {
        GlobalWorld = world;
      }

      MultiWorld_Ptr World::globalWorld()
      {
        return dynamic_pointer_cast<MultiWorld>(GlobalWorld);
      }

      //---------------------------------------------------------------------------

      string
      World::toString (WorldType type)
      {
          switch (type) {
      	case PLAIN_WORLD:	return "plain";
      	case STORE_WORLD:	return "store";
      	case MULTI_WORLD:	return "multi";
      	case SERVICE_WORLD:	return "service";
      	case UNDUMP_WORLD:	return "undump";
      	case LOCALDIR_WORLD:	return "localdir";
      	case SYSTEM_WORLD:	return "system";
      	default:
      		break;
          }
          return "???";
      }


      string
      World::asString ( void ) const
      {
          return toString (*this);
      }


      string
      World::toString ( const World & world )
      {
          return "<world/>";
      }


      ostream &
      World::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }


      ostream&
      operator<<( ostream& os, const World & world)
      {
          return os << world.asString();
      }

      //---------------------------------------------------------------------------

      World::World (WorldType type)
          : _type (type)
      {
      //    fprintf (stderr, "*** creating world[%p]: %s\n", this, toString(_type).c_str());
      }


      World::~World()
      {
      //    fprintf (stderr, "*** deleting world[%p]: %s\n", this, toString(_type).c_str());
      }

      //---------------------------------------------------------------------------
      // sync/refresh functions

      bool
      World::sync (void) const
      {
        if (getenv("FIXME"))     fprintf (stderr, "World::sync() not implemented\n");
        return false;
      }

      bool
      World::syncConditional (Channel_constPtr channel) const
      {
        if (getenv("FIXME"))    fprintf (stderr, "World::syncConditional() not implemented\n");
        return false;
      }


      Pending_Ptr
      World::refresh (void)
      {
        if (getenv("FIXME"))     fprintf (stderr, "World::refresh() not implemented\n");
        return 0;
      }

      bool
      World::hasRefresh (void)
      {
        if (getenv("FIXME"))     fprintf (stderr, "World::hasRefresh() not implemented\n");
        return false;
      }


      bool
      World::isRefreshing (void)
      {
        if (getenv("FIXME"))     fprintf (stderr, "World::isRefreshing() not implemented\n");
        return false;
      }


      	/* These functions are for World-implementers only!  Don't call them! */
      void
      World::refreshBegin (void)
      {
        if (getenv("FIXME"))     fprintf (stderr, "World::refreshBegin() not implemented\n");
        return;
      }

      void
      World::refreshComplete (void)
      {
        if (getenv("FIXME"))     fprintf (stderr, "World::refreshComplete() not implemented\n");
        return;
      }

      //---------------------------------------------------------------------------
      // channels, subscriptions

      void
      World::setSubscription (Channel_Ptr channel, bool is_subscribed)
      {
          bool curr_subs_status;

          if (channel == NULL) return;

	  _XXX("RC_SPEW") << "World::setSubscription (" << channel->asString() << ", " <<
	      (is_subscribed?"subscribe":"unsubscribe") << ")" << endl;

          if (channel->system ()) {
      	fprintf (stderr, "Can't subscribe to system channel '%s'\n",  channel->name ());
      	return;
          }

          curr_subs_status = isSubscribed (channel);

          Subscription::setStatus (channel, is_subscribed);

          if (curr_subs_status != isSubscribed (channel))
      	touchSubscriptionSequenceNumber ();

          return;
      }


      bool
      World::isSubscribed (Channel_constPtr channel) const
      {
          if (channel == NULL) return false;
	  _XXX("RC_SPEW") <<  "World::isSubscribed (" << channel->asString() << ")" << endl;

          if (channel->system ())
      	return false;

          return Subscription::status (channel) ? true : false;
      }



      //---------------------------------------------------------------------------
      // ResItem Locks

      typedef struct {
          ResItem_constPtr resItem;
          World_Ptr world;
          bool is_locked;
      } IsLockedInfo;


      static bool
      is_locked_cb (Match_constPtr match, void *data)
      {
          IsLockedInfo *info = (IsLockedInfo *)data;

          if (match->test (info->resItem, info->world)) {
      	info->is_locked = true;
      	return false;
          }

          return true;
      }


      bool
      World::resItemIsLocked (ResItem_constPtr resItem)
      {
          IsLockedInfo info;

          info.resItem = resItem;
          info.world = this;
          info.is_locked = false;

          foreachLock (is_locked_cb, &info);

          return info.is_locked;
      }


      //---------------------------------------------------------------------------
      // Transacting

      bool
      World::canTransactResItem (ResItem_constPtr resItem)
      {
        if (getenv("FIXME"))      fprintf (stderr, "World::canTransactResItem() not implemented\n");
        return false;
      }

      bool
      World::transact (const ResItemList & installResItems, const ResItemList & remove_resItems, int flags)
      {
        if (getenv("FIXME"))      fprintf (stderr, "World::transact() not implemented\n");
        return false;
      }


      //---------------------------------------------------------------------------
      // XML serialization

      void
      World::serialize (XmlNode_Ptr parent)
      {
        if (getenv("FIXME"))      fprintf (stderr, "World::serialize() not implemented\n");
        return;
      }

      void
      World::toFile (const char *filename)
      {
        if (getenv("FIXME"))      fprintf (stderr, "World::toFile() not implemented\n");
        return;
      }


      //---------------------------------------------------------------------------
      // Duplicating (primarily for atomic refreshes)

      World_Ptr
      World::dup (void)
      {
        if (getenv("FIXME"))      fprintf (stderr, "World::dup() not implemented\n");
        return 0;
      }


      //---------------------------------------------------------------------------
      // only used for bindings

      void
      World::setRefreshFunction (WorldRefreshFn refresh_fn)
      {
        if (getenv("FIXME"))      fprintf (stderr, "World::setRefreshFunction() not implemented\n");
        return;
      }



      //-----------------------------------------------------------------------------
      // Upgrades

      typedef struct  {
          ResItem_constPtr original_resItem;
          CResItemFn fn;
          void *data;
          int count;
          World_Ptr world;
      } ForeachUpgradeInfo;

      static bool
      foreach_upgrade_cb (ResItem_constPtr resItem, void *data)
      {
          ForeachUpgradeInfo *info = (ForeachUpgradeInfo *)data;
          int cmp;

          cmp = ResItem::compare (info->original_resItem, resItem);

          if (cmp >= 0)				// original is already better
      	return true;

          if (info->world->resItemIsLocked (resItem))
      	return true;

          if (info->fn)
      	info->fn (resItem, info->data);
          ++info->count;

          return true;
      }


      // rc_world_foreach_upgrade:
      // @world: An #RCWorld.
      // @resItem: An #RCResItem.
      // @channel: An #RCChannel or channel wildcard.
      // @fn: A callback function.
      // @user_data: Pointer passed to the callback function.
      //
      // Searchs @world for all resItems whose channel matches
      // @channel and that are an upgrade for @resItem.
      // (To be precise, an upgrade is a resItem with the same
      // name as @resItem but with a greater version number.)
      //
      // Return value: The number of matching resItems
      // that the callback functions was invoked on, or
      // -1 in the case of an error.

      int
      World::foreachUpgrade (ResItem_constPtr resItem, Channel_Ptr channel, CResItemFn fn, void *data)
      {
          ForeachUpgradeInfo info;

          syncConditional (channel);

          info.original_resItem = resItem;
          info.fn = fn;
          info.data = data;
          info.count = 0;
          info.world = this;

          foreachResItemByName (resItem->name(), channel, foreach_upgrade_cb, (void *)&info);

          return info.count;
      }



      typedef struct {
          World_Ptr world;
          ResItem_constPtr system_resItem;
          CResItemList best_upgrades;
          bool subscribed_only;
          ResItemPairFn fn;
          void *data;
          int count;
      } SystemUpgradeInfo;


      static bool
      foreach_system_upgrade_cb (ResItem_constPtr upgrade, void *data)
      {
          SystemUpgradeInfo *info = (SystemUpgradeInfo *)data;
          Channel_constPtr channel = upgrade->channel();
          int cmp;

          if (info->subscribed_only) {
      	if (!(channel && channel->isSubscribed ()))
      	    return true;
          }

          if (info->world->resItemIsLocked (upgrade))
      	return true;

          if (info->best_upgrades.empty()) {
      	info->best_upgrades.push_back (upgrade);
          }
          else {
      	/* All the versions are equal, so picking the first is fine */
      	ResItem_constPtr best_up = info->best_upgrades.front();

      	cmp = ResItem::compare (best_up, upgrade);

      	if (cmp <= 0) {
      	    /* We have a new best resItem... */
      	    info->best_upgrades.pop_front();
      	    info->best_upgrades.push_back (upgrade);
      	}
          }

          return true;
      }


      static void
      foreach_system_resItem_cb (const string & name, ResItem_constPtr resItem, SystemUpgradeInfo *info)
      {
          info->system_resItem = resItem;
          info->best_upgrades.clear();

          /* If the resItem is excluded, skip it. */
          if (info->world->resItemIsLocked (info->system_resItem))
      	return;

          info->world->foreachUpgrade (info->system_resItem, new Channel (CHANNEL_TYPE_NONSYSTEM), foreach_system_upgrade_cb, info);

          for (CResItemList::const_iterator iter = info->best_upgrades.begin(); iter != info->best_upgrades.end(); iter++) {
      	ResItem_constPtr upgrade = *iter;

      	if (info->fn)
      	    info->fn (info->system_resItem, upgrade, info->data);

      	++info->count;
          }

          info->best_upgrades.clear();
      }

      typedef map<const string,ResItem_constPtr> UniqueTable;

      static bool
      build_unique_table_cb (ResItem_constPtr resItem, void *data)
      {
          UniqueTable *unique_table = (UniqueTable *)data;

          UniqueTable::const_iterator pos = unique_table->find (resItem->name());

          if (pos != unique_table->end()) {
      	if (ResItem::compare (resItem, pos->second) <= 0)
      	    return true;
          }

          (*unique_table)[resItem->name()] = resItem;

          return true;
      }


      /**
       * foreachSystemUpgrade:
       * @world: An #RCWorld.
       * @subscribed_only: if TRUE, only subscribed channels are used.
       * @fn: A callback function.
       * @user_data: Pointer to be passed to the callback function.
       *
       * Iterates across all system resItems in @world for which there
       * exists an upgrade, and passes both the original resItem and
       * the upgrade resItem to the callback function.
       *
       * Return value: The number of matching resItems that the callback
       * function was invoked on, or -1 in case of an error.
       **/

      int
      World::foreachSystemUpgrade (bool subscribed_only, ResItemPairFn fn, void *data)
      {
          SystemUpgradeInfo info;
          UniqueTable unique_table;

          /* rc_world_foreach_resItem calls rc_world_sync */

          foreachResItem (new Channel (CHANNEL_TYPE_SYSTEM), build_unique_table_cb, &unique_table);

          info.world = this;
          info.subscribed_only = subscribed_only;
          info.fn = fn;
          info.data = data;
          info.count = 0;

          for (UniqueTable::const_iterator iter = unique_table.begin(); iter != unique_table.end(); iter++) {
      	foreach_system_resItem_cb (iter->first, iter->second, &info);
          }

          return info.count;
      }


      PackageUpdateList
      World::getUpgrades (ResItem_constPtr resItem, Channel_constPtr channel)
      {
          fprintf (stderr, "World::getUpgrades not implemented\n");
        return PackageUpdateList();
      }

      ResItem_constPtr
      World::getBestUpgrade (ResItem_constPtr resItem, bool subscribed_only)
      {
          fprintf (stderr, "World::getBestUpgrade not implemented\n");
        return 0;
      }


      //-----------------------------------------------------------------------------
      // Locks

      int
      World::foreachLock (MatchFn fn, void *data) const
      {
          int count = 0;

          for (MatchList::const_iterator iter = _locks.begin(); iter != _locks.end(); iter++) {
              if (! fn (*iter, data))
                  return -1;
              ++count;
          }

          return count;
      }


      void
      World::addLock (Match_constPtr lock)
      {
          _locks.push_back (lock);
      }


      void
      World::removeLock (Match_constPtr lock)
      {
          for (MatchList::iterator iter = _locks.begin(); iter != _locks.end(); iter++) {
      	if (*iter == lock) {
      	    _locks.erase (iter);
      	    break;
      	}
          }
      }


      void
      World::clearLocks (void)
      {
          _locks.clear();
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

