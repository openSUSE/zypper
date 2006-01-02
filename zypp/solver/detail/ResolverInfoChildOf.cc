/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverInfoChildOf.cc
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

#include <zypp/solver/detail/ResolverInfoChildOf.h>

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
      
      IMPL_DERIVED_POINTER(ResolverInfoChildOf, ResolverInfo);
      
      //---------------------------------------------------------------------------
      
      
      string
      ResolverInfoChildOf::asString ( void ) const
      {
          return toString (*this);
      }
      
      
      string
      ResolverInfoChildOf::toString ( const ResolverInfoChildOf & child)
      {
          string res = "<resolverinfochildof '";
      
          res += ResolverInfo::toString (child);
          res += string ("part of ") + child.resItemsToString(false);
          res += "'>";
      
          return res;
      }
      
      
      ostream &
      ResolverInfoChildOf::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }
      
      
      ostream&
      operator<<( ostream& os, const ResolverInfoChildOf & child)
      {
          return os << child.asString();
      }
      
      //---------------------------------------------------------------------------
      
      ResolverInfoChildOf::ResolverInfoChildOf (constResItemPtr resItem, constResItemPtr dependency)
          : ResolverInfoContainer (RESOLVER_INFO_TYPE_CHILD_OF, resItem, RESOLVER_INFO_PRIORITY_USER, dependency)
      {
      }
      
      
      ResolverInfoChildOf::~ResolverInfoChildOf ()
      {
      }
      
      //---------------------------------------------------------------------------
      
      
      ResolverInfoPtr
      ResolverInfoChildOf::copy (void) const
      {
          ResolverInfoChildOfPtr cpy = new ResolverInfoChildOf(resItem(), NULL);
      
          ((ResolverInfoContainerPtr)cpy)->copy (this);
      
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

