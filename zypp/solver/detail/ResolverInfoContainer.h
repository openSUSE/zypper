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

#include <iosfwd>
#include <list>
#include <string>
#include "zypp/solver/detail/ResolverInfoContainerPtr.h"
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

    CResItemList _resItem_list;

  protected:

    ResolverInfoContainer (ResolverInfoType type, ResItem_constPtr resItem, int priority, ResItem_constPtr child = NULL);

  public:
    virtual ~ResolverInfoContainer();

    void copy (ResolverInfoContainer_constPtr from);

    // ---------------------------------- I/O

    static std::string toString (const ResolverInfoContainer & context);
    virtual std::ostream & dumpOn(std::ostream & str ) const;
    friend std::ostream& operator<<(std::ostream&, const ResolverInfoContainer & context);
    std::string asString (void ) const;

    // ---------------------------------- accessors

    CResItemList resItems (void) const { return _resItem_list; }

    // ---------------------------------- methods

    virtual bool merge (ResolverInfoContainer_Ptr to_be_merged);
    virtual ResolverInfo_Ptr copy (void) const;

    std::string resItemsToString (const bool names_only,
				  const bool shorten_output = false) const;

    bool mentions (ResItem_constPtr resItem) const;
    void addRelatedResItem (ResItem_constPtr resItem);
    void addRelatedResItemList (const CResItemList & resItems);

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
