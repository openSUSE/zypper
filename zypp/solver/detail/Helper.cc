/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* Helper.cc
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

#include "zypp/solver/detail/Helper.h"

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

PoolItem
Helper::findInstalledItem (const ResPool *pool, PoolItem_Ref item)
{
    struct LookForUpgrades : public resfilter::OnCapMatchCallbackFunctor
    {
	PoolItem_Ref installed;

	LookForUpgrades ()
	{ }

	bool operator()( PoolItem_Ref  & provider )
	{
	    installed = provider;
	    return false;				// stop here, we found it
	}
    };

    LookForUpgrades info;

    invokeOnEach( pool->byNameBegin( item->name() ), pool->byNameEnd( item->name() ),
		  functor::chain( resfilter::ByStatus ( ResStatus::INSTALLED ),
				  resfilter::ByKind( item->kind() ),
				  resfilter::ByEdition<CompareByLT<Edition> >( item->edition() ),
		  info );

    return info.installed;
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

