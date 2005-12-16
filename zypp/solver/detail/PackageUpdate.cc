/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* PackageUpdate.cc
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

#include <stdio.h>
#include <string.h>
#include <libgen.h>

#include <zypp/solver/detail/utils.h>
#include <zypp/solver/detail/PackageUpdate.h>
#include <zypp/solver/detail/Package.h>

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
      
      IMPL_DERIVED_POINTER(PackageUpdate, Spec);
      
      //---------------------------------------------------------------------------
      
      string
      PackageUpdate::asString ( bool full ) const
      {
          return toString (*this);
      }
      
      
      string
      PackageUpdate::toString ( const PackageUpdate & package_update, bool full )
      {
          string ret;
          ret += ((const Spec &)package_update).asString(full);
      
          return ret;
      }
      
      
      ostream &
      PackageUpdate::dumpOn( ostream & str ) const
      {
          str << asString();
          return str;
      }
      
      
      ostream&
      operator<<( ostream& os, const PackageUpdate& package_update)
      {
          return os << package_update.asString();
      }
      
      
      const XmlNodePtr
      PackageUpdate::asXmlNode (void) const
      {
          XmlNodePtr update_node = new XmlNode("update");
          string tmp;
      
          if (hasEpoch()) {
      	tmp = stringutil::form("%d", epoch());
      	update_node->addTextChild ("epoch", tmp.c_str());
          }
      
          update_node->addTextChild ("version", version().c_str());
      
          if (!release().empty()) {
      	update_node->addTextChild ("release", release().c_str());
          }
      
          if (_package_url && *_package_url) {
      	update_node->addTextChild ("filename", basename (strdup (_package_url)));
          }
      
          tmp = stringutil::form ("%ld", (unsigned long)_package_size);
          update_node->addTextChild ("filesize", tmp.c_str());
      
          tmp = stringutil::form ("%ld", (unsigned long)_installed_size);
          update_node->addTextChild ("installedsize", tmp.c_str());
      
          if (_signature_url) {
      	update_node->addTextChild ("signaturename", _signature_url);
      
      	tmp = stringutil::form ("%ld", (unsigned long)_signature_size);
      	update_node->addTextChild ("signaturesize", tmp.c_str());
          }
      
          if (_md5sum) {
      	update_node->addTextChild ("md5sum", _md5sum);
          }
      
          update_node->addTextChild ("importance", _importance->asString().c_str());
      
          update_node->addTextChild ("description", _description);
      
          if (_hid) {
      	tmp = stringutil::form ("%d", _hid);
      	update_node->addTextChild ("hid", tmp.c_str());
          }
      
          if (_license) {
      	update_node->addTextChild ("license", _license);
          }
      
          return update_node;
      }
      
      //---------------------------------------------------------------------------
      
      PackageUpdate::PackageUpdate (const string & name)
          : Spec (ResTraits<zypp::Package>::kind, name)
          , _package (NULL)
          , _package_url (NULL)
          , _package_size (0)
          , _installed_size (0)
          , _signature_url (NULL)
          , _signature_size (0)
          , _md5sum (NULL)
          , _importance (NULL)
          , _hid (0)
          , _description (NULL)
          , _license (NULL)
          , _parent (NULL)
      {
      }
      
      
      PackageUpdate::PackageUpdate (constXmlNodePtr node, PackagePtr package)
          : Spec (ResTraits<zypp::Package>::kind, package->name())
          , _package (NULL)
          , _package_url (NULL)
          , _package_size (0)
          , _installed_size (0)
          , _signature_url (NULL)
          , _signature_size (0)
          , _md5sum (NULL)
          , _importance (NULL)
          , _hid (0)
          , _description (NULL)
          , _license (NULL)
          , _parent (NULL)
      {
          constChannelPtr channel;
          const char *url_prefix = NULL;
      
          if (node == NULL) {
      	fprintf (stderr, "PackageUpdate::PackageUpdate(NULL)\n");
      	exit (1);
          }
      
          /* Make sure this is an update node */
          if (strcasecmp (node->name(), "update")) {
      	fprintf (stderr, "PackageUpdate::PackageUpdate() wrong node (%s)\n", node->name());
      	exit (1);
          }
      
          channel = package->channel();
      
          _package = package;
      
          if (channel) {
      	url_prefix = channel->filePath();
          }
      
          XmlNodePtr iter = node->children();
      
          while (iter) { 
      	if (iter->equals ("epoch")) {			setEpoch (iter->getUnsignedIntContentDefault (0));
      	} else if (iter->equals ("version")) {		setVersion (iter->getContent());
      	} else if (iter->equals ("release")) {		setRelease (iter->getContent());
      	} else if (iter->equals ("arch")) {		setArch (iter->getContent());
      	} else if (iter->equals ("filename")) {
      	    const char *tmp = iter->getContent();
      	    if (url_prefix) {
      		_package_url = maybe_merge_paths (url_prefix, tmp);
      	    } else {
      		_package_url = strdup (tmp);
      	    }
      	} else if (iter->equals ("filesize")) {		_package_size = iter->getUnsignedIntContentDefault (0);
      	} else if (iter->equals ("installedsize")) {	_installed_size = iter->getUnsignedIntContentDefault (0);
      	} else if (iter->equals ("signaturename")) {
      	    const char *tmp = iter->getContent();
      	    if (url_prefix) {
      		_signature_url = maybe_merge_paths (url_prefix, tmp);
      	    } else {
      		_signature_url = strdup (tmp);
      	    }
      	} else if (iter->equals ("signaturesize")) {	_signature_size = iter->getUnsignedIntContentDefault (0);
      	} else if (iter->equals ("md5sum")) {		_md5sum = iter->getContent();
      	} else if (iter->equals ("importance")) {	_importance = new Importance (iter->getContent());
      	} else if (iter->equals ("description")) {	_description = iter->getContent();
      	} else if (iter->equals ("hid")) {		_hid = iter->getUnsignedIntContentDefault (0);
      	} else if (iter->equals ("license")) {		_license = iter->getContent();
      	}
      
      	iter = iter->next();
          }
      }
      
      
      PackageUpdate::~PackageUpdate()
      {
          if (_package != NULL) _package = NULL;
          if (_package_url != NULL) free ((void *)_package_url);
          if (_signature_url != NULL) free ((void *)_signature_url);
          if (_md5sum != NULL) free ((void *)_md5sum);
          if (_description != NULL) free ((void *)_description);
          if (_license != NULL) free ((void *)_license);
          if (_parent != NULL) _parent = NULL;
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

