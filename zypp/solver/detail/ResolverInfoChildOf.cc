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

#include "zypp/solver/detail/ResolverInfoChildOf.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"

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
      
      IMPL_PTR_TYPE(ResolverInfoChildOf);
      
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
      
          res += str::form (_("%s part of %s"),
			    ResolverInfo::toString (child).c_str(),
			    child.resItemsToString(false).c_str());
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
      
      ResolverInfoChildOf::ResolverInfoChildOf (ResItem_constPtr resItem, ResItem_constPtr dependency)
          : ResolverInfoContainer (RESOLVER_INFO_TYPE_CHILD_OF, resItem, RESOLVER_INFO_PRIORITY_USER, dependency)
      {
      }
      
      
      ResolverInfoChildOf::~ResolverInfoChildOf ()
      {
      }
      
      //---------------------------------------------------------------------------
      
      
      ResolverInfo_Ptr
      ResolverInfoChildOf::copy (void) const
      {
          ResolverInfoChildOf_Ptr cpy = new ResolverInfoChildOf(resItem(), NULL);
      
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

