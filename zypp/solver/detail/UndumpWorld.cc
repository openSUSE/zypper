/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* UndumpWorld.cc
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

#include <zypp/solver/detail/UndumpWorld.h>
#include <zypp/solver/detail/extract.h>

///////////////////////////////////////////////////////////////////
namespace ZYPP {
//////////////////////////////////////////////////////////////////

using namespace std;

IMPL_DERIVED_POINTER(UndumpWorld, World);

//---------------------------------------------------------------------------

string
UndumpWorld::asString ( void ) const
{
    return toString (*this);
}


string
UndumpWorld::toString ( const UndumpWorld & world )
{
    return "<undumpworld/>";
}


ostream &
UndumpWorld::dumpOn( ostream & str ) const
{
    str << asString();
    return str;
}


ostream&
operator<<( ostream& os, const UndumpWorld & world)
{
    return os << world.asString();
}

//---------------------------------------------------------------------------

UndumpWorld::UndumpWorld (const char *filename)
    : StoreWorld (UNDUMP_WORLD)
{
    load (filename);
}


UndumpWorld::~UndumpWorld()
{
    fprintf (stderr, "*** deleting undump world[%p]: %s\n", this, World::toString(type()).c_str());
}

//---------------------------------------------------------------------------


static bool
add_channel_cb (ChannelPtr channel, bool subscribed, void *data)
{
    UndumpWorld *undump = (UndumpWorld *)data;

    undump->addChannel (channel);

    if (!channel->system ()) {
	undump->setSubscription (channel, subscribed);
    }

    return true;
}


static bool
add_resolvable_cb (constResolvablePtr res, void *data)
{
    UndumpWorld *undump = (UndumpWorld *)data;

    undump->addResolvable (res);

    return true;
}


static bool
add_lock_cb (constMatchPtr lock, void *data)
{
    UndumpWorld *undump = (UndumpWorld *)data;

    undump->addLock (lock);
    
    return true;
}


void
UndumpWorld::load (const char *filename)
{
    if (filename) {
	extract_packages_from_undump_file (filename, add_channel_cb, add_resolvable_cb, add_lock_cb, (void *)this);
    }
}


void
UndumpWorld::setSubscription (constChannelPtr channel, bool subscribe)
{
//    if (getenv("RC_SPEW")) fprintf (stderr, "UndumpWorld::setSubscription (%s, %s)\n", channel->asString().c_str(), subscribe?"subscribe":"unsubscribe");
    for (ChannelSubscriptions::iterator i = _subscriptions.begin(); i != _subscriptions.end(); i++) {
	if (*i == channel) {
	    if (!subscribe) {
		_subscriptions.erase (i);
	    }
	    return;
	}
    }

    if (subscribe) {
	_subscriptions.push_back (channel);
    }

    return;
}


bool
UndumpWorld::isSubscribed (constChannelPtr channel) const
{
    for (ChannelSubscriptions::const_iterator i = _subscriptions.begin(); i != _subscriptions.end(); i++) {
	if (*i == channel) {
	    if (getenv("RC_SPEW")) fprintf (stderr, "UndumpWorld::isSubscribed (%s) YES\n", channel->asString().c_str());
	    return true;
	}
    }

    if (getenv("RC_SPEW")) fprintf (stderr, "UndumpWorld::isSubscribed (%s) NO\n", channel->asString().c_str());
    return false;
}



///////////////////////////////////////////////////////////////////
}; // namespace ZYPP
///////////////////////////////////////////////////////////////////

