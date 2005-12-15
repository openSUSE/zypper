/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* QueueItem.cc
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

#include <zypp/solver/detail/QueueItem.h>
#include <zypp/solver/detail/ResolverContext.h>


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
      
      IMPL_BASE_POINTER(QueueItem);
      
      //---------------------------------------------------------------------------
      
      string
      QueueItem::asString ( void ) const
      {
          return toString (*this);
      }
      
      
      string
      QueueItem::toString ( const QueueItem & item )
      {
          return "<queueitem/>";
      }
      
      
      string
      QueueItem::toString ( const QueueItemList & itemlist, const string & sep )
      {
          string res = "[";
          for (QueueItemList::const_iterator iter = itemlist.begin(); iter != itemlist.end(); iter++) {
      	if (iter != itemlist.begin())
      	    res += sep;
      	res += (*iter)->asString();
          }
          return res + "]";
      }
      
      
      ostream &
      QueueItem::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }
      
      
      ostream&
      operator<<( ostream& os, const QueueItem & item )
      {
          return os << item.asString();
      }
      
      //---------------------------------------------------------------------------
      
      QueueItem::QueueItem (QueueItemType type, WorldPtr world)
          : _type (type)
          , _world (world)
          , _priority (0)
          , _size (0)
      {
      }
      
      
      QueueItem::~QueueItem()
      {
      }
      
      //---------------------------------------------------------------------------
      
      void
      QueueItem::copy (constQueueItemPtr from)
      {
          _priority = from->_priority;
          _size = from->_size;
          _pending_info = ResolverInfoList (from->_pending_info.begin(), from->_pending_info.end());
      }
      
      
      //---------------------------------------------------------------------------
      
      void
      QueueItem::addInfo (ResolverInfoPtr info)
      {
          _pending_info.push_back (info);
      }
      
      
      void
      QueueItem::logInfo (ResolverContextPtr context)
      {
          for (ResolverInfoList::const_iterator iter = _pending_info.begin(); iter != _pending_info.end(); iter++) {
      	context->addInfo (*iter);
          }
          _pending_info.clear();
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
