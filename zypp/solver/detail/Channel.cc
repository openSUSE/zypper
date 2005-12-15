/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <y2util/stringutil.h>

#include <zypp/solver/detail/Channel.h>
#include <zypp/solver/detail/World.h>

///////////////////////////////////////////////////////////////////
namespace zypp {
//////////////////////////////////////////////////////////////////

using namespace std;

IMPL_BASE_POINTER(Channel);

//---------------------------------------------------------------------------

int Channel::_fake_id = 1;

//---------------------------------------------------------------------------

string
Channel::asString ( void ) const
{
    if (this == NULL) fprintf (stderr, "Channel::asString NULL\n");
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
    if (channel.legacyId() != NULL
	&& (*(channel.legacyId()) != 0)) {
	res += ", LegacyId: ";
	res += channel.legacyId();
    }
    res += ", Name: ";
    res += channel.name();
    res += ", Alias: ";
    res += channel.alias();

    if (channel.description() != NULL) {
	res += ", Description: ";
	res += channel.description();
    }

    res += ", Priority: ";
    res += stringutil::numstring (channel.priority());
    res += ", PriorityUnsubscribed: ";
    res += stringutil::numstring (channel.priorityUnsubscribed());

    if (channel.path() != NULL) {
	res += ", Path: ";
	res += channel.path();
    }
    if (channel.filePath() != NULL) {
	res += ", FilePath: ";
	res += channel.filePath();
    }
    if (channel.iconFile() != NULL) {
	res += ", IconFile: ";
	res += channel.iconFile();
    }
    if (channel.pkginfoFile() != NULL) {
	res += ", PkginfoFile: ";
	res += channel.pkginfoFile();
    }
	//    list<char *> *_distro_targets; /* List of targets (char *) for this channel */

    res += ", LastUpdate: ";
    res += stringutil::numstring(channel.lastUpdate());

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
	_id = stringutil::form( "fake-id-%d", _fake_id++).c_str();
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

//    if (getenv ("RC_SPEW")) fprintf (stderr, "Channel() [%p] (%s)\n", this, asString().c_str());
}


Channel::Channel (const XmlNodePtr node, int *subscribed, WorldPtr world)
    : _world (world)
    , _last_update (0)
    , _system (false)
    , _hidden (false)
    , _immutable (false)
{
    static unsigned int dummy_id = 0xdeadbeef;
    const char *subscribed_str;
    const char *priority_str;
    const char *priority_unsubscribed_str;

    _name = node->getProp ("name");
    _alias = node->getProp ("alias");
	    
    _id = node->getProp ("id");
    if (_id.empty()) {
	char *temp;
	asprintf (&temp, "dummy:%d", dummy_id);
	_id = temp;
	++dummy_id;
    }

    subscribed_str = node->getProp ("subscribed");
    *subscribed = subscribed_str ? atoi (subscribed_str) : 0;

    priority_str = node->getProp ("priority_base");
    priority_unsubscribed_str = node->getProp ("priority_unsubd");

    _priority = priority_str ? atoi (priority_str) : 0;
    _priority_unsubscribed = priority_unsubscribed_str ? atoi (priority_unsubscribed_str) : 0;
		
    free ((void *)subscribed_str);
    free ((void *)priority_str);
    free ((void *)priority_unsubscribed_str);

//    if (getenv ("RC_SPEW")) fprintf (stderr, "Channel(xml) [%p] (%s)\n", this, asString().c_str());
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
	fprintf (stderr, "Channel::isSubscribed() without world\n");
    return _world->isSubscribed (this);
}


void
Channel::setSubscription (bool subscribed)
{
    if (_world == NULL)
	fprintf (stderr, "Channel::setSubscription() without world\n");
    _world->setSubscription (this, subscribed);
}


int
Channel::priorityParse (const char *priority_cptr) const
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

    if (priority_cptr && *priority_cptr) {
	c = priority_cptr;
	while (*c && is_numeric) {
	    if (! isdigit (*c))
	        is_numeric = false;
	    c++;
	}
	if (is_numeric) {
	    return atoi (priority_cptr);
	}
	
	for (i=0; channel_priority_table[i].str != NULL; ++i) {
	    if (! strcasecmp (channel_priority_table[i].str, priority_cptr))
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
Channel::equals (constChannelPtr channel) const
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
Channel::hasEqualId (constChannelPtr channel) const
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
}; // namespace zypp
///////////////////////////////////////////////////////////////////

