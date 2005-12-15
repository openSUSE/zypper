/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* Subscription.cc
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

#include <y2util/stringutil.h>

#include <zypp/solver/detail/Subscription.h>
#include <zypp/solver/detail/Channel.h>
#include <zypp/solver/detail/XmlNode.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <list>
#include <unistd.h>
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
      
      #define SUBSCRIPTION_PATH "/var/adm/zypp"
      #define OLD_SUBSCRIPTION_PATH "/var/lib/rcd"
      #define SUBSCRIPTION_NAME "/subscriptions.xml"
      
      #define DEFAULT_SUBSCRIPTION_FILE SUBSCRIPTION_PATH SUBSCRIPTION_NAME
      #define OLD_SUBSCRIPTION_FILE OLD_SUBSCRIPTION_PATH SUBSCRIPTION_NAME
      
      /* Old subscriptions expire in 60 days */
      #define OLD_SUBSCRIPTION_EXPIRATION 60*24*60*60
      
      SubscriptionList Subscription::subscriptions;
      bool Subscription::subscriptions_changed = false;
      const char *Subscription::subscription_file = DEFAULT_SUBSCRIPTION_FILE;
      
      //---------------------------------------------------------------------------
      
      string
      Subscription::asString ( void ) const
      {
          return toString (*this);
      }
      
      
      string
      Subscription::toString ( const Subscription & s)
      {
          return "<subscription/>";
      }
      
      ostream &
      Subscription::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }
      
      
      ostream&
      operator<<( ostream& os, const Subscription & s)
      {
          return os << s.asString();
      }
      
      //---------------------------------------------------------------------------
      
      
      void
      Subscription::save (void)
      {
          xmlDoc *doc;
          xmlNode *root;
          char buf[64];
          time_t now;
          int save_retval;
      
          if (! subscriptions_changed)
      	return;
      
          time (&now);
      
          root = xmlNewNode (NULL, (const xmlChar*)"subscriptions");
          xmlNewProp (root, (const xmlChar*)"version", (const xmlChar*)"2.0");
      
          doc = xmlNewDoc ((const xmlChar*)"1.0");
          xmlDocSetRootElement (doc, root);
      
          for (SubscriptionList::iterator iter = subscriptions.begin(); iter != subscriptions.end(); iter++) {
      	xmlNode *sub_node;
      
      	Subscription *sub = *iter;
      
      	/* Drop "old" (i.e. imported from 1.x) subscriptions that
      	   we haven't seen for a while. */
      	if (sub->_old) {
      	    double elapsed = difftime (now, sub->_last_seen);
      	    if (elapsed > OLD_SUBSCRIPTION_EXPIRATION)
      		continue;
      	}
      
      	sub_node = xmlNewChild (root, NULL, (const xmlChar*)"channel", NULL);
      
      	xmlNewProp (sub_node, (const xmlChar*)"id", (const xmlChar*)(sub->_channel_id.c_str()));
      
      	snprintf (buf, sizeof (buf), "%ld", (long) sub->_last_seen);
      	xmlNewProp (sub_node, (const xmlChar*)"last_seen", (const xmlChar*)buf);
      
      	if (sub->_old)
      	    xmlNewProp (sub_node, (const xmlChar*)"old", (const xmlChar*)"1");
          }
      
          save_retval = xmlSaveFile (subscription_file, doc);
          xmlFreeDoc (doc);
      
          if (save_retval > 0) {
      	/* Writing out the subscription file succeeded. */
      	subscriptions_changed = false;
          } else {
      	rc_debug (RC_DEBUG_LEVEL_WARNING, "Unable to save subscription data to '%s'", subscription_file);
      	rc_debug (RC_DEBUG_LEVEL_WARNING, "Subscription will not be saved!");
          }
      }
      
      
      void
      Subscription::load_old_subscriptions (void)
      {
          static bool tried_to_do_this_already = false;
          xmlDoc *doc;
          XmlNodePtr node;
      
          if (tried_to_do_this_already)
      	return;
          tried_to_do_this_already = true;
      
          if (access (OLD_SUBSCRIPTION_FILE, R_OK) != 0) {
      	rc_debug (RC_DEBUG_LEVEL_WARNING, "Can't find rcd 1.x subscription file '%s'", OLD_SUBSCRIPTION_FILE);
      	return;
          }
      
          doc = xmlParseFile (OLD_SUBSCRIPTION_FILE);
          if (doc == NULL) {
      	rc_debug (RC_DEBUG_LEVEL_ERROR, "Can't parse rcd 1.x subscription file '%s'", OLD_SUBSCRIPTION_FILE);
      	return;
          }
      
          node = new XmlNode (xmlDocGetRootElement (doc));
      
          if (!node->equals("subscriptions")) {
      	rc_debug (RC_DEBUG_LEVEL_ERROR, "rcd 1.x subscription file '%s' is malformed", OLD_SUBSCRIPTION_FILE);
      	return;
          }
      
          rc_debug (RC_DEBUG_LEVEL_INFO, "Importing rcd 1.x subscriptions.");
          
          node = node->children();
      
          while (node != NULL) {
      
      	if (node->equals ("channel")) {
      	    const char *id_str;
      
      	    id_str = node->getProp ("channel_id");
      	    if (id_str && *id_str) {
      
      		Subscription *sub = new Subscription (id_str);
      		sub->_old = true;
      
      		subscriptions.push_back (sub);
      	    }
      	}
      
      	node = node->next();
          }
      
          /* If we've imported old subscriptions, we need to write them
             out immediately into the new subscriptions file. */
      
          subscriptions_changed = true;
          save ();
      }
      
      
      void
      Subscription::load (void)
      {
          xmlDoc *doc;
          XmlNodePtr node;
      
          if (access (subscription_file, R_OK) != 0) {
      	load_old_subscriptions ();
      	return;
          }
      
          doc = xmlParseFile (subscription_file);
          if (doc == NULL) {
      	rc_debug (RC_DEBUG_LEVEL_ERROR, "Can't parse subscription file '%s'", subscription_file);
      	return;
          }
      
          node = new XmlNode (xmlDocGetRootElement (doc));
      
          if (! node->equals ("subscriptions")) {
      	rc_debug (RC_DEBUG_LEVEL_ERROR, "Subscription file '%s' is malformed", subscription_file);
      	return;
          }
      
          node = node->children();
      
          while (node != NULL) {
      
      	if (node->equals ("channel")) {
      	    const char *id_str, *last_seen_str;
      
      	    id_str = node->getProp ("id");
      	    last_seen_str = node->getProp ("last_seen");
      
      	    if (id_str && *id_str) {
      		Subscription *sub = new Subscription (id_str);
      	     
      		if (last_seen_str)
      		    sub->_last_seen = (time_t) atol (last_seen_str);
      		else
      		    sub->_last_seen = time (NULL);
      
      		sub->_old = node->getUnsignedIntValueDefault("old", 0);
      
      		subscriptions.push_back (sub);
      	    }
      
      	    free ((void *)id_str);
      	    free ((void *)last_seen_str);
      
      	}
      
      	node = node->next();
          }
      
          xmlFreeDoc (doc);
      }
      
      //---------------------------------------------------------------------------
      
      bool
      Subscription::match (constChannelPtr channel)
      {
          bool match;
      
          /* Paranoia is the programmer's friend. */
          if (channel == NULL) return false;
          if (channel->id() == NULL) return false;
      
          /* If this is an old (i.e. imported from 1.x) subscription, we
             compare it against the channel id's tail. */
      
          if (_old) {
      	const char *id = channel->legacyId ();
      	int len1, len2;
      
      	if (!id)
      	    return false;
      
      	len1 = strlen (_channel_id.c_str());
      	len2 = strlen (id);
      
      	if (len1 > len2)
      	    return false;
      
      	/* If the tails match, mutate the Subscription into a
      	   new-style subscription for that channel. */
      	if (! strcmp (id + (len2 - len1), _channel_id.c_str())) {
      	    _channel_id = channel->id ();
      	    _old = false;
      	    subscriptions_changed = true;
      
      	    return true;
      	}
      
      	return false;
          }
      
          match = (_channel_id == channel->id ());
      
          if (match) {
      	time (&_last_seen);
          }
      
          return match;
      }
      
      //-----------------------------------------------------------------------------
      
      void
      Subscription::setFile (const char *path)
      {
          subscription_file = path;
      }
      
      
      bool
      Subscription::status (constChannelPtr channel)
      {
          if (subscriptions.empty())
      	load ();
      
          if (channel == NULL)
      	return false;
      
          for (SubscriptionList::iterator iter = subscriptions.begin(); iter != subscriptions.end(); iter++) {
      	Subscription *sub = *iter;
      	if (sub->match (channel))
      	    return true;
          }
      
          save ();
      
          return false;
      }
      
      
      void
      Subscription::setStatus (constChannelPtr channel, bool subscribe_to_channel)
      {
          bool currently_subscribed;
      
          if (channel == NULL) return;
      
          currently_subscribed = status (channel);
      
          if (currently_subscribed && !subscribe_to_channel) {
      
      	/* Unsubscribe to the channel */
      	for (SubscriptionList::iterator iter = subscriptions.begin(); iter != subscriptions.end(); iter++) {
      	    Subscription *sub = *iter;
      	    if (sub->match (channel)) {
      		subscriptions.erase (iter);
      		subscriptions_changed = true;
      		break;
      	    }
      	}
      
          } else if (!currently_subscribed && subscribe_to_channel) {
      
      	/* Subscribe to the channel */
      	Subscription *sub;
      	sub = new Subscription (channel->id ());
      	subscriptions.push_back(sub);
      	subscriptions_changed = true;
          }
      
          save ();
      }
      
      //---------------------------------------------------------------------------
      
      Subscription::Subscription(const char *id)
      {
          _channel_id = string (id);
          _last_seen  = time (NULL);
          _old        = false;
      }
      
      
      Subscription::~Subscription()
      {
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

