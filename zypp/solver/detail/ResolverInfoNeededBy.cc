/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverInfoNeededBy.cc
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
#include <zypp/solver/detail/ResolverInfoNeededBy.h>

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
      
      IMPL_PTR_TYPE(ResolverInfoNeededBy);
      
      //---------------------------------------------------------------------------
      
      
      string
      ResolverInfoNeededBy::asString ( void ) const
      {
          return toString (*this);
      }
      
      
      string
      ResolverInfoNeededBy::toString ( const ResolverInfoNeededBy & by)
      {
          string res;
      
          res += ResolverInfo::toString (by, false);
          res += string (" needed by ") + by.resItemsToString(false);
      
          return res;
      }
      
      
      ostream &
      ResolverInfoNeededBy::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }
      
      
      ostream&
      operator<<( ostream& os, const ResolverInfoNeededBy & by)
      {
          return os << by.asString();
      }
      
      //---------------------------------------------------------------------------
      
      ResolverInfoNeededBy::ResolverInfoNeededBy (ResItem_constPtr resItem)
          : ResolverInfoContainer (RESOLVER_INFO_TYPE_NEEDED_BY, resItem, RESOLVER_INFO_PRIORITY_USER, NULL)
      {
      }
      
      
      ResolverInfoNeededBy::~ResolverInfoNeededBy ()
      {
      }
      
      //---------------------------------------------------------------------------
      
      ResolverInfo_Ptr
      ResolverInfoNeededBy::copy (void) const
      {
          ResolverInfoNeededBy_Ptr cpy = new ResolverInfoNeededBy(resItem());
      
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

