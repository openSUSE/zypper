/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
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

#ifndef ZYPP_SOLVER_DETAIL_RESITEMANDDEPENDENCY_H
#define ZYPP_SOLVER_DETAIL_RESITEMANDDEPENDENCY_H

#include <iosfwd>
#include <string>
#include <list>
#include <map>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/solver/detail/ResItemAndDependencyPtr.h"
#include "zypp/solver/detail/ResItem.h"
#include "zypp/Capability.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

typedef std::multimap<const std::string, ResItem_constPtr> ResItemTable;
      typedef std::multimap<const std::string, ResItemAndDependency_constPtr> ResItemAndDependencyTable;

      #if PHI
      typedef std::list <ResItemAndDependency_constPtr> CResItemAndDependencyList;
      #endif

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : ResItemAndDependency

      class ResItemAndDependency: public base::ReferenceCounted, private base::NonCopyable {
          

        private:
          ResItem_constPtr _resItem;
          const Capability _dependency;

        public:

          ResItemAndDependency (ResItem_constPtr resItem, const Capability & dependency);
          ~ResItemAndDependency () {}

          // ---------------------------------- I/O

          static std::string toString (const ResItemAndDependency & r_and_d, bool full = false);

          virtual std::ostream & dumpOn(std::ostream & str ) const;

          friend std::ostream& operator<<(std::ostream&, const ResItemAndDependency & r_and_d);

          std::string asString (bool full = false) const;

          // ---------------------------------- accessors

          ResItem_constPtr resItem() const { return _resItem; }
          const Capability & dependency() const { return _dependency; }

          // ---------------------------------- methods

          bool verifyRelation (const Capability & dep) const;
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

#endif // ZYPP_SOLVER_DETAIL_RESITEMANDDEPENDENCY_H
