/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverInfoContainer.h
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

#ifndef ZYPP_SOLVER_DETAIL_RESOLVERINFOCONTAINER_H
#define ZYPP_SOLVER_DETAIL_RESOLVERINFOCONTAINER_H

#include "zypp/solver/detail/Types.h"
#include "zypp/solver/detail/ResolverInfo.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//
//	CLASS NAME : ResolverInfoContainer

class ResolverInfoContainer : public ResolverInfo {

  private:

    CPoolItemList _item_list;

  protected:

    ResolverInfoContainer (ResolverInfoType type, PoolItem initial_item, int priority, PoolItem child = NULL);

  public:
    virtual ~ResolverInfoContainer();

    void copy (ResolverInfoContainer_constPtr from);

    // ---------------------------------- I/O

    friend std::ostream& operator<<(std::ostream&, const ResolverInfoContainer & context);

    // ---------------------------------- accessors

    CPoolItemList items (void) const { return _item_list; }

    // ---------------------------------- methods

    virtual bool merge (ResolverInfoContainer_Ptr to_be_merged);
    virtual ResolverInfo_Ptr copy (void) const;

    std::string itemsToString (const bool names_only,
				const bool shorten_output = false) const;

    bool mentions (PoolItem item) const;
    void addRelatedPoolItem (PoolItem item);
    void addRelatedPoolItemList (const CPoolItemList & items);

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
#endif // ZYPP_SOLVER_DETAIL_RESOLVERINFOCONTAINER_H
