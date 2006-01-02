/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResItemAndDependency.cc
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

#include <zypp/solver/detail/ResItemAndDependency.h>
#include <zypp/solver/detail/debug.h>

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
      
      IMPL_BASE_POINTER(ResItemAndDependency);
      
      //---------------------------------------------------------------------------
      
      ResItemAndDependency::ResItemAndDependency (constResItemPtr resItem, const Capability & dependency)
          : _resItem(resItem)
          , _dependency(dependency)
      {
      }
      
      //---------------------------------------------------------------------------
      
      string
      ResItemAndDependency::asString (bool full) const
      {
          return toString (*this, full);
      }
      
      
      string
      ResItemAndDependency::toString ( const ResItemAndDependency & r_and_d, bool full )
      {
          string res ("{");
          res += r_and_d._resItem->asString(full);
          res += ", ";
          res += r_and_d._dependency.asString();
          res += "}";
          return res;
      }
      
      
      ostream &
      ResItemAndDependency::dumpOn (ostream & str) const
      {
          str << asString();
          return str;
      }
      
      
      ostream &
      operator<< (ostream & os, const ResItemAndDependency & r_and_d)
      {
          return os << r_and_d.asString();
      }
      
      //---------------------------------------------------------------------------
      
      /* This function also checks channels in addition to just dep relations */
      /* FIXME: rc_resItem_dep_verify_relation already checks the channel */
      
      bool
      ResItemAndDependency::verifyRelation (const Capability & dep) const
      {
      #if PHI
          // don't check the channel, thereby honoring conflicts from installed resItems to to-be-installed resItems
          return dep.matches (_dependency);
      #else
          if (!dep.matches (_dependency)) {
      	return false;
          }
#if 0
          if (getenv ("SPEW_DEP")) fprintf (stderr, "ResItemAndDependency::verifyRelation _resItem->channel() %s, dep->channel() %s\n", _resItem->channel()->asString().c_str(), dep->channel()->asString().c_str());
          return _resItem->channel()->equals (dep->channel());
#else
          return true;
#endif
      #endif
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

