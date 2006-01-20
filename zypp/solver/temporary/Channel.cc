/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* Channel.cc
 *
 * Copyright (C) 2000-2002 Ximian, Inc.
 * Copyright (C) 2005 SUSE Linux Products GmbH
 *
 * Definition of 'channel'
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

#include <iostream>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"

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

using namespace std;

IMPL_PTR_TYPE(Channel);

//---------------------------------------------------------------------------

int Channel::_fake_id = 1;

//---------------------------------------------------------------------------

string
Channel::asString ( void ) const
{
    if (this == NULL) ERR << "Channel::asString NULL" << endl;
    return toString (*this);
}


string
Channel::toString ( const Channel & channel )
{
    string res ("<channel '");
    res += "Type: ";
    switch (channel.type()) {
	case CHANNEL_TYPE_ANY: res += "any"; break;
	case CHANNEL_TYPE_SYSTEM: res += "system"; break;
	case CHANNEL_TYPE_NONSYSTEM: res += "non-system"; break;

	case CHANNEL_TYPE_UNKNOWN: res += "unknown"; break;
	case CHANNEL_TYPE_HELIX: res += "helix"; break;
	case CHANNEL_TYPE_DEBIAN: res += "debian"; break;
	case CHANNEL_TYPE_APTRPM: res += "apt-rpm"; break;
	case CHANNEL_TYPE_YAST: res += "yast"; break;
	case CHANNEL_TYPE_YUM: res += "yum"; break;
    }
    res += ", ";

    res += "Id: ";
    res += channel.id();
    if (!channel.legacyId().empty()) {
	res += ", LegacyId: ";
	res += channel.legacyId();
    }
    res += ", Name: ";
    res += channel.name();
    res += ", Alias: ";
    res += channel.alias();

    if (!channel.description().empty()) {
	res += ", Description: ";
	res += channel.description();
    }

    res += ", Priority: ";
    res += str::numstring (channel.priority());
    res += ", PriorityUnsubscribed: ";
    res += str::numstring (channel.priorityUnsubscribed());

    if (!channel.path().empty()) {
	res += ", Path: ";
	res += channel.path();
    }
    if (!channel.filePath().empty()) {
	res += ", FilePath: ";
	res += channel.filePath();
    }
    if (!channel.iconFile().empty()) {
	res += ", IconFile: ";
	res += channel.iconFile();
    }
    if (!channel.pkginfoFile().empty()) {
	res += ", PkginfoFile: ";
	res += channel.pkginfoFile();
    }
	//    list<char *> *_distro_targets; /* List of targets (char *) for this channel */

    res += ", LastUpdate: ";
    res += str::numstring(channel.lastUpdate());

    if (channel.system()) res += ", System! ";
    if (channel.hidden()) res += ", Hidden! ";
    if (channel.immutable()) res += ", Immutable! ";

    return res + "'>";
}


ostream &
Channel::dumpOn( ostream & str ) const
{
    str << asString();
    return str;
}


ostream&
operator<<( ostream& os, const Channel& channel)
{
    return os << channel.asString();
}

//---------------------------------------------------------------------------

Channel::Channel(const string & id, const string & name, const string & alias, const string & description)
    : _world (NULL)
    , _last_update (0)
    , _system (false)
    , _hidden (false)
    , _immutable (false)
{
    if (id.empty()) {
	_id = str::form( "fake-id-%d", _fake_id++).c_str();
    }
    else {
	_id = id;
    }

    if (name.empty()) {
	_name = "Unnamed Channel";
    }
    else {
	_name = name;
    }

    if (alias.empty()) {
	_alias = name;
    }
    else {
	_alias = alias;
    }

    if (description.empty()) {
	_description = "No description available.";
    }
    else {
	_description = description;
    }

    _type = CHANNEL_TYPE_UNKNOWN;

    _priority = -1;
    _priority_unsubscribed = -1;

	  _DBG("RC_SPEW") << "Channel() [" << this << "] (" << asString() << ")" << endl ;
}


Channel::Channel (const XmlNode_Ptr node, int *subscribed, World_Ptr world)
    : _world (world)
    , _last_update (0)
    , _system (false)
    , _hidden (false)
    , _immutable (false)
{
    static unsigned int dummy_id = 0xdeadbeef;

    _name = node->getProp ("name");
    _alias = node->getProp ("alias");

    _id = node->getProp ("id");
    if (_id.empty()) {
	char *temp;
	asprintf (&temp, "dummy:%d", dummy_id);
	_id = temp;
	++dummy_id;
    }

    *subscribed = node->getIntValueDefault ("subscribed", 0);

    _priority = node->getIntValueDefault ("priority_base", 0);
    _priority_unsubscribed = node->getIntValueDefault ("priority_unsubd", 0);

    _DBG("RC_SPEW") << "Channel(xml) [" << this << "] (" << asString() << ")" << endl;
}


