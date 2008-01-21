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

#include "zypp/Capabilities.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"
#include "zypp/VendorAttr.h"
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


class LookFor : public resfilter::PoolItemFilterFunctor
{
  public:
    PoolItem_Ref item;

    bool operator()( PoolItem_Ref provider )
    {
	item = provider;
	return false;				// stop here, we found it
    }
};


// just find installed item with same kind/name as item

PoolItem_Ref
Helper::findInstalledByNameAndKind (const ResPool & pool, const string & name, const Resolvable::Kind & kind)
{
    LookFor info;

    invokeOnEach( pool.byNameBegin( name ),
		  pool.byNameEnd( name ),
		  functor::chain (resfilter::ByInstalled (),			// ByInstalled
				  resfilter::ByKind( kind ) ),			// equal kind
		  functor::functorRef<bool,PoolItem> (info) );

    _XDEBUG("Helper::findInstalledByNameAndKind (" << name << ", " << kind << ") => " << info.item);
    return info.item;
}


// just find uninstalled item with same kind/name as item

PoolItem_Ref
Helper::findUninstalledByNameAndKind (const ResPool & pool, const string & name, const Resolvable::Kind & kind)
{
    LookFor info;

    invokeOnEach( pool.byNameBegin( name ),
		  pool.byNameEnd( name ),
		  functor::chain (resfilter::ByUninstalled (),			// ByUninstalled
				  resfilter::ByKind( kind ) ),			// equal kind
		  functor::functorRef<bool,PoolItem> (info) );

    _XDEBUG("Helper::findUninstalledByNameAndKind (" << name << ", " << kind << ") => " << info.item);
    return info.item;
}


// just find installed item with same kind/name as item
// does *NOT* check edition

PoolItem_Ref
Helper::findInstalledItem (const ResPool & pool, PoolItem_Ref item)
{
    return findInstalledByNameAndKind (pool, item->name(), item->kind() );
}

//----------------------------------------------------------------------------

class LookForUpdate : public resfilter::PoolItemFilterFunctor
{
  public:
    PoolItem_Ref uninstalled;
    PoolItem_Ref installed;

    bool operator()( PoolItem_Ref provider )
    {
        // is valid
        if ( ! provider.resolvable() )
        {
          WAR << "Warning: '" << provider << "' not valid" << endl;
          return true;
        }

        if ( installed.resolvable() )
        {
          if ( !VendorAttr::instance().equivalent(installed->vendor(),provider->vendor()) )
          {
            MIL << "Discarding '" << provider << "' from vendor '"
                << provider->vendor() << "' different to uninstalled '"
                << installed->vendor() << "' vendor." << endl;
            return true;
          }
        }
        
	if ((!uninstalled							// none yet
	    || (uninstalled->edition().compare( provider->edition() ) < 0)	// or a better edition
	    || (uninstalled->arch().compare( provider->arch() ) < 0) ) // or a better architecture
	    && !provider.status().isLocked() )                                  // is not locked
	{
	    uninstalled = provider;						// store 
	}
	return true;
    }
};


// just find best (according to edition) uninstalled item with same kind/name as item
// *DOES* check edition

PoolItem_Ref
Helper::findUpdateItem (const ResPool & pool, PoolItem_Ref item)
{
    LookForUpdate info;
    info.installed = item;

    invokeOnEach( pool.byNameBegin( item->name() ),
		  pool.byNameEnd( item->name() ),
		  functor::chain (functor::chain (resfilter::ByUninstalled (),			// ByUninstalled
						  resfilter::ByKind( item->kind() ) ),		// equal kind
				  resfilter::byEdition<CompareByGT<Edition> >( item->edition() )),	// only look at better editions
		  functor::functorRef<bool,PoolItem> (info) );

    _XDEBUG("Helper::findUpdateItem(" << item << ") => " << info.uninstalled);
    return info.uninstalled;
}


//----------------------------------------------------------------------------

class LookForReinstall : public resfilter::PoolItemFilterFunctor
{
  public:
    PoolItem_Ref uninstalled;

    bool operator()( PoolItem_Ref provider )
    {
	if (provider.status().isLocked()) {
	    return true; // search next
	} else {
	    uninstalled = provider;
	    return false;				// stop here, we found it
	}
    }
};


PoolItem_Ref
Helper::findReinstallItem (const ResPool & pool, PoolItem_Ref item)
{
    LookForReinstall info;

    invokeOnEach( pool.byNameBegin( item->name() ),
		  pool.byNameEnd( item->name() ),
		  functor::chain (functor::chain (resfilter::ByUninstalled (),			// ByUninstalled
						  resfilter::ByKind( item->kind() ) ),		// equal kind
				  resfilter::byEdition<CompareByEQ<Edition> >( item->edition() )),
		  functor::functorRef<bool,PoolItem> (info) );

    _XDEBUG("Helper::findReinstallItem(" << item << ") => " << info.uninstalled);
    return info.uninstalled;
}

//----------------------------------------------------------------------------

class CheckIfBest : public resfilter::PoolItemFilterFunctor
{
  public:
    PoolItem_Ref _item;
    bool is_best;

    CheckIfBest( PoolItem_Ref item )
	: _item( item )
	, is_best( true )		// assume we already have the best
    {}

    // check if provider is better. If yes, end the search.

    bool operator()( PoolItem_Ref provider )
    {
	int archcmp = _item->arch().compare( provider->arch() );
	if (((archcmp < 0) 							// provider has a better architecture
	     || ((archcmp == 0)
		 && (_item->edition().compare( provider->edition() ) < 0)))	// or a better edition
	    && !provider.status().isLocked())					// and is not locked
	{
	    is_best = false;
	    return false;
	}
	return true;
    }
};


// check if the given item is the best one of the pool

bool
Helper::isBestUninstalledItem (const ResPool & pool, PoolItem_Ref item)
{
    CheckIfBest info( item );

    invokeOnEach( pool.byNameBegin( item->name() ),
		  pool.byNameEnd( item->name() ),
		  functor::chain( resfilter::ByUninstalled(),			// ByUninstalled
				  resfilter::ByKind( item->kind() ) ),		// equal kind
		  functor::functorRef<bool,PoolItem>( info ) );

    _XDEBUG("Helper::isBestUninstalledItem(" << item << ") => " << info.is_best);
    return info.is_best;
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

