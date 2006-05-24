/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* XmlNode.h  wrapper for xmlNode* from libxml2
 *
 * Copyright (C) 2000-2003 Ximian, Inc.
 * Copyright (C) 2005 SUSE Linux Products GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation
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

#ifndef ZYPP_SOLVER_TEMPORARY_XMLNODE_H
#define ZYPP_SOLVER_TEMPORARY_XMLNODE_H

#include <list>
#include <iostream>
#include <iosfwd>
#include <string>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

DEFINE_PTR_TYPE(XmlNode);

///////////////////////////////////////////////////////////////////
//
//      CLASS NAME : XmlNode

class XmlNode : public base::ReferenceCounted, private base::NonCopyable
{

  private:
    const xmlNodePtr _node;

  public:
    XmlNode (const xmlNodePtr node);
    XmlNode (const std::string & name);
    virtual ~XmlNode ();

    // ---------------------------------- I/O

    static std::string toString ( const XmlNode & node );

    virtual std::ostream & dumpOn( std::ostream & str ) const;

    friend std::ostream& operator<<( std::ostream&, const XmlNode & );

    std::string asString ( void ) const;

    // ---------------------------------- accessors

    const std::string name() const { return (std::string((const char *)_node->name)); }
    xmlNodePtr node() const { return (_node); }
    XmlNode_Ptr next() const { return (_node->next == NULL ? NULL : new XmlNode (_node->next)); }
    XmlNode_Ptr children() const { return (_node->xmlChildrenNode == NULL ? NULL : new XmlNode (_node->xmlChildrenNode)); }
    xmlElementType type() const { return (_node->type); }

    // ---------------------------------- methods

    bool hasProp (const std::string & name) const;
    std::string getProp (const std::string & name, const std::string & deflt = "") const;
    std::string getValue (const std::string & name, const std::string & deflt = "") const;
    std::string getContent (void) const;

    bool equals (const std::string & n) const { return (strcasecmp (name().c_str(), n.c_str()) == 0); }
    bool isElement (void) const { return (type() == XML_ELEMENT_NODE); }

    const XmlNode_Ptr getNode (const std::string & name) const;

    // The former will get either a property or a tag, whereas the latter will
    //   get only a property

    bool getIntValue (const std::string & name, int *value) const;
    int getIntValueDefault (const std::string & name, int def) const;

    bool getUnsignedIntValue (const std::string & name, unsigned int *value) const;
    unsigned int getUnsignedIntValueDefault (const std::string & name, unsigned int def) const;

    unsigned int getUnsignedIntPropDefault (const std::string & name, unsigned int def) const;

    unsigned int getUnsignedIntContentDefault (unsigned int def) const;

    void addTextChild (const std::string & name, const std::string & content);
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



#endif  // ZYPP_SOLVER_TEMPORARY_XMLNODE_H
