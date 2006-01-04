/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverInfoConflictsWith.cc
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

#include <zypp/solver/detail/ResolverInfo.h>
#include <zypp/solver/detail/ResolverInfoConflictsWith.h>

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
      
      IMPL_PTR_TYPE(ResolverInfoConflictsWith);
      
      //---------------------------------------------------------------------------
      
      
      string
      ResolverInfoConflictsWith::asString ( void ) const
      {
          return toString (*this);
      }
      
      
      string
      ResolverInfoConflictsWith::toString ( const ResolverInfoConflictsWith & with)
      {
          string res;
      
          res += ResolverInfo::toString (with);
          res += string ("conflicts with ") + with.resItemsToString(false);
      
          return res;
      }
      
      
      ostream &
      ResolverInfoConflictsWith::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }
      
      
      ostream&
      operator<<( ostream& os, const ResolverInfoConflictsWith & with)
      {
          return os << with.asString();
      }
      
      //---------------------------------------------------------------------------
      
      ResolverInfoConflictsWith::ResolverInfoConflictsWith (ResItem_constPtr resItem, ResItem_constPtr with)
          : ResolverInfoContainer (RESOLVER_INFO_TYPE_CONFLICTS_WITH, resItem, RESOLVER_INFO_PRIORITY_USER, with)
      {
      }
      
      
      ResolverInfoConflictsWith::~ResolverInfoConflictsWith ()
      {
      }
      
      
      //---------------------------------------------------------------------------
      
      ResolverInfo_Ptr
      ResolverInfoConflictsWith::copy (void) const
      {
          ResolverInfoConflictsWith_Ptr cpy = new ResolverInfoConflictsWith(resItem(), NULL);
      
          ((ResolverInfoContainer_Ptr)cpy)->copy (this);
      
          return cpy;
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

