/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverInfoMissingReq.cc
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
#include <zypp/solver/detail/ResolverInfoMissingReq.h>

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
      
      IMPL_DERIVED_POINTER(ResolverInfoMissingReq, ResolverInfo);
      
      //---------------------------------------------------------------------------
      
      
      string
      ResolverInfoMissingReq::asString ( void ) const
      {
          return toString (*this);
      }
      
      
      string
      ResolverInfoMissingReq::toString ( const ResolverInfoMissingReq & missing)
      {
          string res;
      
          res += ResolverInfo::toString (missing);
          res += string ("missing requirement ") + missing._missing_req.asString();
      
          return res;
      }
      
      
      ostream &
      ResolverInfoMissingReq::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }
      
      
      ostream&
      operator<<( ostream& os, const ResolverInfoMissingReq & missing)
      {
          return os << missing.asString();
      }
      
      //---------------------------------------------------------------------------
      
      ResolverInfoMissingReq::ResolverInfoMissingReq (constResItemPtr resItem, const Capability & missing_req)
          : ResolverInfo (RESOLVER_INFO_TYPE_MISSING_REQ, resItem, RESOLVER_INFO_PRIORITY_USER)
          , _missing_req (missing_req)
      {
      }
      
      
      ResolverInfoMissingReq::~ResolverInfoMissingReq ()
      {
      }
      
      //---------------------------------------------------------------------------
      
      ResolverInfoPtr
      ResolverInfoMissingReq::copy (void) const
      {
          ResolverInfoMissingReqPtr cpy = new ResolverInfoMissingReq(resItem(), _missing_req);
      
          ((ResolverInfoPtr)cpy)->copy (this);
      
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


