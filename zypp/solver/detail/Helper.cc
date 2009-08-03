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
#include <sstream>

#include "zypp/solver/detail/Helper.h"
#include "zypp/Capabilities.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"
#include "zypp/VendorAttr.h"
#include "zypp/base/Algorithm.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/RepoInfo.h"

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
    PoolItem item;

    bool operator()( PoolItem provider )
    {
	item = provider;
	return false;				// stop here, we found it
    }
};


// just find installed item with same kind/name as item

template<class _Iter>
static PoolItem findInstalledByNameAndKind ( _Iter begin, _Iter end, const string & name, const Resolvable::Kind & kind)
{
    LookFor info;

    invokeOnEach(begin, end,
		  resfilter::ByInstalled (),					// ByInstalled
		  functor::functorRef<bool,PoolItem> (info) );

    _XDEBUG("Helper::findInstalledByNameAndKind (" << name << ", " << kind << ") => " << info.item);
    return info.item;

}

PoolItem Helper::findInstalledByNameAndKind (const ResPool & pool, const string & name, const Resolvable::Kind & kind)
{ return detail::findInstalledByNameAndKind( pool.byIdentBegin( kind, name ), pool.byIdentEnd( kind, name ), name, kind ); }

PoolItem Helper::findInstalledItem (const ResPool & pool, PoolItem item)
{ return findInstalledByNameAndKind(pool, item->name(), item->kind() ); }

PoolItem Helper::findInstalledItem( const std::vector<PoolItem> & pool, PoolItem item )
{ return detail::findInstalledByNameAndKind( pool.begin(), pool.end(), item->name(), item->kind() ); }


// just find uninstalled item with same kind/name as item

PoolItem
Helper::findUninstalledByNameAndKind (const ResPool & pool, const string & name, const Resolvable::Kind & kind)
{
    LookFor info;

    invokeOnEach( pool.byIdentBegin( kind, name ),
		  pool.byIdentEnd( kind, name ),
		  resfilter::ByUninstalled(),					// ByUninstalled
		  functor::functorRef<bool,PoolItem> (info) );

    _XDEBUG("Helper::findUninstalledByNameAndKind (" << name << ", " << kind << ") => " << info.item);
    return info.item;
}


//----------------------------------------------------------------------------

class LookForUpdate : public resfilter::PoolItemFilterFunctor
{
  public:
    PoolItem uninstalled;
    PoolItem installed;

    bool operator()( PoolItem provider )
    {
        // is valid
        if ( ! provider.resolvable() )
        {
          WAR << "Warning: '" << provider << "' not valid" << endl;
          return true;
        }

        if ( installed.resolvable() )
        {
          if ( !VendorAttr::instance().equivalent( installed, provider ) )
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

template<class _Iter>
static PoolItem findUpdateItem( _Iter begin, _Iter end, PoolItem item )
{
    LookForUpdate info;
    info.installed = item;

    invokeOnEach( begin, end,
		  functor::chain (resfilter::ByUninstalled (),						// ByUninstalled
				  resfilter::byEdition<CompareByGT<Edition> >( item->edition() )),	// only look at better editions
		  functor::functorRef<bool,PoolItem> (info) );

    _XDEBUG("Helper::findUpdateItem(" << item << ") => " << info.uninstalled);
    return info.uninstalled;
}

PoolItem Helper::findUpdateItem (const ResPool & pool, PoolItem item)
{ return detail::findUpdateItem( pool.byIdentBegin( item ), pool.byIdentEnd( item ), item ); }

PoolItem Helper::findUpdateItem (const std::vector<PoolItem> & pool, PoolItem item)
{ return detail::findUpdateItem( pool.begin(), pool.end(), item ); }


//----------------------------------------------------------------------------

class LookForReinstall : public resfilter::PoolItemFilterFunctor
{
  public:
    PoolItem uninstalled;

    bool operator()( PoolItem provider )
    {
	if (provider.status().isLocked()) {
	    return true; // search next
	} else {
	    uninstalled = provider;
	    return false;				// stop here, we found it
	}
    }
};


PoolItem
Helper::findReinstallItem (const ResPool & pool, PoolItem item)
{
    LookForReinstall info;

    invokeOnEach( pool.byIdentBegin( item ),
		  pool.byIdentEnd( item ),
		  functor::chain (resfilter::ByUninstalled (),						// ByUninstalled
				  resfilter::byEdition<CompareByEQ<Edition> >( item->edition() )),
		  functor::functorRef<bool,PoolItem> (info) );

    _XDEBUG("Helper::findReinstallItem(" << item << ") => " << info.uninstalled);
    return info.uninstalled;
}

//----------------------------------------------------------------------------

class CheckIfBest : public resfilter::PoolItemFilterFunctor
{
  public:
    PoolItem _item;
    bool is_best;

    CheckIfBest( PoolItem item )
	: _item( item )
	, is_best( true )		// assume we already have the best
    {}

    // check if provider is better. If yes, end the search.

    bool operator()( PoolItem provider )
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
Helper::isBestUninstalledItem (const ResPool & pool, PoolItem item)
{
    CheckIfBest info( item );

    invokeOnEach( pool.byIdentBegin( item ),
		  pool.byIdentEnd( item ),
		  resfilter::ByUninstalled(),			// ByUninstalled
		  functor::functorRef<bool,PoolItem>( info ) );

    _XDEBUG("Helper::isBestUninstalledItem(" << item << ") => " << info.is_best);
    return info.is_best;
}

std::string
Helper::itemToString (PoolItem item, bool shortVersion)
{
    ostringstream os;
    if (!item) return "";

    if (item->kind() != ResKind::package)
	os << item->kind() << ':';
    os  << item->name();
    if (!shortVersion) {
	os << '-' << item->edition();
	if (item->arch() != "") {
	    os << '.' << item->arch();
	}

	string alias = item->repoInfo().alias();
	if (!alias.empty()
	    && alias != "@System")
	{
	    os << '[' << alias << ']';
	}
    }
    return os.str();
}

std::string
Helper::capToString (const Capability & capability)
{
    ostringstream os;
    os << capability.asString();
    return os.str();
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

