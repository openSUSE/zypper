/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* Subscription.h
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

#ifndef ZYPP_SOLVER_TEMPORARY_SUBSCRIPTION_H
#define ZYPP_SOLVER_TEMPORARY_SUBSCRIPTION_H

#include <iosfwd>
#include <string>
#include <list>
#include <ctime>

#include "zypp/solver/temporary/ChannelPtr.h"


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
//	CLASS NAME : Subscription


class Subscription;
typedef std::list<Subscription *> SubscriptionList;

class Subscription {

  private:

    static SubscriptionList subscriptions;
    static bool subscriptions_changed;
    static std::string subscription_file;

    std::string _channel_id;
    time_t _last_seen;
    bool _old;		// subscription imported from an old-style subs file

    bool match (Channel_constPtr channel);
    static void save (void);
    static void load (void);
    static void load_old_subscriptions (void);

  public:

    Subscription (const std::string & id);
    virtual ~Subscription();

    // ---------------------------------- I/O

    static std::string toString ( const Subscription & section);

    virtual std::ostream & dumpOn( std::ostream & str ) const;

    friend std::ostream& operator<<( std::ostream&, const Subscription & section);

    std::string asString ( void ) const;

    // ---------------------------------- accessors

    // ---------------------------------- methods

    void setFile (const std::string & file);
    static bool status (Channel_constPtr channel);
    static void setStatus (Channel_constPtr channel, bool channel_is_subscribed);

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

#endif // ZYPP_SOLVER_TEMPORARY_SUBSCRIPTION_H
