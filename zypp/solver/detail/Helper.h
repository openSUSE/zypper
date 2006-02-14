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
#include "zypp/CapSet.h"

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
    static PoolItem_Ref findInstalledByNameAndKind (const ResPool & pool, const std::string & name, const Resolvable::Kind & kind);

    // for name, find uninstalled item which has same name
    static PoolItem_Ref findUninstalledByNameAndKind (const ResPool & pool, const std::string & name, const Resolvable::Kind & kind);

    // for item, find installed item which has same name
    // does *NOT* check edition
    //  FIXME: should probably take provides/obsoletes into account for
    //	       renamed upgrades
    static PoolItem_Ref findInstalledItem (const ResPool & pool, PoolItem_Ref item);

    // for item, find uninstalled item which has same name and higher edition
    static PoolItem_Ref findUninstalledItem (const ResPool & pool, PoolItem_Ref item);

    // for item, find uninstalled item which has same name and equal edition
    static PoolItem_Ref findReinstallItem (const ResPool & pool, PoolItem_Ref item);

    static PoolItem_Ref findUpdateItem (const ResPool & pool, PoolItem_Ref item);

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
