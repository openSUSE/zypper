/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverInfoMisc.h
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

#ifndef ZYPP_SOLVER_DETAIL_RESOLVERINFOMISC_H
#define ZYPP_SOLVER_DETAIL_RESOLVERINFOMISC_H

#include <string>
#include "zypp/solver/detail/ResolverInfoMiscPtr.h"
#include "zypp/solver/detail/ResolverInfoContainer.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp 
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : ResolverInfoMisc
      
      class ResolverInfoMisc : public ResolverInfoContainer {
      
          
      
        private:
      
          std::string _msg;
          std::string _action;
          std::string _trigger;
      
        public:
      
          ResolverInfoMisc (ResItem_constPtr resItem, int priority, const std::string & msg);
          virtual ~ResolverInfoMisc();
      
          // ---------------------------------- I/O
      
          static std::string toString (const ResolverInfoMisc & context);
          virtual std::ostream & dumpOn(std::ostream & str ) const;
          friend std::ostream& operator<<(std::ostream&, const ResolverInfoMisc & context);
          std::string asString (void ) const;
      
          // ---------------------------------- accessors
      
          // ---------------------------------- methods
      
          virtual bool merge (ResolverInfo_Ptr to_be_merged);
          virtual ResolverInfo_Ptr copy (void) const;
      
          void addAction (const std::string & action_msg);
          void addTrigger (const std::string & trigger_msg);
      
      };

      ///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////        
#endif // ZYPP_SOLVER_DETAIL_RESOLVERINFOMISC_H
 
