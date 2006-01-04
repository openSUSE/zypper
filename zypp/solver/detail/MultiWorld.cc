/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* MultiWorld.cc
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

#include <zypp/base/String.h>
#include <zypp/solver/detail/MultiWorld.h>
#include <zypp/solver/detail/ServiceWorld.h>

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

      //===========================================================================

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : SubWorldInfo

      class SubWorldInfo {

        private:
          World_Ptr _subworld;
          World_Ptr _refreshed_subworld;

          bool _refreshed_ready;

          unsigned int _changed_resItems_id;
          unsigned int _changed_channels_id;
          unsigned int _changed_subscriptions_id;
          unsigned int _changed_locks_id;

        public:

          SubWorldInfo (World_Ptr subworld, MultiWorld_Ptr multiworld);
          virtual ~SubWorldInfo();

          // ---------------------------------- I/O

          static std::string toString (const SubWorldInfo & subworldinfo);

          virtual std::ostream & dumpOn(std::ostream & str ) const;

          friend std::ostream& operator<<(std::ostream&, const SubWorldInfo & subworldinfo);

          std::string asString (void ) const;

          // ---------------------------------- accessors

          World_Ptr subworld () const { return _subworld; }

          // ---------------------------------- methods

      };

      //---------------------------------------------------------------------------

      string
      SubWorldInfo::asString ( void ) const
      {
          return toString (*this);
      }


      string
      SubWorldInfo::toString ( const SubWorldInfo & subworldinfo )
      {
          return "<subworldinfo/>";
      }


      ostream &
      SubWorldInfo::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }


      ostream&
      operator<<( ostream& os, const SubWorldInfo & subworldinfo)
      {
          return os << subworldinfo.asString();
      }

      //---------------------------------------------------------------------------

      SubWorldInfo::SubWorldInfo (World_Ptr subworld, MultiWorld_Ptr multiworld)
          : _subworld (subworld)
          , _changed_resItems_id (0)
          , _changed_channels_id (0)
          , _changed_subscriptions_id (0)
          , _changed_locks_id (0)

      {
      #if 0
          _changed_resItems_id =
              g_signal_connect (G_OBJECT (subworld),
                                "changed_resItems",
                                (GCallback) changed_resItems_cb,
                                world);

          _changed_channels_id =
              g_signal_connect (G_OBJECT (subworld),
                                "changed_channels",
                                (GCallback) changed_channels_cb,
                                world);

          _changed_subscriptions_id =
              g_signal_connect (G_OBJECT (subworld),
                                "changed_subscriptions",
                                (GCallback) changed_subscriptions_cb,
                                world);

          _changed_locks_id =
              g_signal_connect (G_OBJECT (subworld),
                                "changed_locks",
                                (GCallback) changed_locks_cb,
                                world);
      #endif
      }


      SubWorldInfo::~SubWorldInfo()
      {
      }


      //===========================================================================

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : NameConflictInfo

      class NameConflictInfo {

        private:
          int _depth;
          MultiWorld_Ptr _multiworld;
          World_Ptr _subworld;
          const char *_name;

        public:
          NameConflictInfo(int depth, MultiWorld_Ptr multiworld, World_Ptr subworld, const char *name);
          virtual ~NameConflictInfo();

          // ---------------------------------- I/O

          static std::string toString (const NameConflictInfo & nameconflictinfo);

          virtual std::ostream & dumpOn(std::ostream & str ) const;

          friend std::ostream& operator<<(std::ostream&, const NameConflictInfo & nameconflictinfo);

          std::string asString (void ) const;

          // ---------------------------------- accessors

          int depth () const { return _depth; }
          MultiWorld_Ptr multiworld () const { return _multiworld; }
          World_Ptr subworld () const { return _subworld; }
          const char *name () const { return _name; }
          void setName (const char *name) { free((void *)_name); _name = strdup (name); }

          // ---------------------------------- methods

          void incDepth (void) { _depth++; }
      };


      //---------------------------------------------------------------------------

      string
      NameConflictInfo::asString ( void ) const
      {
          return toString (*this);
      }


      string
      NameConflictInfo::toString ( const NameConflictInfo & subworldinfo )
      {
          return "<nameconflictinfo/>";
      }


      ostream &
      NameConflictInfo::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }


      ostream&
      operator<<( ostream& os, const NameConflictInfo & subworldinfo)
      {
          return os << subworldinfo.asString();
      }

      //---------------------------------------------------------------------------

      NameConflictInfo::NameConflictInfo (int depth, MultiWorld_Ptr multiworld, World_Ptr subworld, const char *name)
          : _depth (depth)
          , _multiworld (multiworld)
          , _subworld (subworld)
          , _name (strdup (name))
      {
      }


      NameConflictInfo::~NameConflictInfo()
      {
          free ((void *)_name);
      }


      //===========================================================================

      IMPL_PTR_TYPE(MultiWorld);

      string
      MultiWorld::asString ( void ) const
      {
          return toString (*this);
      }


      string
      MultiWorld::toString ( const MultiWorld & world )
      {
          return "<undumpworld/>";
      }


      ostream &
      MultiWorld::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }


      ostream&
      operator<<( ostream& os, const MultiWorld & world)
      {
          return os << world.asString();
      }

      //---------------------------------------------------------------------------

      MultiWorld::MultiWorld ()
          : World (MULTI_WORLD)
      {
      }


      MultiWorld::~MultiWorld()
      {
      }

      //---------------------------------------------------------------------------

      class ForeachByTypeInfo {
          public:
      	WorldType type;
      	WorldFn callback;
      	NameConflictInfo *name_conflict_info;

      	int count;
      };


      int
      MultiWorld::foreachSubworld (WorldFn callback, void *user_data)
      {
          if (callback == NULL) return -1;

          /* Make a copy of subworlds for case where user callback is
             running main loop and a refresh starts at that time. */

          WorldList copied_subworlds;

          for (SubWorldInfoList::iterator iter = _subworlds.begin(); iter != _subworlds.end(); iter++) {
      	copied_subworlds.push_front ((*iter)->subworld());
          }

          int count = 0;

          for (WorldList::const_iterator iter = copied_subworlds.begin(); iter != copied_subworlds.end(); iter++) {
              if (! callback (*iter, user_data)) {
                  count = -1;
                  break;
              } else
                  ++count;
          }

          return count;
      }


      static bool
      foreach_by_type_cb (World_constPtr subworld, void *user_data)
      {
          ForeachByTypeInfo *info = (ForeachByTypeInfo *)user_data;

          if ((subworld->type() != info->type)
              || info->callback == NULL)
          {
      	return true;
          }

          if (! info->callback (subworld, info->name_conflict_info)) {
      	info->count = -1;
      	return false;
          } else {
      	++info->count;
      	return true;
          }
      }


      int
      MultiWorld::foreachSubworldByType (WorldType type, WorldFn callback, NameConflictInfo *name_conflict_info)
      {
          ForeachByTypeInfo info;

          info.type = type;
          info.callback = callback;
          info.name_conflict_info = name_conflict_info;
          info.count = 0;

          foreachSubworld (foreach_by_type_cb, (void *)(&info));

          return info.count;
      }

      //---------------------------------------------------------------------------
      // subworld

      static bool
      service_name_conflict_cb (World_constPtr world, void *user_data)
      {
          ServiceWorld_constPtr service = dynamic_pointer_cast<const ServiceWorld>(world);
          if (service == NULL) {
      	fprintf (stderr, "OOPS: service_name_conflict_cb: world is no service\n");
      	abort();
          }

          NameConflictInfo *info = (NameConflictInfo *)user_data;
          if (!strcasecmp (service->name(), info->name())) {
      	info->incDepth();
      	ServiceWorld_Ptr infoservice = dynamic_pointer_cast<ServiceWorld>(info->subworld());
      	if (infoservice == NULL) {
      	    fprintf (stderr, "OOPS: service_name_conflict_cb: info->subworld is no service\n");
      	    abort();
      	}
              info->setName (str::form ("%s (%d)", infoservice->name(), info->depth()).c_str());
      	info->multiworld()->foreachSubworldByType (SERVICE_WORLD, service_name_conflict_cb, info);
              return false;
          }

          return true;
      }


      void
      MultiWorld::addSubworld (World_Ptr subworld)
      {
          if (subworld == NULL) return;

          /*
           * If we're adding a service, make sure that the name of the service
           * doesn't conflict with any other.
           */
          ServiceWorld_Ptr service = dynamic_pointer_cast<ServiceWorld>(subworld);			// service will be NULL if subworld is not a ServiceWorld

          if (service != NULL) {
      	NameConflictInfo conflict_info (0, this, subworld, service->name());

      	foreachSubworldByType (SERVICE_WORLD, service_name_conflict_cb, &conflict_info);

      	service->setName (conflict_info.name());
          }

          SubWorldInfo *subworld_info = new SubWorldInfo (subworld, this);

          _subworlds.push_back (subworld_info);

      //    g_signal_emit (multi, signals[SUBWORLD_ADDED], 0, subworld);

          return;
      }


      void
      MultiWorld::removeSubworld (World_Ptr subworld)
      {
          if (subworld == NULL) return;

          for (SubWorldInfoList::iterator iter = _subworlds.begin(); iter != _subworlds.end(); iter++) {
      	if ((*iter)->subworld() == subworld) {
      	    _subworlds.erase (iter);
      //	    g_signal_emit (multi, signals[SUBWORLD_REMOVED], 0, subworld);
      	    return;
      	}
          }
          return;
      }


      //---------------------------------------------------------------------------
      // channels

      ChannelList
      MultiWorld::channels () const
      {
          ChannelList cl;
          for (SubWorldInfoList::const_iterator iter = _subworlds.begin(); iter != _subworlds.end(); iter++) {
      	cl = (*iter)->subworld()->channels();
      //FIXME	cl.merge ((*iter)->subworld()->channels());
          }
          return cl;
      }


      bool
      MultiWorld::containsChannel (Channel_constPtr channel) const
      {
          for (SubWorldInfoList::const_iterator iter = _subworlds.begin(); iter != _subworlds.end(); iter++) {
      	if ((*iter)->subworld()->containsChannel(channel))
      	    return true;
          }
          return false;
      }


      Channel_Ptr
      MultiWorld::getChannelByName (const char *channel_name) const
      {
          Channel_Ptr channel;
          for (SubWorldInfoList::const_iterator iter = _subworlds.begin(); iter != _subworlds.end(); iter++) {
      	channel = (*iter)->subworld()->getChannelByName(channel_name);
      	if (channel != NULL)
      	    return channel;
          }
          return NULL;
      }


      Channel_Ptr
      MultiWorld::getChannelByAlias (const char *alias) const
      {
          Channel_Ptr channel;
          for (SubWorldInfoList::const_iterator iter = _subworlds.begin(); iter != _subworlds.end(); iter++) {
      	channel = (*iter)->subworld()->getChannelByAlias(alias);
      	if (channel != NULL)
      	    return channel;
          }
          return NULL;
      }


      Channel_Ptr
      MultiWorld::getChannelById (const char *channel_id) const
      {
          Channel_Ptr channel;
          for (SubWorldInfoList::const_iterator iter = _subworlds.begin(); iter != _subworlds.end(); iter++) {
      	channel = (*iter)->subworld()->getChannelById(channel_id);
      	if (channel != NULL)
      	    return channel;
          }
          return NULL;
      }


      int
      MultiWorld::foreachChannel (ChannelFn fn, void *data) const
      {
          int count = 0;
          for (SubWorldInfoList::const_iterator iter = _subworlds.begin(); iter != _subworlds.end(); iter++) {
      	int this_count;
      	this_count = (*iter)->subworld()->foreachChannel(fn, data);
      	if (this_count < 0)
      	    return -1;
      	count += this_count;
          }
          return count;
      }


      //---------------------------------------------------------------------------
      // Single resItem queries

      ResItem_constPtr
      MultiWorld::findInstalledResItem (ResItem_constPtr resItem)
      {
          ResItem_constPtr installed;
          for (SubWorldInfoList::const_iterator iter = _subworlds.begin(); iter != _subworlds.end(); iter++) {
      	installed = (*iter)->subworld()->findInstalledResItem(resItem);
      	if (installed != NULL)
      	    return installed;
          }
          return NULL;
      }


      ResItem_constPtr
      MultiWorld::findResItem (Channel_constPtr channel, const char *name) const
      {
          ResItem_constPtr resItem;
          for (SubWorldInfoList::const_iterator iter = _subworlds.begin(); iter != _subworlds.end(); iter++) {
      	resItem = (*iter)->subworld()->findResItem(channel, name);
      	if (resItem != NULL)
      	    return resItem;
          }
          return NULL;
      }


      ResItem_constPtr
      MultiWorld::findResItemWithConstraint (Channel_constPtr channel, const char *name, const Capability & constraint, bool is_and) const
      {
          ResItem_constPtr resItem;
          for (SubWorldInfoList::const_iterator iter = _subworlds.begin(); iter != _subworlds.end(); iter++) {
      	resItem = (*iter)->subworld()->findResItemWithConstraint(channel, name, constraint, is_and);
      	if (resItem != NULL)
      	    return resItem;
          }
          return NULL;
      }


      Channel_Ptr
      MultiWorld::guessResItemChannel (ResItem_constPtr resItem) const
      {
          Channel_Ptr channel;
          for (SubWorldInfoList::const_iterator iter = _subworlds.begin(); iter != _subworlds.end(); iter++) {
      	channel = (*iter)->subworld()->guessResItemChannel(resItem);
      	if (channel != NULL)
      	    return channel;
          }
          return NULL;
      }

      //---------------------------------------------------------------------------
      // iterate over resItems

      int
      MultiWorld::foreachResItem (Channel_Ptr channel, CResItemFn fn, void *data)
      {
          return foreachResItemByName ("", channel, fn, data);
      }


      int
      MultiWorld::foreachResItemByName (const std::string & name, Channel_Ptr channel, CResItemFn fn, void *data)
      {
          int count = 0;
          for (SubWorldInfoList::const_iterator iter = _subworlds.begin(); iter != _subworlds.end(); iter++) {
      	int this_count;
      	this_count = (*iter)->subworld()->foreachResItemByName(name, channel, fn, data);
      	if (this_count < 0)
      	    return -1;
      	count += this_count;
          }
          return count;
      }


      int
      MultiWorld::foreachResItemByMatch (Match_constPtr match, CResItemFn fn, void *data)
      {
          fprintf (stderr, "MultiWorld::foreachResItemByMatch not implemented\n");
          return 0;
      }


      //-----------------------------------------------------------------------------
      // iterater over resItems with dependency

      int
      MultiWorld::foreachProvidingResItem (const Capability & dep, ResItemAndDepFn fn, void *data)
      {
          int count = 0;
          for (SubWorldInfoList::const_iterator iter = _subworlds.begin(); iter != _subworlds.end(); iter++) {
      	int this_count;
      	this_count = (*iter)->subworld()->foreachProvidingResItem (dep, fn, data);
      	if (this_count < 0)
      	    return -1;
      	count += this_count;
          }
          return count;
      }

      int
      MultiWorld::foreachRequiringResItem (const Capability & dep, ResItemAndDepFn fn, void *data)
      {
          int count = 0;
          for (SubWorldInfoList::const_iterator iter = _subworlds.begin(); iter != _subworlds.end(); iter++) {
      	int this_count;
      	this_count = (*iter)->subworld()->foreachRequiringResItem (dep, fn, data);
      	if (this_count < 0)
      	    return -1;
      	count += this_count;
          }
          return count;
      }

      int
      MultiWorld::foreachConflictingResItem (const Capability & dep, ResItemAndDepFn fn, void *data)
      {
          int count = 0;
          for (SubWorldInfoList::const_iterator iter = _subworlds.begin(); iter != _subworlds.end(); iter++) {
      	int this_count;
      	this_count = (*iter)->subworld()->foreachConflictingResItem (dep, fn, data);
      	if (this_count < 0)
      	    return -1;
      	count += this_count;
          }
          return count;
      }


      //-----------------------------------------------------------------------------
      // iterater over resItems with locks

      int
      MultiWorld::foreachLock (MatchFn fn, void *data) const
      {
          int count = 0;
          for (SubWorldInfoList::const_iterator iter = _subworlds.begin(); iter != _subworlds.end(); iter++) {
      	int this_count;
      	this_count = (*iter)->subworld()->foreachLock(fn, data);
      	if (this_count < 0)
      	    return -1;
      	count += this_count;
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

