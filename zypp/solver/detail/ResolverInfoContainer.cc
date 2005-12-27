/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ResolverInfoContainer.cc
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

#include <zypp/solver/detail/ResolverInfoContainer.h>

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
      
      IMPL_DERIVED_POINTER(ResolverInfoContainer, ResolverInfo);
      
      //---------------------------------------------------------------------------
      
      
      string
      ResolverInfoContainer::asString ( void ) const
      {
          return toString (*this);
      }
      
      
      string
      ResolverInfoContainer::toString ( const ResolverInfoContainer & container )
      {
          string res = "<resolverinfocontainer '";
      
          res += ResolverInfo::toString (container);
          for (CResItemList::const_iterator iter = container._resItem_list.begin(); iter != container._resItem_list.end(); iter++) {
      	if (iter != container._resItem_list.begin()) res += ", ";
      	res += (*iter)->asString();
          }
          res += "'>";
      
          return res;
      }
      
      
      ostream &
      ResolverInfoContainer::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }
      
      
      ostream&
      operator<<( ostream& os, const ResolverInfoContainer & container)
      {
          return os << container.asString();
      }
      
      //---------------------------------------------------------------------------
      
      ResolverInfoContainer::ResolverInfoContainer (ResolverInfoType type, constResItemPtr resItem, int priority, constResItemPtr child)
          : ResolverInfo (type, resItem, priority)
      {
          if (child != NULL)
      	_resItem_list.push_back (child);
      }
      
      
      ResolverInfoContainer::~ResolverInfoContainer ()
      {
      }
      
      //---------------------------------------------------------------------------
      
      bool
      ResolverInfoContainer::merge (ResolverInfoContainerPtr to_be_merged)
      {
          bool res;
      
          res = ((ResolverInfoPtr)this)->merge ((ResolverInfoPtr)to_be_merged);
          if (!res) return res;
      
          typedef std::map<constResItemPtr, bool> SeenTable;
          SeenTable seen_packages;
      
          for (CResItemList::const_iterator iter = _resItem_list.begin(); iter != _resItem_list.end(); iter++) {
      	seen_packages[*iter] = true;
          }
      
          CResItemList rl = to_be_merged->resItems();
          for (CResItemList::const_iterator iter = rl.begin(); iter != rl.end(); iter++) {
      	SeenTable::const_iterator pos = seen_packages.find(*iter);
      	if (pos == seen_packages.end()) {
      	    _resItem_list.push_front (*iter);
      	    seen_packages[*iter] = true;
      	}
          }
      
          return true;
      }
      
      
      void
      ResolverInfoContainer::copy (constResolverInfoContainerPtr from)
      {
          ((ResolverInfoPtr)this)->copy(from);
      
          for (CResItemList::const_iterator iter = from->_resItem_list.begin(); iter != from->_resItem_list.end(); iter++) {
      	_resItem_list.push_back (*iter);
          }
      }
      
      
      ResolverInfoPtr
      ResolverInfoContainer::copy (void) const
      {
          ResolverInfoContainerPtr cpy = new ResolverInfoContainer(type(), resItem(), priority());
      
          cpy->copy (this);
      
          return cpy;
      }
      
      //---------------------------------------------------------------------------
      
      string
      ResolverInfoContainer::resItemsToString (bool names_only) const
      {
          string res;
      
          if (_resItem_list.empty())
      	return res;
      
          res += " [";
          for (CResItemList::const_iterator iter = _resItem_list.begin(); iter != _resItem_list.end(); iter++) {
      	if (iter != _resItem_list.begin())
      	    res += ", ";
      
      	res += (names_only ? (*iter)->name() : (*iter)->asString());
          }
          res += "]";
      
          return res;
      }
      
      
      bool
      ResolverInfoContainer::mentions (constResItemPtr resItem) const
      {
          if (isAbout(resItem))
      	return true;
      
          // Search resItem_list for any mention of the resItem.
      
          for (CResItemList::const_iterator iter = _resItem_list.begin(); iter != _resItem_list.end(); iter++) {
      	if ((*iter)->name() == resItem->name()) {
      	    return true;
      	}
          }
          
          return false;
      }
      
      
      void
      ResolverInfoContainer::addRelatedResItem (constResItemPtr resItem)
      {
          if (!mentions(resItem)) {
      	_resItem_list.push_front (resItem);
          }
      }
      
      
      void
      ResolverInfoContainer::addRelatedResItemList (const CResItemList & resItems)
      {
          for (CResItemList::const_iterator iter = resItems.begin(); iter != resItems.end(); iter++) {
      	_resItem_list.push_front (*iter);
          }
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

