/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverInfoMissingReq.cc
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

#include <map>
#include <sstream>

#include "zypp/solver/detail/ResolverInfo.h"
#include "zypp/solver/detail/ResolverInfoMissingReq.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp 
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

using namespace std;

IMPL_PTR_TYPE(ResolverInfoMissingReq);

//---------------------------------------------------------------------------


std::ostream &
ResolverInfoMissingReq::dumpOn( std::ostream & os ) const
{
    ResolverInfo::dumpOn (os);

    ostringstream affected_str;
    affected_str << ResolverInfo::toString (affected());

    ostringstream mis_str;
    mis_str << _missing;

    // Translator: 1.%s = name of package,patch,...; 2.%s = dependency
    os << str::form (_("%s is missing the requirement %s"),
			    affected_str.str().c_str(),
			    mis_str.str().c_str());
    return os;
}

//---------------------------------------------------------------------------

ResolverInfoMissingReq::ResolverInfoMissingReq (PoolItem_Ref item, const Capability & missing)
    : ResolverInfo (RESOLVER_INFO_TYPE_MISSING_REQ, item, RESOLVER_INFO_PRIORITY_USER)
    , _missing (missing)
{
}


ResolverInfoMissingReq::~ResolverInfoMissingReq ()
{
}

//---------------------------------------------------------------------------

ResolverInfo_Ptr
ResolverInfoMissingReq::copy (void) const
{
    ResolverInfoMissingReq_Ptr cpy = new ResolverInfoMissingReq(affected(), _missing);

    ((ResolverInfo_Ptr)cpy)->copy (this);

    return cpy;
}
///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////      


