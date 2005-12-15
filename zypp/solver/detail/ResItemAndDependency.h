/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ResItemAndDependency.h
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

#ifndef _ResItemAndDependency_h
#define _ResItemAndDependency_h

#include <iosfwd>
#include <string>
#include <list>
#include <map>

#include <zypp/solver/detail/ResItemAndDependencyPtr.h>
#include <zypp/solver/detail/ResItem.h>
#include <zypp/solver/detail/Dependency.h>

///////////////////////////////////////////////////////////////////
namespace zypp {
//////////////////////////////////////////////////////////////////

typedef std::multimap<const std::string, constResItemPtr> ResItemTable;
typedef std::multimap<const std::string, constResItemAndDependencyPtr> ResItemAndDependencyTable;

#if PHI
typedef std::list <constResItemAndDependencyPtr> CResItemAndDependencyList;
#endif

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : ResItemAndDependency

class ResItemAndDependency: public CountedRep {
    REP_BODY(ResItemAndDependency);

  private:
    constResItemPtr _resItem;
    constDependencyPtr _dependency;

  public:

    ResItemAndDependency (constResItemPtr resItem, constDependencyPtr dependency);
    ~ResItemAndDependency () {}

    // ---------------------------------- I/O

    static std::string toString (const ResItemAndDependency & r_and_d, bool full = false);

    virtual std::ostream & dumpOn(std::ostream & str ) const;

    friend std::ostream& operator<<(std::ostream&, const ResItemAndDependency & r_and_d);

    std::string asString (bool full = false) const;

    // ---------------------------------- accessors

    constResItemPtr resItem() const { return _resItem; }
    constDependencyPtr dependency() const { return _dependency; }

    // ---------------------------------- methods

    bool verifyRelation (constDependencyPtr dep) const;
};
    
///////////////////////////////////////////////////////////////////
}; // namespace zypp
///////////////////////////////////////////////////////////////////

#endif // _ResItemAndDependency_h
