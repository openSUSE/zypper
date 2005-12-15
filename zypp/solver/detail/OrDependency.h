/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* OrDependency.h
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

#ifndef _OrDependency_h
#define _OrDependency_h

#include <list>
#include <map>
#include <iosfwd>
#include <string.h>

#include <y2util/Ustring.h>

#include <zypp/solver/detail/OrDependencyPtr.h>
#include <zypp/solver/detail/Dependency.h>
#include <zypp/solver/detail/XmlNode.h>

///////////////////////////////////////////////////////////////////
namespace zypp {
//////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : OrDependency
/**
 *
 **/

class OrDependency : public CountedRep {
    REP_BODY(OrDependency);

  private:
    typedef std::map<std::string, OrDependencyPtr> OrDependencyTable;

    static OrDependencyTable _or_dep_table;

    std::string _or_dep;
    CDependencyList _split_ors;
    CDependencyList _created_provides;
    int _ref;

    OrDependency (std::string & dep, const CDependencyList & split_ors);

    static std::string dependencyListToString (const CDependencyList & deplist);
    static CDependencyList stringToDependencyList (const char *s);

    void incRef() { _ref++; }
    void decRef() { _ref--; }

  public:

    OrDependency (constXmlNodePtr node);

    virtual ~OrDependency();

    // ---------------------------------- I/O

    const xmlNodePtr asXmlNode (void) const;

    static std::string toString ( const OrDependency & ordep );

    virtual std::ostream & dumpOn( std::ostream & str ) const;

    friend std::ostream& operator<<( std::ostream&, const OrDependency & ordep );

    std::string asString ( void ) const;

    // ---------------------------------- accessors

    const char *name (void) const { return _or_dep.c_str(); }
    void addCreatedProvide (constDependencyPtr dep);

    // ---------------------------------- methods

    static OrDependencyPtr fromDependencyList (const CDependencyList & deplist);
    static OrDependencyPtr fromString (const char *dep);
    static constDependencyPtr find (const char *dep);
};

///////////////////////////////////////////////////////////////////
}; // namespace zypp
///////////////////////////////////////////////////////////////////

#endif // _OrDependency_h
