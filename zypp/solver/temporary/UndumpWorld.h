/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* UndumpWorld.h
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

#ifndef ZYPP_SOLVER_TEMPORARY_UNDUMPWORLD_H
#define ZYPP_SOLVER_TEMPORARY_UNDUMPWORLD_H

#include <iosfwd>
#include <string>

#include "zypp/solver/temporary/UndumpWorldPtr.h"
#include "zypp/solver/temporary/StoreWorld.h"
#include "zypp/solver/temporary/Channel.h"
#include "zypp/solver/temporary/World.h"

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
      //	CLASS NAME : UndumpWorld
      
      class UndumpWorld : public StoreWorld {
          
      
        private:
      
          typedef std::list<Channel_constPtr> ChannelSubscriptions;
          ChannelSubscriptions _subscriptions;
      
        public:
      
          UndumpWorld (const char *filename);
          virtual ~UndumpWorld();
      
          // ---------------------------------- I/O
      
          static std::string toString (const UndumpWorld & section);
      
          virtual std::ostream & dumpOn(std::ostream & str ) const;
      
          friend std::ostream& operator<<(std::ostream&, const UndumpWorld & section);
      
          std::string asString (void ) const;
      
          // ---------------------------------- accessors
      
          // ---------------------------------- methods
      
          void load (const char *filename);
          virtual bool isSubscribed (Channel_constPtr channel) const;
          virtual void setSubscription (Channel_constPtr channel, bool is_subscribed);
      
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

#endif // ZYPP_SOLVER_TEMPORARY_UNDUMPWORLD_H
