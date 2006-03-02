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

    std::ostream & operator<<( std::ostream & str, const CapAndItem & obj )
    {
	return str << "{" << obj.cap << ", " << obj.item << "}";
    }

  ///////////////////////////////////////////////////////////////////
  namespace pool
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : NameHash::NameHash
    //	METHOD TYPE : Ctor
    //
    NameHash::NameHash()
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : NameHash::~NameHash
    //	METHOD TYPE : Dtor
    //
    NameHash::~NameHash()
    {}

    void
    NameHash::insert( const PoolItem & item_r )
    {
      _store[item_r->name()].insert( item_r );
    }

    void
    NameHash::erase( const PoolItem & item_r )
    {
      PoolTraits::ItemContainerT items = _store[item_r->name()];
      for (PoolTraits::iterator nit = items.begin();
		    nit != items.end(); ++nit)
      {
	if (*nit == item_r)
	    items.erase( nit );
      }
    }

    NameHash::ItemContainerT & NameHash::getItemContainer( const std::string & tag_r )
	{ ContainerT::iterator it = _store.find( tag_r );
	  if (it == _store.end()) {
//XXX << "item container for " << tag_r << " not found" << endl;
	    return _empty;
	  }
//XXX << "item container for " << tag_r << " contains " << it->second.size() << " items" << endl;
	  return it->second;
	}

    const NameHash::ItemContainerT & NameHash::getConstItemContainer( const std::string & tag_r ) const
	{ ContainerT::const_iterator it = _store.find( tag_r );
	  if (it == _store.end()) {
//XXX << "const item container for " << tag_r << " not found" << endl;
	    return _empty;
	  }
//XXX << "const item container for " << tag_r << " contains " << it->second.size() << " items" << endl;
	  return it->second;
	}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : CapHash::CapHash
    //	METHOD TYPE : Ctor
    //
    CapHash::CapHash()
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : CapHash::~CapHash
    //	METHOD TYPE : Dtor
    //
    CapHash::~CapHash()
    {}

    static void
    storeInsert( CapHash::ContainerT & store_r, const PoolItem & item_r, Dep cap_r )
    {
      CapSet caps = item_r->dep( cap_r );
      for (CapSet::iterator ic = caps.begin(); ic != caps.end(); ++ic) {
	store_r[cap_r][ic->index()].push_back( CapAndItem( *ic, item_r ) );
      }
    }

    void CapHash::insert( const PoolItem & item_r )
    {
      storeInsert( _store, item_r, Dep::PROVIDES );
      storeInsert( _store, item_r, Dep::REQUIRES );
      storeInsert( _store, item_r, Dep::CONFLICTS );
      storeInsert( _store, item_r, Dep::OBSOLETES );
      storeInsert( _store, item_r, Dep::FRESHENS );
      storeInsert( _store, item_r, Dep::SUPPLEMENTS );
    }

    static void
    storeDelete( PoolTraits::DepCapItemContainerT & store_r, const PoolItem & item_r, Dep cap_r )
    {
      CapSet caps = item_r->dep( cap_r );
//XXX << "storeDelete(" << item_r << ")" << endl;
      for (CapSet::iterator ic = caps.begin(); ic != caps.end(); ++ic) {
	PoolTraits::CapItemContainerT capitems = store_r[cap_r][ic->index()];
	for (PoolTraits::CapItemContainerT::iterator pos = capitems.begin();
					     pos != capitems.end();)
	{
	    PoolTraits::CapItemContainerT::iterator next = pos; ++next;
	    if (pos->item == item_r)
		capitems.erase( pos );
	    pos = next;
	}
      }
    }

    void CapHash::erase( const PoolItem & item_r )
    {
//XXX << "CapHash::erase(" << item_r << ")" << endl;
      storeDelete( _store, item_r, Dep::PROVIDES );
      storeDelete( _store, item_r, Dep::REQUIRES );
      storeDelete( _store, item_r, Dep::CONFLICTS );
      storeDelete( _store, item_r, Dep::OBSOLETES );
      storeDelete( _store, item_r, Dep::FRESHENS );
      storeDelete( _store, item_r, Dep::SUPPLEMENTS );
    }

      const CapHash::CapItemStoreT & CapHash::capItemStore ( Dep cap_r ) const
      { static CapItemStoreT capitemstore;
	ContainerT::const_iterator it = store().find( cap_r );
	if (it == store().end()) {
//XXX << "CapItemStoreT for " << cap_r << " not found" << endl;
	    return capitemstore;
	}
//XXX << "CapItemStoreT for " << cap_r << " contains " << it->second.size() << " items" << endl;
	return it->second;
      }

      // CapItemStoreT, index -> CapItemContainerT
      const CapHash::CapItemContainerT & CapHash::capItemContainer( const CapItemStoreT & cis, const std::string & tag_r ) const
      { static CapItemContainerT captemcontainer;
	CapItemStoreT::const_iterator it = cis.find( tag_r );
	if (it == cis.end()) {
//XXX << "CapItemContainerT for " << tag_r << " not found" << endl;
	    return captemcontainer;
	}
//XXX << "CapItemContainerT for " << tag_r << " contains " << it->second.size() << " items" << endl;
//for (CapItemContainerT::const_iterator cai = it->second.begin(); cai != it->second.end(); ++cai) XXX << *cai << endl;
	return it->second;
      }

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


    /******************************************************************
    **
    **	FUNCTION NAME : PoolImplInserter::operator()
    **	FUNCTION TYPE : void
    */
    /** Bottleneck inserting ResObjects in to the pool.
     * Filters arch incomatible available(!) objects.
    */
    void PoolImplInserter::operator()( ResObject::constPtr ptr_r )
    {
      if ( ptr_r && ( ptr_r->kind() != ResTraits<SrcPackage>::kind
		      && ( _installed
			   || ptr_r->arch().compatibleWith( _poolImpl.targetArch() ) ) ) )
        {
          PoolImpl::Item item ( ptr_r, ResStatus (_installed) );
          _poolImpl._store.insert( item );
          _poolImpl._namehash.insert( item );
          _poolImpl._caphash.insert( item );

          // don't miss to invalidate ResPoolProxy
          _poolImpl.invalidateProxy();
        }
    }

    /******************************************************************
    **
    **	FUNCTION NAME : PoolImplDeleter::operator()
    **	FUNCTION TYPE : void
    */
    void PoolImplDeleter::operator()( ResObject::constPtr ptr_r )
    {
      PoolImpl::Item item( ptr_r );
      _poolImpl._store.erase( item );
      _poolImpl._namehash.erase( item );
      _poolImpl._caphash.erase( item );

      // don't miss to invalidate ResPoolProxy
      _poolImpl.invalidateProxy();
    }


    /////////////////////////////////////////////////////////////////
  } // namespace pool
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
