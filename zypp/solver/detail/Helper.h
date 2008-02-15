/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* Helper.h
 *
 * Static helpers
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

#ifndef ZYPP_SOLVER_DETAIL_HELPER_H
#define ZYPP_SOLVER_DETAIL_HELPER_H

#include <iosfwd>

#include "zypp/ResPool.h"
#include "zypp/PoolItem.h"
#include "zypp/Capabilities.h"
#include "zypp/base/String.h"
#include "zypp/solver/detail/Types.h"

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
//	CLASS NAME : Helper

class Helper {
  public:

    // for name, find installed item which has same name
    // does *NOT* check edition
    //  FIXME: should probably take provides/obsoletes into account for
    //	       renamed upgrades
    static PoolItem findInstalledByNameAndKind (const ResPool & pool, const std::string & name, const Resolvable::Kind & kind);

    // for name, find uninstalled item which has same name
    static PoolItem findUninstalledByNameAndKind (const ResPool & pool, const std::string & name, const Resolvable::Kind & kind);

    // for item, find installed item which has same name -> calls findInstalledByNameAndKind()
    // does *NOT* check edition
    //  FIXME: should probably take provides/obsoletes into account for
    //	       renamed upgrades
    static PoolItem findInstalledItem (const ResPool & pool, PoolItem item);
    /** \overload Using ident cache entry. */
    static PoolItem findInstalledItem (const std::vector<PoolItem> & pool, PoolItem item);

    // for item, find uninstalled item which has same name and higher edition
    static PoolItem findUninstalledItem (const ResPool & pool, PoolItem item);

    // for item, find uninstalled item which has same name and equal edition
    static PoolItem findReinstallItem (const ResPool & pool, PoolItem item);

    static PoolItem findUpdateItem (const ResPool & pool, PoolItem item);
    /** \overload Using ident cache entry. */
    static PoolItem findUpdateItem (const std::vector<PoolItem> & pool, PoolItem item);

    // for item, check if this is the 'best' uninstalled (best arch, best version) item
    static bool isBestUninstalledItem (const ResPool & pool, PoolItem item);

    // Human readable item
    static std::string itemToString (PoolItem item, bool shortVersion=false);
    static std::string capToString (const Capability & capability);

    friend std::ostream& operator<<(std::ostream&, const PoolItemList & itemlist);

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

#endif // ZYPP_SOLVER_DETAIL_HELPER_H
