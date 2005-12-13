/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _UndumpWorld_h
#define _UndumpWorld_h

#include <iosfwd>
#include <string.h>

#include <zypp/solver/detail/UndumpWorldPtr.h>
#include <zypp/solver/detail/StoreWorld.h>
#include <zypp/solver/detail/Channel.h>
#include <zypp/solver/detail/World.h>

///////////////////////////////////////////////////////////////////
namespace ZYPP {
//////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : UndumpWorld

class UndumpWorld : public StoreWorld {
    REP_BODY(UndumpWorld);

  private:

    typedef std::list<constChannelPtr> ChannelSubscriptions;
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
    virtual bool isSubscribed (constChannelPtr channel) const;
    virtual void setSubscription (constChannelPtr channel, bool is_subscribed);

};
    
///////////////////////////////////////////////////////////////////
}; // namespace ZYPP
///////////////////////////////////////////////////////////////////

#endif // _UndumpWorld_h
