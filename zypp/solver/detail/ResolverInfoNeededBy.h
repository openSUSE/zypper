/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ResolverInfoNeededBy.h
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

#ifndef _ResolverInfoNeededBy_h
#define _ResolverInfoNeededBy_h

#include <zypp/solver/detail/ResolverInfoNeededByPtr.h>
#include <zypp/solver/detail/ResolverInfoContainer.h>

///////////////////////////////////////////////////////////////////
namespace zypp {
//////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : ResolverInfoNeededBy

class ResolverInfoNeededBy : public ResolverInfoContainer {

    REP_BODY(ResolverInfoNeededBy);

  private:

  public:

    ResolverInfoNeededBy (constResItemPtr resItem);
    virtual ~ResolverInfoNeededBy();

    // ---------------------------------- I/O

    static std::string toString (const ResolverInfoNeededBy & context);
    virtual std::ostream & dumpOn(std::ostream & str ) const;
    friend std::ostream& operator<<(std::ostream&, const ResolverInfoNeededBy & context);
    std::string asString (void ) const;

    // ---------------------------------- accessors

    // ---------------------------------- methods

    virtual ResolverInfoPtr copy (void) const;
};
 
///////////////////////////////////////////////////////////////////
}; // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // _ResolverInfoNeededBy_h
 
