/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _XmlNode_h
#define _XmlNode_h

#include <list>

#include <iosfwd>
#include <string>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include <zypp/solver/detail/XmlNodePtr.h>

///////////////////////////////////////////////////////////////////
namespace zypp {
//////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//      CLASS NAME : XmlNode

class XmlNode : public CountedRep 
{
    REP_BODY(XmlNode);

  private:
    const xmlNodePtr _node;

  public:
    XmlNode (const xmlNodePtr node);
    XmlNode (const char *name);
    virtual ~XmlNode ();

    // ---------------------------------- I/O

    static std::string toString ( const XmlNode & node );

    virtual std::ostream & dumpOn( std::ostream & str ) const;

    friend std::ostream& operator<<( std::ostream&, const XmlNode & );

    std::string asString ( void ) const;

    // ---------------------------------- accessors

    const char *name() const { return ((const char *)(_node->name)); }
    xmlNodePtr node() const { return (_node); }
    XmlNodePtr next() const { return (_node->next == NULL ? NULL : new XmlNode (_node->next)); }
    XmlNodePtr children() const { return (_node->xmlChildrenNode == NULL ? NULL : new XmlNode (_node->xmlChildrenNode)); }
    xmlElementType type() const { return (_node->type); }

    // ---------------------------------- methods

    const char *getProp (const char *name, const char *deflt = "") const;
    const char *getValue (const char *name, const char *deflt = "") const;
    const char *getContent (void) const;

    bool equals (const char *n) const { return (strcasecmp (name(), n) == 0); }
    bool isElement (void) const { return (type() == XML_ELEMENT_NODE); }

    bool match (const char *str) const { return (! strcasecmp ((const char *)(_node->name), str)); }

    const XmlNodePtr getNode (const char *name) const;

    // The former will get either a property or a tag, whereas the latter will
    //   get only a property

    bool getIntValue (const char *name, int *value) const;
    int getIntValueDefault (const char *name, int def) const;

    bool getUnsignedIntValue (const char *name, unsigned int *value) const;
    unsigned int getUnsignedIntValueDefault (const char *name, unsigned int def) const;

    unsigned int getUnsignedIntPropDefault (const char *name, unsigned int def) const;

    unsigned int getUnsignedIntContentDefault (unsigned int def) const;

    void addTextChild (const char *name, const char *content);
};

///////////////////////////////////////////////////////////////////
}; // namespace zypp
///////////////////////////////////////////////////////////////////


#endif  // _XmlNode_h
