/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* StoreWorld.cc
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

#include "config.h"

#include <y2util/stringutil.h>

#include <zypp/solver/detail/Packman.h>
#include <zypp/solver/detail/StoreWorld.h>
#include <zypp/solver/detail/ResItemAndDependency.h>
#include <zypp/solver/detail/debug.h>
#include <zypp/Arch.h>
#include <zypp/CapSet.h>

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
      
      IMPL_DERIVED_POINTER(StoreWorld, World);
      
      //---------------------------------------------------------------------------
      
      string
      StoreWorld::asString ( void ) const
      {
          return toString (*this);
      }
      
      
      string
      StoreWorld::toString ( const StoreWorld & storeworld )
      {
          string res ("<storeworld/>");
      
          return res;
      }
      
      
      ostream &
      StoreWorld::dumpOn (ostream & str) const
      {
          str << asString();
          return str;
      }
      
      
      ostream &
      operator<< (ostream & os, const StoreWorld & storeworld)
      {
          return os << storeworld.asString();
      }
      
      //---------------------------------------------------------------------------
      
      StoreWorld::StoreWorld (WorldType type)
          : World (type)
      {
      }
      
      
      StoreWorld::~StoreWorld()
      {
          fprintf (stderr, "*** deleting store world[%p]: %s\n", this, World::toString(type()).c_str());
      }
      
      
      //---------------------------------------------------------------------------
      
      // Add/remove resItems
      
      bool
      StoreWorld::addResItem (constResItemPtr resItem)
      {
          ResItemAndDependencyPtr r_and_d;
          const char *package_name;
          constChannelPtr channel;
          bool actually_added_package = false;
      
          if (resItem == NULL) return false;
      
          channel = resItem->channel ();
      
//          fprintf (stderr, "StoreWorld[%p]::addResItem(%s) [%s]\n", this, ((constSpecPtr)resItem)->asString().c_str(), channel?channel->name():"?");
      
      
          /* Before we do anything, check to make sure that a resItem of the
             same name isn't already in that channel.  If there is a
             duplicate, we keep the one with the most recent version number
             and drop the other.
      
             This check only applies to resItems in a channel.  We have
             to allow for multiple installs.  Grrrr...
          */
      
          if (!resItem->isInstalled ()) {			// its not a system package
      
      	constResItemPtr dup_res;
      
      	/* Filter out resItems with totally incompatible arches */
      	if ( !resItem->arch().compatibleWith(Arch::System)) {
      	    rc_debug (RC_DEBUG_LEVEL_DEBUG, "Ignoring resItem with incompatible arch: Arch '%s', %s",  resItem->arch().asString().c_str(), resItem->asString(true).c_str());
      	    goto finished;
      	}
      
      	package_name = resItem->name().c_str();
      	dup_res = findResItem (channel, package_name);
      
      	/* This shouldn't happen (and would be caught by the check
      	   below, because cmp will equal 0), but it never hurts to
      	   check and produce a more explicit warning message. */
      
      	if (resItem == dup_res) {
      	    rc_debug (RC_DEBUG_LEVEL_WARNING, "Ignoring re-add of resItem '%s'", package_name);
      	    goto finished;
      	}
      
      	if (dup_res != NULL) {
      	    int cmp;
      
      	    cmp = ResItem::compare (resItem, dup_res);
      //fprintf (stderr, "res: %s, dup_res %s, cmp %d\n", resItem->asString().c_str(), dup_res->asString().c_str(), cmp);
      
      	    /* If the resItem we are trying to add has a lower 
      	       version number, just ignore it. */
      
      	    if (cmp < 0) {
      		rc_debug (RC_DEBUG_LEVEL_INFO, "Not adding resItem '%s'.\n\tA newer version is already in the channel.", resItem->asString().c_str());
      		rc_debug (RC_DEBUG_LEVEL_INFO, "\t%s", dup_res->asString().c_str());
      		goto finished;
      	    }
      
#if 0
      	    /* If the version numbers are equal, we ignore the resItem to
      	       add if it has a less-preferable arch.  If both
      	       resItems have the same version # and arch, we favor the
      	       first resItem and just return. */
      
      	    if (cmp == 0 && arch_score > dup_arch_score) {
      		rc_debug (RC_DEBUG_LEVEL_INFO, "Not adding resItem '%s'.\n\tAnother resItem with the same version but with a preferred arch is already in the channel.", resItem->asString().c_str());
      		rc_debug (RC_DEBUG_LEVEL_INFO, "\t%s", dup_res->asString().c_str());
      		goto finished;
      	    }
#endif
      
      	    /* Otherwise we throw out the old resItem and proceed with
      	       adding the newer one. */
      
      	    rc_debug (RC_DEBUG_LEVEL_INFO, "Replacing resItem '%s'.\n\tAnother resItem in the channel has the same name and a superior %s.",  dup_res->asString().c_str(), cmp ? "version" : "arch");
      	    rc_debug (RC_DEBUG_LEVEL_INFO, "\t%s", resItem->asString().c_str());
      
      	    removeResItem (dup_res);
      	}
          }
      
          actually_added_package = true;
      
          if (channel && !channel->hidden()) {
      	touchResItemSequenceNumber ();
          }
      
          /* StoreWorld all of our resItems in a hash by name. */
          _resItems_by_name.insert (ResItemTable::value_type (resItem->name(), resItem));
      
          /* StoreWorld all of the resItem's provides in a hash by name. */
          for (CapSet::const_iterator i = resItem->provides().begin(); i != resItem->provides().end(); i++) {
      	r_and_d = new ResItemAndDependency (resItem, *i);
      
      	_provides_by_name.insert (ResItemAndDependencyTable::value_type (r_and_d->dependency().name(), r_and_d));
          }
      
          /* StoreWorld all of the resItem's requires in a hash by name. */
      
          for (CapSet::const_iterator i = resItem->requires().begin(); i != resItem->requires().end(); i++) {
      	r_and_d = new ResItemAndDependency (resItem, *i);
      
      	_requires_by_name.insert (ResItemAndDependencyTable::value_type (r_and_d->dependency().name(), r_and_d));
          }
      
          /* "Recommends" are treated as requirements. */
      #warning Recommends are treated as requirements
      
          for (CapSet::const_iterator i = resItem->recommends().begin(); i != resItem->recommends().end(); i++) {
      	r_and_d = new ResItemAndDependency (resItem, *i);
      
      	_requires_by_name.insert (ResItemAndDependencyTable::value_type (r_and_d->dependency().name(), r_and_d));
          }
      
          /* StoreWorld all of the resItem's conflicts in a hash by name. */
      
          for (CapSet::const_iterator i = resItem->conflicts().begin(); i != resItem->conflicts().end(); i++) {
      	r_and_d = new ResItemAndDependency (resItem, *i);
      	_conflicts_by_name.insert (ResItemAndDependencyTable::value_type (r_and_d->dependency().name(), r_and_d));
          }
          
       finished:
      
          return actually_added_package;
      }
      
      
      void
      StoreWorld::addResItemsFromList (const CResItemList & slist)
      {
          for (CResItemList::const_iterator i = slist.begin(); i != slist.end(); i++) {
      	if (!addResItem (*i)) {
      	    fprintf (stderr, "addResItem failed\n");
      	    break;
      	}
          }
          return;
      }
      
      //---------------------------------------------------------------------------
      
      static void
      resItem_table_remove (ResItemTable & table, constResItemPtr resItem)
      {
          const string name = resItem->name();
          for (ResItemTable::iterator pos = table.lower_bound(name); pos != table.upper_bound(name); pos++) {
      	constResItemPtr res = pos->second;
      	if (res == resItem) {
      	    table.erase (pos);
      	    break;
      	}
          }
          return;
      }
      
      
      static void
      resItem_and_dependency_table_remove (ResItemAndDependencyTable & table, constResItemPtr resItem)
      {
          const string name = resItem->name();
      // FIXME: this is inefficient but lower_bound can't to strcasecmp :-(
      //    for (ResItemAndDependencyTable::iterator pos = table.lower_bound(name); pos != table.upper_bound(name); pos++) {
          for (ResItemAndDependencyTable::iterator pos = table.begin(); pos != table.end(); pos++) {
      	constResItemAndDependencyPtr r_and_d = pos->second;
      	if (r_and_d->resItem() == resItem) {
      	    table.erase (pos);
      	    break;
      	}
          }
          return;
      }
      
      void
      StoreWorld::removeResItem (constResItemPtr resItem)
      {
          if (getenv("RC_SPEW")) fprintf (stderr, "StoreWorld::removeResItem (%s)\n", resItem->asString().c_str());
      
          constChannelPtr channel = resItem->channel ();
      
          if (! (channel && channel->hidden ()))
      	touchResItemSequenceNumber ();
      
          resItem_and_dependency_table_remove (_provides_by_name, resItem);
          resItem_and_dependency_table_remove (_requires_by_name, resItem);
          resItem_and_dependency_table_remove (_conflicts_by_name, resItem);
      
          resItem_table_remove (_resItems_by_name, resItem);
      
          return;
      }
      
      
      void
      StoreWorld::removeResItems (constChannelPtr channel)
      {
      	fprintf (stderr, "StoreWorld::removeResItems(%s) not implemented\n", channel->asString().c_str());
      }
      
      
      void
      StoreWorld::clear ()
      {
      	fprintf (stderr, "StoreWorld::clear() not implemented\n");
      }
      
      //---------------------------------------------------------------------------
      // Single resItem queries
      
      static bool
      installed_version_cb (constResItemPtr resItem, void *data)
      {
          constResItemPtr *installed = (constResItemPtr *)data;
      
          if (resItem->isInstalled ()) {
      	*installed = resItem;
      	return false;
          }
          return true;
      }
      
      
      constResItemPtr
      StoreWorld::findInstalledResItem (constResItemPtr resItem)
      {
          constResItemPtr installed;
          sync ();
      
          foreachResItemByName (resItem->name(), new Channel(CHANNEL_TYPE_ANY) /* is this right? */, installed_version_cb, &installed);
      
          return installed;
      }
      
      
      //
      // findResItem 
      // @channel: A non-wildcard #Channel.
      // @name: The name of a resItem.
      //
      // Searches the world for a resItem in the specified channel
      // with the specified name.  @channel must be an actual
      // channel, not a wildcard.
      //
      // Return value: The matching resItem, or %NULL if no such
      // resItem exists.
      //
      
      constResItemPtr
      StoreWorld::findResItem (constChannelPtr channel, const char *name) const
      {
          syncConditional (channel);
          for (ResItemTable::const_iterator pos = _resItems_by_name.lower_bound(name); pos != _resItems_by_name.upper_bound(name); pos++) {
      	constResItemPtr res = pos->second;
      	if (res->channel() == channel) {
      	    return res;
      	}
          }
          return NULL;
      }
      
      
      constResItemPtr
      StoreWorld::findResItemWithConstraint (constChannelPtr channel, const char *name, const Capability & constraint, bool is_and) const
      {
      	fprintf (stderr, "StoreWorld::findResItemWithConstraint() not implemented\n");
          return 0;
      }
      
      
      ChannelPtr
      StoreWorld::guessResItemChannel (constResItemPtr resItem) const
      {
      	fprintf (stderr, "StoreWorld::guessResItemChannel(%s) not implemented\n", resItem->asString().c_str());
          return 0;
      }
      
      
      //-----------------------------------------------------------------------------
      // foreach resItem
      
      typedef struct {
          ChannelPtr channel;
          CResItemFn callback;
          void *data;
          int count;
          bool short_circuit;
      } ForeachResItemInfo;
      
      
      static void
      foreach_resItem_cb (const string &name, constResItemPtr resItem, void *data)
      {
          ForeachResItemInfo *info = (ForeachResItemInfo *)data;
      
          if (info->short_circuit)
      	return;
      
          /* FIXME: we should filter out dup uninstalled resItems. */
      
          if (resItem && info->channel->equals(resItem->channel ())) {
      	if (info->callback) {
      	    if (! info->callback (resItem, info->data))
      		info->short_circuit = true;
      	}
      	++info->count;
          }
      }
      
      
      int
      StoreWorld::foreachResItem (ChannelPtr channel, CResItemFn fn, void *data)
      {
          return foreachResItemByName ("", channel, fn, data);
      }
      
      
      int
      StoreWorld::foreachResItemByName (const std::string & name, ChannelPtr channel, CResItemFn fn, void *data)
      {
          if (name.empty()) {
      
      	ForeachResItemInfo info;
      
      	info.channel = channel;
      	info.callback = fn;
      	info.data = data;
      	info.count = 0;
      	info.short_circuit = false;
      
      	for (ResItemTable::const_iterator iter = _resItems_by_name.begin(); iter != _resItems_by_name.end(); iter++) {
      	    foreach_resItem_cb (iter->first, iter->second, (void *)&info);
      	}
      
      	return info.short_circuit ? -1 : info.count;
          }
      
      	
          ResItemTable installed;			// FIXME: <Spec, resItem> rc_resItem_spec_equal
          int count = 0;
      
          for (ResItemTable::const_iterator iter = _resItems_by_name.lower_bound(name); iter != _resItems_by_name.upper_bound(name); iter++) {
      	constResItemPtr resItem = iter->second;
      	if (resItem->isInstalled()) {
      	    const string str = resItem->asString();
      	    installed.insert (ResItemTable::value_type(str,resItem));
      	}
          }    
      
          for (ResItemTable::const_iterator iter = _resItems_by_name.lower_bound(name); iter != _resItems_by_name.upper_bound(name); iter++) {
      	constResItemPtr resItem = iter->second;
      	if (channel->equals (resItem->channel())) {
      	    if (resItem->isInstalled()
      		|| installed.find(resItem->asString()) == installed.end()) {
      		if (fn) {
      		    if (! fn(resItem, data)) {
      			count = -1;
      			goto finished;
      		    }
      		}
      		++count;
      	    }
      	}
          }
      
      finished:
      
          return count;
      }
      
      
      int
      StoreWorld::foreachResItemByMatch (constMatchPtr match, CResItemFn fn, void *data)
      {
      	fprintf (stderr, "StoreWorld::foreachResItemByMatch () not implemented\n");
          return 0;
      }
      
      
      //-----------------------------------------------------------------------------
      // iterater over resItems with dependency
      
      typedef std::map<constResItemPtr, constResItemAndDependencyPtr> InstalledTable;
      
      int
      StoreWorld::foreachProvidingResItem (const Capability & dep, ResItemAndDepFn fn, void *data)
      {
          int count = 0;
          InstalledTable installed;
      //fprintf (stderr, "StoreWorld::foreachProvidingResItem(%s)\n", dep->asString().c_str());
          for (ResItemAndDependencyTable::const_iterator iter = _provides_by_name.lower_bound(dep.name()); iter != _provides_by_name.upper_bound(dep.name()); iter++) {
      	constResItemAndDependencyPtr r_and_d = iter->second;
      	constResItemPtr res = r_and_d->resItem();
      //fprintf (stderr, "StoreWorld::foreachProvidingResItem(): %s\n", res->asString(true).c_str());
      	if (res != NULL && res->isInstalled ()) {
      	    installed[res] = r_and_d;
      	}
          }
      
          for (ResItemAndDependencyTable::const_iterator iter = _provides_by_name.lower_bound(dep.name()); iter != _provides_by_name.upper_bound(dep.name()); iter++) {
      	constResItemAndDependencyPtr r_and_d = iter->second;
      
      	if (r_and_d && r_and_d->verifyRelation (dep)) {
      //fprintf (stderr, "found: %s\n", r_and_d->resItem()->asString(true).c_str());
      	    /* If we have multiple identical resItems in RCWorld,
      	       we want to only include the resItem that is installed and
      	       skip the rest. */
      	    if (r_and_d->resItem()->isInstalled()
      		|| installed.find(r_and_d->resItem()) == installed.end()) {
      
      		if (fn) {
      		    if (! fn(r_and_d->resItem(), r_and_d->dependency(), data)) {
      			count = -1;
      			goto finished;
      		    }
      		}
      		++count;
      	    }
      	}
          }
      
       finished:
      
          return count;
      }
      
      int
      StoreWorld::foreachRequiringResItem (const Capability & dep, ResItemAndDepFn fn, void *data)
      {
          int count = 0;
          InstalledTable installed;
      
      
          for (ResItemAndDependencyTable::const_iterator iter = _requires_by_name.lower_bound(dep.name()); iter != _requires_by_name.upper_bound(dep.name()); iter++) {
      	constResItemAndDependencyPtr r_and_d = iter->second;
      	constResItemPtr res = r_and_d->resItem();
      	if (res != NULL && res->isInstalled ()) {
      //fprintf (stderr, "is installed: %s\n", res->asString(true).c_str());
      	    installed[res] = r_and_d;
      	}
          }
      
          for (ResItemAndDependencyTable::const_iterator iter = _requires_by_name.lower_bound(dep.name()); iter != _requires_by_name.upper_bound(dep.name()); iter++) {
      	constResItemAndDependencyPtr r_and_d = iter->second;
      
      	if (r_and_d && r_and_d->dependency().matches (dep)) {
      
      	    /* Skip dups if one of them in installed. */
      	    if (r_and_d->resItem()->isInstalled()
      		|| installed.find(r_and_d->resItem()) == installed.end()) {
      
      		if (fn) {
      		    if (! fn(r_and_d->resItem(), r_and_d->dependency(), data)) {
      			count = -1;
      			goto finished;
      		    }
      		}
      		++count;
      	    }
      	}
          }
      
       finished:
      
          return count;
      }
      
      
      int
      StoreWorld::foreachConflictingResItem (const Capability & dep, ResItemAndDepFn fn, void *data)
      {
          int count = 0;
          InstalledTable installed;
      //fprintf (stderr, "StoreWorld::foreachConflictingResItem (%s)\n", dep->name().c_str());
          for (ResItemAndDependencyTable::const_iterator iter = _conflicts_by_name.lower_bound(dep.name()); iter != _conflicts_by_name.upper_bound(dep.name()); iter++) {
      	constResItemAndDependencyPtr r_and_d = iter->second;
      	constResItemPtr res = r_and_d->resItem();
      //fprintf (stderr, "==> %s\n", res->asString().c_str());
      	if (res != NULL && res->isInstalled ()) {
      	    installed[res] = r_and_d;
      	}
          }
      
          for (ResItemAndDependencyTable::const_iterator iter = _conflicts_by_name.lower_bound(dep.name()); iter != _conflicts_by_name.upper_bound(dep.name()); iter++) {
      	constResItemAndDependencyPtr r_and_d = iter->second;
      
      	if (r_and_d)
      //fprintf (stderr, "==> %s verify %s ? %s\n", r_and_d->asString().c_str(), dep->asString().c_str(), r_and_d->verifyRelation (dep) ? "Y" : "N");
      	if (r_and_d && r_and_d->verifyRelation (dep)) {
      
      	    /* Skip dups if one of them in installed. */
      	    if (r_and_d->resItem()->isInstalled()
      		|| installed.find(r_and_d->resItem()) == installed.end()) {
      
      		if (fn) {
      		    if (! fn(r_and_d->resItem(), r_and_d->dependency(), data)) {
      			count = -1;
      			goto finished;
      		    }
      		}
      		++count;
      	    }
      	}
          }
      
       finished:
      
          return count;
      }
      
      //-----------------------------------------------------------------------------
      // channel functions
      
      void
      StoreWorld::addChannel (ChannelPtr channel)
      {
          if (channel == NULL) return;
      
          channel->setWorld (this);
          channel->setImmutable (true);
      
          _channels.push_back (channel);
      
          touchChannelSequenceNumber ();
      }
      
      
      void
      StoreWorld::removeChannel (constChannelPtr channel)
      {
          if (channel == NULL
      	|| ! containsChannel (channel))
      	return;
      
          removeResItems (channel);
      
          for (ChannelList::iterator iter = _channels.begin(); iter != _channels.end(); iter++) {
      	if ((*iter)->equals (channel)) {
      	    _channels.erase (iter);
      	    touchChannelSequenceNumber ();
      	    break;
      	}
          }
      }
      
      
      bool
      StoreWorld::containsChannel (constChannelPtr channel) const
      {
          for (ChannelList::const_iterator iter = _channels.begin(); iter != _channels.end(); iter++) {
      	if ((*iter)->equals (channel)) {
      	    return true;
      	}
          }
          return false;
      }
      
      
      ChannelPtr
      StoreWorld::getChannelByName (const char *channel_name) const
      {
          if (channel_name == NULL
      	|| *channel_name == 0) {
      	return NULL;
          }
      
          for (ChannelList::const_iterator iter = _channels.begin(); iter != _channels.end(); iter++) {
      	if (strcasecmp ((*iter)->name(), channel_name) == 0) {
      	    return *iter;
      	}
          }
          return NULL;
      }
      
      
      ChannelPtr
      StoreWorld::getChannelByAlias (const char *alias) const
      {
          if (alias == NULL
      	|| *alias == 0) {
      	return NULL;
          }
      
          for (ChannelList::const_iterator iter = _channels.begin(); iter != _channels.end(); iter++) {
      	if (strcasecmp ((*iter)->alias(), alias) == 0) {
      	    return *iter;
      	}
          }
          return NULL;
      }
      
      
      ChannelPtr
      StoreWorld::getChannelById (const char *channel_id) const
      {
          if (channel_id == NULL
      	|| *channel_id  == 0) {
      	return NULL;
          }
      
          for (ChannelList::const_iterator iter = _channels.begin(); iter != _channels.end(); iter++) {
      	if (strcasecmp ((*iter)->id(), channel_id) == 0) {
      	    return *iter;
      	}
          }
          return NULL;
      }
      
      
      int
      StoreWorld::foreachChannel (ChannelFn fn, void *data) const
      {
          int count = 0;
          for (ChannelList::const_iterator iter = _channels.begin(); iter != _channels.end(); iter++) {
      	if (!(*fn) (*iter, data))
      	    return -1;
      	count++;
          }
          return count;
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

