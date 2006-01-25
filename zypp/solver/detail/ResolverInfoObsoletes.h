/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverInfoObsoletes.h
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

#ifndef ZYPP_SOLVER_DETAIL_RESOLVERINFOOBSOLETES_H
#define ZYPP_SOLVER_DETAIL_RESOLVERINFOOBSOLETES_H

#include "zypp/solver/detail/ResolverInfoObsoletesPtr.h"
#include "zypp/solver/detail/ResolverInfoContainer.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp 
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : ResolverInfoObsoletes

class ResolverInfoObsoletes : public ResolverInfoContainer {

  private:

  public:

    ResolverInfoObsoletes (PoolItem resItem, PoolItem obsoletes);
    virtual ~ResolverInfoObsoletes();

    // ---------------------------------- I/O

    friend std::ostream& operator<<(std::ostream&, const ResolverInfoObsoletes & context);

    // ---------------------------------- accessors

    // ---------------------------------- methods

    virtual ResolverInfo_Ptr copy (void) const;
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
#endif // ZYPP_SOLVER_DETAIL_RESOLVERINFOOBSOLETES_H
 
