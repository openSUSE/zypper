/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* Resolver.cc
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
#include <boost/static_assert.hpp>

#include "zypp/solver/detail/Resolver.h"
#include "zypp/solver/detail/Helper.h"

#include "zypp/Capabilities.h"
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"
#include "zypp/base/Algorithm.h"
#include "zypp/ResPool.h"
#include "zypp/ResFilters.h"
#include "zypp/CapFilters.h"
#include "zypp/sat/Pool.h"
#include "zypp/sat/Solvable.h"
#include "zypp/sat/SATResolver.h"

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

IMPL_PTR_TYPE(Resolver);


//---------------------------------------------------------------------------


std::ostream &
Resolver::dumpOn( std::ostream & os ) const
{
    return os << "<resolver/>";
}


//---------------------------------------------------------------------------

Resolver::Resolver (const ResPool & pool)
    : _pool (pool)
    , _satResolver (NULL)
    , _poolchanged( _pool.serial() )
    , _forceResolve (false)

{

}


Resolver::~Resolver()
{
}

//---------------------------------------------------------------------------

ResPool
Resolver::pool (void) const
{
    return _pool;
}

void
Resolver::reset (bool keepExtras )
{
    _items_to_verify.clear();

    if (!keepExtras) {
      _extra_requires.clear();
      _extra_conflicts.clear();
    }
}


void
Resolver::addPoolItemToVerify (PoolItem_Ref item)
{

    _items_to_verify.push_back (item);
}


void
Resolver::addExtraRequire (const Capability & capability)
{
    _extra_requires.insert (capability);
}

void
Resolver::removeExtraRequire (const Capability & capability)
{
    _extra_requires.erase (capability);
}

void
Resolver::addExtraConflict (const Capability & capability)
{
    _extra_conflicts.insert (capability);
}

void
Resolver::removeExtraConflict (const Capability & capability)
{
    _extra_conflicts.erase (capability);
}

void
Resolver::addIgnoreConflict (const PoolItem_Ref item,
		   const Capability & capability)
{
    _ignoreConflicts.insert(make_pair(item, capability));
}

void
Resolver::addIgnoreRequires (const PoolItem_Ref item,
			     const Capability & capability)
{
    _ignoreRequires.insert(make_pair(item, capability));
}

void
Resolver::addIgnoreObsoletes (const PoolItem_Ref item,
			      const Capability & capability)
{
    _ignoreObsoletes.insert(make_pair(item, capability));
}

void
Resolver::addIgnoreInstalledItem (const PoolItem_Ref item)
{
    _ignoreInstalledItem.push_back (item);
}

void
Resolver::addIgnoreArchitectureItem (const PoolItem_Ref item)
{
    _ignoreArchitectureItem.push_back (item);
}

void
Resolver::addIgnoreVendorItem (const PoolItem_Ref item)
{
    _ignoreVendorItem.push_back (item);
}

//---------------------------------------------------------------------------

struct UndoTransact : public resfilter::PoolItemFilterFunctor
{
    ResStatus::TransactByValue resStatus;
    UndoTransact ( const ResStatus::TransactByValue &status)
	:resStatus(status)
    { }

    bool operator()( PoolItem_Ref item )		// only transacts() items go here
    {
	item.status().resetTransact( resStatus );// clear any solver/establish transactions
	return true;
    }
};


struct DoTransact : public resfilter::PoolItemFilterFunctor
{
    ResStatus::TransactByValue resStatus;
    DoTransact ( const ResStatus::TransactByValue &status)
	:resStatus(status)
    { }

    bool operator()( PoolItem_Ref item )		// only transacts() items go here
    {
	item.status().setTransact( true, resStatus );
	return true;
    }
};


struct VerifySystem : public resfilter::PoolItemFilterFunctor
{
    Resolver & resolver;

    VerifySystem (Resolver & r)
	: resolver (r)
    { }

    bool operator()( PoolItem_Ref provider )
    {
	resolver.addPoolItemToVerify (provider);
	return true;
    }
};

bool
Resolver::verifySystem ()
{
    UndoTransact resetting (ResStatus::APPL_HIGH);

    _DEBUG ("Resolver::verifySystem() ");

    invokeOnEach ( _pool.begin(), _pool.end(),
		   resfilter::ByTransact( ),			// Resetting all transcations
		   functor::functorRef<bool,PoolItem>(resetting) );

    VerifySystem info (*this);

    invokeOnEach( pool().byKindBegin( ResTraits<Package>::kind ),
		  pool().byKindEnd( ResTraits<Package>::kind ),
		  resfilter::ByInstalled ( ),
		  functor::functorRef<bool,PoolItem>(info) );

    invokeOnEach( pool().byKindBegin( ResTraits<Pattern>::kind ),
		  pool().byKindEnd( ResTraits<Pattern>::kind ),
		  resfilter::ByInstalled ( ),
		  functor::functorRef<bool,PoolItem>(info) );

// FIXME setting verify mode
#if 0    
    bool success = resolveDependencies (); // do solve only

    DoTransact setting (ResStatus::APPL_HIGH);

    invokeOnEach ( _pool.begin(), _pool.end(),
		   resfilter::ByTransact( ),
		   functor::functorRef<bool,PoolItem>(setting) );
#endif
    return true; // FIXME success
}


//----------------------------------------------------------------------------
// undo

void
Resolver::undo(void)
{
    UndoTransact info(ResStatus::APPL_LOW);
    MIL << "*** undo ***" << endl;
    invokeOnEach ( _pool.begin(), _pool.end(),
		   resfilter::ByTransact( ),			// collect transacts from Pool to resolver queue
		   functor::functorRef<bool,PoolItem>(info) );
    // These conflict should be ignored of the concering item
    _ignoreConflicts.clear();
    // These requires should be ignored of the concering item
    _ignoreRequires.clear();
    // These obsoletes should be ignored of the concering item
    _ignoreObsoletes.clear();
    // Ignore architecture of the item
    _ignoreArchitecture.clear();
    // Ignore the status "installed" of the item
    _ignoreInstalledItem.clear();
    // Ignore the architecture of the item
    _ignoreArchitectureItem.clear();
    // Ignore the vendor of the item
    _ignoreVendorItem.clear();


    return;
}


bool
Resolver::resolvePool()
{

    // Solving with the satsolver
	MIL << "-------------- Calling SAT Solver -------------------" << endl;
	if ( !_satResolver ) {
	    sat::Pool satPool( sat::Pool::instance() );
	    _satResolver = new SATResolver(_pool, satPool.get());
	}
#if 0
	MIL << "------SAT-Pool------" << endl;
	for (sat::Pool::SolvableIterator i = satPool.solvablesBegin();
	     i != satPool.solvablesEnd(); i++ ) {
	    MIL << *i << " ID: " << i->id() << endl;
	}
	MIL << "------SAT-Pool end------" << endl;
#endif
	return _satResolver->resolvePool();
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

