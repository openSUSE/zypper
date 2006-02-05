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

#include "zypp/CapSet.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"

#include "zypp/base/Algorithm.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////
namespace zypp
{ ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  namespace solver
  { /////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////
    namespace detail
    { ///////////////////////////////////////////////////////////////////

ostream &
operator<< (ostream & os, const PoolItemList & itemlist)
{
    for (PoolItemList::const_iterator iter = itemlist.begin(); iter != itemlist.end(); ++iter) {
	if (iter != itemlist.begin())
	    os << ", ";
	os << *iter;
    }
    return os;
}


class LookForInstalled : public resfilter::OnCapMatchCallbackFunctor, public resfilter::PoolItemFilterFunctor
{
  public:
    PoolItem_Ref installed;

    bool operator()( PoolItem_Ref provider )
    {
	installed = provider;
	return false;				// stop here, we found it
    }
};


// just find installed item with same kind/name as item

PoolItem_Ref
Helper::findInstalledByNameAndKind (const ResPool & pool, const string & name, const Resolvable::Kind & kind)
{
    LookForInstalled info;
#if 0
    invokeOnEach( pool.byNameBegin( name ),
		  pool.byNameEnd( name ),
		  functor::chain (resfilter::ByInstalled (),			// ByInstalled
				  resfilter::ByKind( kind ) ),			// equal kind
		  functor::functorRef<bool,PoolItem> (info) );
#endif
	ResPool::const_nameiterator pend = pool.nameend(name);
	for (ResPool::const_nameiterator it = pool.namebegin(name); it != pend; ++it) {
	    PoolItem item = it->second;
	    if (item.status().isInstalled()
		&& item->kind() == kind) {
		if (!info( it->second ))
		    break;
	    }
	}

    _DEBUG("Helper::findInstalledByNameAndKind (" << name << ", " << kind << ") => " << info.installed);
    return info.installed;
}


// just find installed item with same kind/name as item
// does *NOT* check edition

PoolItem_Ref
Helper::findInstalledItem (const ResPool & pool, PoolItem_Ref item)
{
    return findInstalledByNameAndKind (pool, item->name(), item->kind() );
}

//----------------------------------------------------------------------------

class LookForUpdate : public resfilter::OnCapMatchCallbackFunctor, public resfilter::PoolItemFilterFunctor
{
  public:
    PoolItem_Ref uninstalled;

    bool operator()( PoolItem_Ref provider )
    {
	uninstalled = provider;
	return false;				// stop here, we found it
    }
};


// just find installed item with same kind/name as item
// *DOES* check edition

PoolItem_Ref
Helper::findUpdateItem (const ResPool & pool, PoolItem_Ref item)
{
    LookForUpdate info;
#warning FIXME, should not report locked update candidates.
#if 0
    invokeOnEach( pool.byNameBegin( item->name() ),
		  pool.byNameEnd( item->name() ),
		  functor::chain (functor::chain (resfilter::ByUninstalled (),			// ByUninstalled
						  resfilter::ByKind( item->kind() ) ),		// equal kind
				  resfilter::byEdition<CompareByGT<Edition> >( item->edition() )),
		  functor::functorRef<bool,PoolItem> (info) );
#endif
	ResPool::const_nameiterator pend = pool.nameend(item->name());
	for (ResPool::const_nameiterator it = pool.namebegin(item->name()); it != pend; ++it) {
	    PoolItem pos = it->second;
	    if (pos.status().isUninstalled()
		&& pos->kind() == item->kind()
		&& item->edition().compare(pos->edition()) < 0)
	    {
		if (!info( pos ))
		    break;
	    }
	}

    _DEBUG("Helper::findUpdateItem(" << item << ") => " << info.uninstalled);
    return info.uninstalled;
}


//----------------------------------------------------------------------------

class LookForReinstall : public resfilter::OnCapMatchCallbackFunctor, public resfilter::PoolItemFilterFunctor
{
  public:
    PoolItem_Ref uninstalled;

    bool operator()( PoolItem_Ref provider )
    {
	uninstalled = provider;
	return false;				// stop here, we found it
    }
};


PoolItem_Ref
Helper::findReinstallItem (const ResPool & pool, PoolItem_Ref item)
{
    LookForReinstall info;
#warning FIXME, should not report locked update candidates.
#if 0
    invokeOnEach( pool.byNameBegin( item->name() ),
		  pool.byNameEnd( item->name() ),
		  functor::chain (functor::chain (resfilter::ByUninstalled (),			// ByUninstalled
						  resfilter::ByKind( item->kind() ) ),		// equal kind
				  resfilter::byEdition<CompareByEQ<Edition> >( item->edition() )),
		  functor::functorRef<bool,PoolItem> (info) );
#endif
	ResPool::const_nameiterator pend = pool.nameend(item->name());
	for (ResPool::const_nameiterator it = pool.namebegin(item->name()); it != pend; ++it) {
	    PoolItem pos = it->second;
	    if (pos.status().isUninstalled()
		&& pos->kind() == item->kind()
		&& item->edition().compare(pos->edition()) == 0)
	    {
		if (!info( pos ))
		    break;
	    }
	}

    _DEBUG("Helper::findReinstallItem(" << item << ") => " << info.uninstalled);
    return info.uninstalled;
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

