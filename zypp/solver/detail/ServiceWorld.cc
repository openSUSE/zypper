/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ServiceWorld.cc
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
      
      IMPL_DERIVED_POINTER(ServiceWorld, World);
      
      //---------------------------------------------------------------------------
      
      string
      ServiceWorld::asString ( void ) const
      {
          return toString (*this);
      }
      
      
      string
      ServiceWorld::toString ( const ServiceWorld & service )
      {
          return "<serviceworld/>";
      }
      
      
      ostream &
      ServiceWorld::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }
      
      
      ostream&
      operator<<( ostream& os, const ServiceWorld & service)
      {
          return os << service.asString();
      }
      
      //---------------------------------------------------------------------------
      
      ServiceWorld::ServiceWorld ()
          : StoreWorld (SERVICE_WORLD)
      {
      }
      
      
      ServiceWorld::~ServiceWorld()
      {
          fprintf (stderr, "*** deleting service world[%p]: %s\n", this, World::toString(type()).c_str());
      }
      
      //---------------------------------------------------------------------------
      
      ///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////

