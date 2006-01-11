/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverInfoMisc.cc
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

#include "zypp/solver/detail/ResolverInfo.h"
#include "zypp/solver/detail/ResolverInfoMisc.h"
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

      IMPL_PTR_TYPE(ResolverInfoMisc);

      //---------------------------------------------------------------------------


      string
      ResolverInfoMisc::asString ( void ) const
      {
          return toString (*this);
      }


      string
      ResolverInfoMisc::toString ( const ResolverInfoMisc & misc)
      {
          string res;
          res += misc._msg;
      #if 0
          res += " [";
          res += ResolverInfo::toString (misc, false);
          res += "]";
      #endif
          res += misc.resItemsToString(false);
          if (!misc._action.empty()) {
      	res += string (_(", Action: ")) + misc._action + "\n";
          }
          if (!misc._trigger.empty()) {
      	res += string (_(", Trigger: ")) + misc._trigger + "\n";
          }

          return res;
      }


      ostream &
      ResolverInfoMisc::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }


      ostream&
      operator<<( ostream& os, const ResolverInfoMisc & misc)
      {
          return os << misc.asString();
      }

      //---------------------------------------------------------------------------

      ResolverInfoMisc::ResolverInfoMisc (ResItem_constPtr resItem, int priority, const string & msg)
          : ResolverInfoContainer (RESOLVER_INFO_TYPE_MISC, resItem, priority)
          , _msg (msg)
      {
      }


      ResolverInfoMisc::~ResolverInfoMisc ()
      {
      }

      //---------------------------------------------------------------------------

      bool
      ResolverInfoMisc::merge (ResolverInfo_Ptr info)
      {
          bool res;
          ResolverInfoMisc_Ptr to_be_merged = dynamic_pointer_cast<ResolverInfoMisc>(info);

          res = ResolverInfo::merge(to_be_merged);
          if (!res) return res;

          if (!_msg.empty()
              && !to_be_merged->_msg.empty()
      	&& _msg == to_be_merged->_msg) {
                  return true;
          }

          return false;
      }


      ResolverInfo_Ptr
      ResolverInfoMisc::copy (void) const
      {
          ResolverInfoMisc_Ptr cpy = new ResolverInfoMisc(resItem(), priority(), _msg);

          ((ResolverInfoContainer_Ptr)cpy)->copy (this);

          return cpy;
      }

      //---------------------------------------------------------------------------

      void
      ResolverInfoMisc::addAction (const std::string & action_msg)
      {
          _action = action_msg;
      }


      void
      ResolverInfoMisc::addTrigger (const std::string & trigger_msg)
      {
          _trigger = trigger_msg;
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

