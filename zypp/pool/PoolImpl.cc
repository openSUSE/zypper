/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/pool/PoolImpl.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/pool/PoolImpl.h"
#include "zypp/CapSet.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace pool
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : PoolImpl::PoolImpl
    //	METHOD TYPE : Ctor
    //
    PoolImpl::PoolImpl()
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : PoolImpl::~PoolImpl
    //	METHOD TYPE : Dtor
    //
    PoolImpl::~PoolImpl()
    {}

    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const PoolImpl & obj )
    {
      return str << "PoolImpl " << obj.size();
    }

    void PoolImplInserter::operator()( ResObject::constPtr ptr_r, bool installed )
    {
      PoolImpl::Item item ( ptr_r, ResStatus (installed) );
      _poolImpl.store().insert( item );
      _poolImpl.namestore().insert( PoolImpl::NameContainerT::value_type (item->name(), item ) );
      CapSet provides = item->dep( Dep::PROVIDES );
      for (CapSet::iterator ic = provides.begin(); ic != provides.end(); ++ic) {
	_poolImpl.providesstore().insert( PoolImpl::IndexContainerT::value_type (ic->index(), std::make_pair( *ic, item ) ) );
      }
      CapSet requires = item->dep( Dep::REQUIRES );
      for (CapSet::iterator ic = requires.begin(); ic != requires.end(); ++ic) {
	_poolImpl.requiresstore().insert( PoolImpl::IndexContainerT::value_type (ic->index(), std::make_pair( *ic, item ) ) );
      }
      CapSet conflicts = item->dep( Dep::CONFLICTS );
      for (CapSet::iterator ic = conflicts.begin(); ic != conflicts.end(); ++ic) {
	_poolImpl.conflictsstore().insert( PoolImpl::IndexContainerT::value_type (ic->index(), std::make_pair( *ic, item ) ) );
      }
    }

    void PoolImplDeleter::operator()( ResObject::constPtr ptr_r )
    {
      PoolImpl::Item item( ptr_r );
      _poolImpl.store().erase( item );
      for (PoolImpl::nameiterator nit = _poolImpl.namestore().lower_bound (item->name());
				  nit != _poolImpl.namestore().upper_bound (item->name()); ++nit)
      {
	if (nit->second == item)
	    _poolImpl.namestore().erase( nit );
      }
      CapSet provides = ptr_r->dep( Dep::PROVIDES );
      for (CapSet::iterator ic = provides.begin(); ic != provides.end(); ++ic) {
	for (PoolImpl::indexiterator iit = _poolImpl.providesstore().lower_bound (ic->index());
			   iit != _poolImpl.providesstore().upper_bound (ic->index()); ++iit)
	{
	    if (iit->second.second == item)
		_poolImpl.providesstore().erase( iit );
	}
      }
      CapSet requires = ptr_r->dep( Dep::REQUIRES );
      for (CapSet::iterator ic = requires.begin(); ic != requires.end(); ++ic) {
	for (PoolImpl::indexiterator iit = _poolImpl.requiresstore().lower_bound (ic->index());
			   iit != _poolImpl.requiresstore().upper_bound (ic->index()); ++iit)
	{
	    if (iit->second.second == item)
		_poolImpl.requiresstore().erase( iit );
	}
      }
      CapSet conflicts = ptr_r->dep( Dep::CONFLICTS );
      for (CapSet::iterator ic = conflicts.begin(); ic != conflicts.end(); ++ic) {
	for (PoolImpl::indexiterator iit = _poolImpl.conflictsstore().lower_bound (ic->index());
			   iit != _poolImpl.conflictsstore().upper_bound (ic->index()); ++iit)
	{
	    if (iit->second.second == item)
		_poolImpl.conflictsstore().erase( iit );
	}
      }
    }

    /////////////////////////////////////////////////////////////////
  } // namespace pool
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
