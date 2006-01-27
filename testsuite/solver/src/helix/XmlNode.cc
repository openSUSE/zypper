/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* XmlNode.cc  wrapper for xmlNodePtr from libxml2
 *
 * Copyright (C) 2000-2002 Ximian, Inc.
 * Copyright (C) 2005 SUSE Linux Products GmbH
 *
 * Definition of 'edition'
 *  contains epoch-version-release-arch
 *  and comparision functions
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

#include "XmlNode.h"
#include "zypp/base/Logger.h"

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

IMPL_PTR_TYPE(XmlNode);

//---------------------------------------------------------------------------

XmlNode::XmlNode (const xmlNodePtr node)
    : _node(node)
{
}

XmlNode::XmlNode (const string & name)
    : _node(xmlNewNode (NULL, (const xmlChar *)name.c_str()))
{
}

XmlNode::~XmlNode ()
{
}

//---------------------------------------------------------------------------

string
XmlNode::asString ( void ) const
{
    return toString (*this);
}


string
XmlNode::toString ( const XmlNode & node )
{
    return "<xmlnode/>";
}


ostream &
XmlNode::dumpOn( ostream & str ) const
{
    str << asString();
    return str;
}


ostream&
operator<<( ostream& os, const XmlNode& node)
{
    return os << node.asString();
}

//---------------------------------------------------------------------------

string 
XmlNode::getValue (const string & name, const string & deflt) const
{
    string ret;
    xmlChar *xml_s;
    xmlNode *child;

    xml_s = xmlGetProp(_node, (const xmlChar *)name.c_str());
//	  _XXX("RC_SPEW_XML") << "XmlNode::getValue(" << name << ") xmlGetProp '" << (char *)xml_s << "'" << endl;

    if (xml_s) {
	ret = string ((const char *)xml_s);
	xmlFree (xml_s);
	return ret;
    }

    child = _node->xmlChildrenNode;

    while (child) {
	      _XXX("RC_SPEW_XML") << "XmlNode::getValue(" << name << ") child '" << (child->name) << "'" << endl;
	if (strcasecmp((const char *)(child->name), name.c_str()) == 0) {
	    xml_s = xmlNodeGetContent(child);
	    _XXX("RC_SPEW_XML") << "XmlNode::getValue(" << name << ") xmlNodeGetContent '" << (char *)xml_s << "'" << endl;
	    if (xml_s) {
		ret = string ((const char *)xml_s);
		xmlFree (xml_s);
		return ret;
	    }
	}
	child = child->next;
    }

    _XXX("RC_SPEW_XML") << "XmlNode::getValue(" << name << ") deflt" << endl;
    return deflt;
}


bool
XmlNode::hasProp (const std::string & name) const
{
    xmlChar *ret;

    ret = xmlGetProp (_node, (const xmlChar *)name.c_str());
    if (ret) {
	return true;
    }
    return false;
}


string
XmlNode::getProp (const std::string & name, const std::string & deflt) const
{
    xmlChar *ret;
    string gs;

    ret = xmlGetProp (_node, (const xmlChar *)name.c_str());

    if (ret) {
	_XXX("RC_SPEW_XML") << "XmlNode::getProp(" << name << ") xmlGetProp '" << (char *)ret << "'" << endl;
	
	gs = string ((const char  *)ret);
	xmlFree (ret);
	return gs;
    }
    return deflt;
}


bool
XmlNode::getIntValue (const std::string & name, int *value) const
{
    string strval;
    char *ret;
    long z;

    strval = this->getValue (name);
    if (strval.empty()) {
	return false;
    }

    z = strtol (strval.c_str(), &ret, 10);
    if (*ret != '\0') {
	return false;
    }

    *value = z;
    return true;
}


int
XmlNode::getIntValueDefault (const std::string & name, int def) const
{
    int z;
    if (this->getIntValue (name, &z))
	return z;
    else
	return def;
}

	       
unsigned int
XmlNode::getUnsignedIntValueDefault (const std::string & name, unsigned int def) const
{
    unsigned int z;
    if (this->getUnsignedIntValue (name, &z))
	return z;
    else
	return def;
}


bool
XmlNode::getUnsignedIntValue (const std::string & name, unsigned int *value) const
{
    string strval;
    char *ret;
    int z;

    strval = this->getValue (name);
    if (strval.empty()) {
	return false;
    }

    z = strtoul (strval.c_str(), &ret, 10);
    if (*ret != '\0') {
	return false;
    }

    *value = z;
    return true;
}


unsigned int
XmlNode::getUnsignedIntPropDefault (const std::string & name, unsigned int def) const
{
    xmlChar *buf;
    unsigned int ret;

    buf = xmlGetProp (_node, (const xmlChar *)name.c_str());

    if (buf) {
	ret = strtol ((const char *)buf, NULL, 10);
	xmlFree (buf);
	return (ret);
    }
    return (def);
}


string
XmlNode::getContent (void) const
{
    xmlChar *buf;
    string ret;

    buf = xmlNodeGetContent (_node);

    ret = string ((const char *)buf);

    xmlFree (buf);

    return (ret);
}


unsigned int
XmlNode::getUnsignedIntContentDefault (unsigned int def) const
{
    xmlChar *buf;
    unsigned int ret;

    buf = xmlNodeGetContent (_node);

    if (buf) {
	ret = strtol ((const char *)buf, NULL, 10);
	xmlFree (buf);
	return (ret);
    }
    return (def);
}


const XmlNode_Ptr
XmlNode::getNode (const std::string & name) const
{
    xmlNodePtr iter;

    for (iter = _node->xmlChildrenNode; iter; iter = iter->next) {
	if (strcasecmp ((const char *)(iter->name), name.c_str()) == 0) {
	    return new XmlNode (iter);
	}
    }

    return NULL;
}


//---------------------------------------------------------------------------


void
XmlNode::addTextChild (const std::string & name, const std::string & content)
{
    xmlNewTextChild (_node, NULL, (const xmlChar *)name.c_str(), (const xmlChar *)content.c_str());
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

