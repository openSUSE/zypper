/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* ResolverInfoDependsOn.cc
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
#include "zypp/solver/detail/ResolverInfoDependsOn.h"
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

IMPL_PTR_TYPE(ResolverInfoDependsOn);

//---------------------------------------------------------------------------

std::ostream &
ResolverInfoDependsOn::dumpOn( std::ostream & os ) const
{
    ResolverInfo::dumpOn (os);

    ostringstream affected_str;
    affected_str << ResolverInfo::toString (affected());

    // Translator: all.%s = name of package,patch,....
    os << str::form (_("%s depended on %s"),
			affected_str.str().c_str(),
			itemsToString(false).c_str());
    return os;
}

string
ResolverInfoDependsOn::message( ) const
{
    string affected_str = ResolverInfo::toString(affected());
    string container_str = itemsToString( false );

    // TranslatorExplanation: 1.%s name of package, 2.%s list of names
    // TranslatorExplanation: 1.%s is dependent on list of names
    return str::form (_("%s dependend on %s"),
			affected_str.c_str(),
			container_str.c_str());
}

//---------------------------------------------------------------------------

ResolverInfoDependsOn::ResolverInfoDependsOn (PoolItem_Ref item, PoolItem_Ref on)
    : ResolverInfoContainer (RESOLVER_INFO_TYPE_DEPENDS_ON, item, RESOLVER_INFO_PRIORITY_USER, on)
{
}


ResolverInfoDependsOn::~ResolverInfoDependsOn ()
{
}

//---------------------------------------------------------------------------

ResolverInfo_Ptr
ResolverInfoDependsOn::copy (void) const
{
    ResolverInfoDependsOn_Ptr cpy = new ResolverInfoDependsOn(affected(), PoolItem_Ref());

    ((ResolverInfoContainer_Ptr)cpy)->copy (this);

    return cpy;
}

//---------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////
    };// namespace detail
    /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
  };// namespace solver
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
};// namespace zypp
/////////////////////////////////////////////////////////////////////////