Channel::~Channel()
{
}


#if 0
xmlNode *
rc_channel_to_xml_node (RCChannel *channel)
{
    xmlNode *node;
    char tmp[128];

    g_return_val_if_fail (channel != NULL, NULL);

    node = xmlNewNode (NULL, "channel");

    xmlNewProp (node, "id", rc_channel_get_id (channel));

    xmlNewProp (node, "name", rc_channel_get_name (channel));

    if (rc_channel_get_alias (channel))
	xmlNewProp (node, "alias", rc_channel_get_alias (channel));

    sprintf (tmp, "%d", rc_channel_is_subscribed (channel) ? 1 : 0);
    xmlNewProp (node, "subscribed", tmp);

    sprintf (tmp, "%d", rc_channel_get_priority (channel, true));
    xmlNewProp (node, "priority_base", tmp);

    sprintf (tmp, "%d", rc_channel_get_priority (channel, false));
    xmlNewProp (node, "priority_unsubd", tmp);

    return node;
}
#endif


bool
Channel::isSubscribed (void) const
{
    if (_world == NULL)
	ERR << "Channel::isSubscribed() without world" << endl;
    return _world->isSubscribed (this);
}


void
Channel::setSubscription (bool subscribed)
{
    if (_world == NULL)
	ERR << "Channel::setSubscription() without world" << endl;
    _world->setSubscription (this, subscribed);
}


int
Channel::priorityParse (const string & priority_str) const
{
#define DEFAULT_CHANNEL_PRIORITY 1600

    typedef struct {
	const char *str;
	int priority;
     } ChannelPriorityPair;

    ChannelPriorityPair channel_priority_table[] = {
	{ "private",     6400 },
	{ "ximian",      3200 },
	{ "distro",      1600 },
	{ "third_party",  800 },
	{ "preview",      400 },
	{ "untested",     200 },
	{ "snapshot",     100 },
	{ NULL,		    0 }
    };

    const char *c;
    int i;
    bool is_numeric = true;

    if (!priority_str.empty()) {
	c = priority_str.c_str();
	while (*c && is_numeric) {
	    if (! isdigit (*c))
	        is_numeric = false;
	    c++;
	}
	if (is_numeric) {
	    return atoi (priority_str.c_str());
	}

	for (i=0; channel_priority_table[i].str != NULL; ++i) {
	    if (! strcasecmp (channel_priority_table[i].str, priority_str.c_str()))
	        return channel_priority_table[i].priority;
	}

    }

    return DEFAULT_CHANNEL_PRIORITY;
}


bool
Channel::isWildcard (void) const
{
    return _type == CHANNEL_TYPE_SYSTEM
	|| _type == CHANNEL_TYPE_NONSYSTEM
	|| _type == CHANNEL_TYPE_ANY;
}


bool
Channel::equals (const Channel & channel) const
{
    return equals (&channel);
}

bool
Channel::equals (Channel_constPtr channel) const
{
    if (_type == CHANNEL_TYPE_ANY
	|| channel->_type == CHANNEL_TYPE_ANY) {
	return true;
    }

    if (isWildcard () && channel->isWildcard ()) {
	return this == channel;
    }

    /* So at this point we know that between a and b there is
       at most one wildcard. */

    if (_type == CHANNEL_TYPE_SYSTEM) {
	return channel->system();
    }
    else if (_type == CHANNEL_TYPE_NONSYSTEM) {
	return !channel->system();
    }

    if (channel->_type == CHANNEL_TYPE_SYSTEM) {
	return system();
    }
    else if (channel->_type == CHANNEL_TYPE_NONSYSTEM) {
	return !system();
    }

    return hasEqualId (channel);
}


bool
Channel::hasEqualId (const Channel & channel) const
{
    return hasEqualId (&channel);
}


bool
Channel::hasEqualId (Channel_constPtr channel) const
{
    return (channel->id () == _id);
}


void
Channel::setPriorities (int subscribed_priority, int unsubscribed_priority)
{
    if (immutable()) return;

    _priority = subscribed_priority;
    _priority_unsubscribed = unsubscribed_priority;
}


int
Channel::getPriority(bool is_subscribed) const
{
#define UNSUBSCRIBED_CHANNEL_ADJUSTMENT(x) ((x)/2)

    int priority;

    priority = _priority;
    if (priority <= 0)
	priority = DEFAULT_CHANNEL_PRIORITY;

    if (!is_subscribed) {
	if (_priority_unsubscribed > 0) {
	    priority = _priority_unsubscribed;
	} else {
	    priority = UNSUBSCRIBED_CHANNEL_ADJUSTMENT (priority);
	}
    }

    return priority;
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

