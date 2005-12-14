/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* Dependency.h
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

#ifndef _Dependency_h
#define _Dependency_h

#include <list>
#include <iosfwd>
#include <string.h>

#include <y2util/Ustring.h>

#include <zypp/solver/detail/Channel.h>
#include <zypp/solver/detail/DependencyPtr.h>
#include <zypp/solver/detail/OrDependencyPtr.h>
#include <zypp/solver/detail/Spec.h>
#include <zypp/solver/detail/XmlNode.h>

///////////////////////////////////////////////////////////////////
namespace ZYPP {
//////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//      CLASS NAME : Relation
/**
 * A dependency relation
 **/

class Relation {

  private:

    int _op;

    Relation (int op) { _op = op; }

  public:

    virtual ~Relation() {}

    // ---------------------------------- I/O

    static std::string toString ( const Relation & relation );

    static std::string toWord ( const Relation & relation );

    virtual std::ostream & dumpOn( std::ostream & str ) const;

    friend std::ostream& operator<<( std::ostream & str, const Relation & relation );

    std::string asString ( void ) const;

    // ---------------------------------- consts

    static const Relation & Invalid;
    static const Relation & Any;
    static const Relation & Equal;
    static const Relation & NotEqual;
    static const Relation & Less;
    static const Relation & LessEqual;
    static const Relation & Greater;
    static const Relation & GreaterEqual;
    static const Relation & None;

    static const Relation & parse (const char *relation);

    // ---------------------------------- accessors

    int op (void) const { return _op; }

    bool isEqual () const;

    // equality operator
    bool operator==( const Relation & rel ) const {
	return (_op == rel.op());
    }

    // inequality
    bool operator!=( const Relation & rel ) const {
	return !(*this == rel);
    }

};

///////////////////////////////////////////////////////////////////

typedef std::list <DependencyPtr> DependencyList;
typedef std::list <constDependencyPtr> CDependencyList;

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : Dependency
/**
 *
 **/

class Dependency : public Spec {
    REP_BODY(Dependency);

  private:
    Relation _relation;
    constChannelPtr _channel;
    bool _or_dep;
    bool _pre_dep;

  public:

    Dependency (const std::string & name,
	  const Relation & relation,
	  const Kind & kind = Kind::Package,
	  constChannelPtr channel = NULL,
	  int epoch = -1,
	  const std::string & version = "",
	  const std::string & release = "",
	  const Arch * arch = Arch::Any,
	  bool or_dep = false,
	  bool pre_dep = false);

    Dependency (const std::string & name,
	  const Relation & relation,
	  const Kind & kind = Kind::Package,
	  constChannelPtr channel = NULL,
	  constEditionPtr edition = NULL,
	  bool or_dep = false,
	  bool pre_dep = false);

    Dependency (OrDependencyPtr or_dep);	//RCResItemDep *rc_dep_or_new_provide (RCDepOr *dor);

    Dependency (constXmlNodePtr node);		//RCResItemDep *rc_xml_node_to_resItem_dep (const xmlNode *node);

    virtual ~Dependency();

    // ---------------------------------- I/O

    const xmlNodePtr asXmlNode (void) const;		// xmlNode *rc_resItem_dep_to_xml_node (RCResItemDep *dep_item);

    static std::string toString ( const Dependency & dep );

    static std::string toString ( const CDependencyList & deplist );

    virtual std::ostream & dumpOn( std::ostream & str ) const;

    friend std::ostream& operator<<( std::ostream & str, const Dependency & dep);

    std::string asString ( void ) const;

    // ---------------------------------- accessors

    const Relation & relation() const { return _relation; }
    constChannelPtr channel (void) const { return _channel; }

    bool orDep (void) const { return _or_dep; }
    void setOrDep (bool or_dep) { _or_dep = or_dep; }

    bool preDep (void) const { return _pre_dep; }
    void setPreDep (bool pre_dep) { _pre_dep = pre_dep; }

    // ---------------------------------- methods

    DependencyPtr parseXml (constXmlNodePtr node);
    bool verifyRelation (constDependencyPtr prov) const;
};

// xmlNode *rc_resItem_dep_or_slist_to_xml_node (RCResItemDepSList *dep);

///////////////////////////////////////////////////////////////////
}; // namespace ZYPP
///////////////////////////////////////////////////////////////////

#endif // _Dependency_h
