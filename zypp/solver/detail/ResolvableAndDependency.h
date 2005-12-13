/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ResolvableAndDependency.h
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

#ifndef _ResolvableAndDependency_h
#define _ResolvableAndDependency_h

#include <iosfwd>
#include <string>
#include <list>
#include <map>

#include <zypp/solver/detail/ResolvableAndDependencyPtr.h>
#include <zypp/solver/detail/Resolvable.h>
#include <zypp/solver/detail/Dependency.h>

///////////////////////////////////////////////////////////////////
namespace ZYPP {
//////////////////////////////////////////////////////////////////

typedef std::multimap<const std::string, constResolvablePtr> ResolvableTable;
typedef std::multimap<const std::string, constResolvableAndDependencyPtr> ResolvableAndDependencyTable;

#if PHI
typedef std::list <constResolvableAndDependencyPtr> CResolvableAndDependencyList;
#endif

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : ResolvableAndDependency

class ResolvableAndDependency: public CountedRep {
    REP_BODY(ResolvableAndDependency);

  private:
    constResolvablePtr _resolvable;
    constDependencyPtr _dependency;

  public:

    ResolvableAndDependency (constResolvablePtr resolvable, constDependencyPtr dependency);
    ~ResolvableAndDependency () {}

    // ---------------------------------- I/O

    static std::string toString (const ResolvableAndDependency & r_and_d, bool full = false);

    virtual std::ostream & dumpOn(std::ostream & str ) const;

    friend std::ostream& operator<<(std::ostream&, const ResolvableAndDependency & r_and_d);

    std::string asString (bool full = false) const;

    // ---------------------------------- accessors

    constResolvablePtr resolvable() const { return _resolvable; }
    constDependencyPtr dependency() const { return _dependency; }

    // ---------------------------------- methods

    bool verifyRelation (constDependencyPtr dep) const;
};
    
///////////////////////////////////////////////////////////////////
}; // namespace ZYPP
///////////////////////////////////////////////////////////////////

#endif // _ResolvableAndDependency_h
