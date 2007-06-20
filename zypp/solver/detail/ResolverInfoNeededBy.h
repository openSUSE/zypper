/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
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

#ifndef ZYPP_SOLVER_DETAIL_RESOLVERINFONEEDEDBY_H
#define ZYPP_SOLVER_DETAIL_RESOLVERINFONEEDEDBY_H

#include "zypp/solver/detail/Types.h"
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
//	CLASS NAME : ResolverInfoNeededBy

class ResolverInfoNeededBy : public ResolverInfoContainer {

    

  private:
    Capability _cap;
    Dep _capKind;

  public:

    ResolverInfoNeededBy (PoolItem_Ref item);
    virtual ~ResolverInfoNeededBy();

    // ---------------------------------- I/O

    virtual std::ostream & dumpOn( std::ostream & str ) const;
    friend std::ostream& operator<<(std::ostream & str, const ResolverInfoNeededBy & obj)
    { return obj.dumpOn (str); }

    // ---------------------------------- accessors

    // ---------------------------------- methods

    virtual std::string message (void) const;
    virtual ResolverInfo_Ptr copy (void) const;
    void setCapability (const Capability & cap, const Dep & capKind) { _cap = cap; _capKind = capKind; }
;
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
#endif // ZYPP_SOLVER_DETAIL_RESOLVERINFONEEDEDBY_H
 
