/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/pool/PoolImpl.h
 *
*/
#ifndef ZYPP_POOL_POOLIMPL_H
#define ZYPP_POOL_POOLIMPL_H

#include <iosfwd>

#include "zypp/base/Easy.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/SerialNumber.h"
#include "zypp/base/Deprecated.h"

#include "zypp/pool/PoolTraits.h"
#include "zypp/ResPoolProxy.h"

#include "zypp/sat/Pool.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace pool
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PoolImpl
    //
    /** */
    class PoolImpl
    {
      friend std::ostream & operator<<( std::ostream & str, const PoolImpl & obj );

      public:
        /** */
        typedef PoolTraits::ItemContainerT		ContainerT;
        typedef PoolTraits::size_type			size_type;
        typedef PoolTraits::const_iterator		const_iterator;
	typedef PoolTraits::Id2ItemT			Id2ItemT;

        typedef PoolTraits::repository_iterator		repository_iterator;

        typedef sat::detail::SolvableIdType		SolvableIdType;

      public:
        /** Default ctor */
        PoolImpl();
        /** Dtor */
        ~PoolImpl();

      public:
        /** convenience. */
        const sat::Pool satpool() const
        { return sat::Pool::instance(); }

        /** Housekeeping data serial number. */
        const SerialNumber & serial() const
        { return satpool().serial(); }

        ///////////////////////////////////////////////////////////////////
        //
        ///////////////////////////////////////////////////////////////////
      public:
        /**  */
        bool empty() const
        { return satpool().solvablesEmpty(); }

        /**  */
        size_type size() const
        { return satpool().solvablesSize(); }

        const_iterator begin() const
        { return make_filter_begin( pool::ByPoolItem(), store() ); }

        const_iterator end() const
        { return make_filter_end( pool::ByPoolItem(), store() ); }

      public:
        /** Return the corresponding \ref PoolItem.
         * Pool and sat pool should be in sync. Returns an empty
         * \ref PoolItem if there is no corresponding \ref PoolItem.
         * \see \ref PoolItem::satSolvable.
         */
        PoolItem find( const sat::Solvable & slv_r ) const
        {
          const ContainerT & mystore( store() );
          return( slv_r.id() < mystore.size() ? mystore[slv_r.id()] : PoolItem() );
        }

        ///////////////////////////////////////////////////////////////////
        //
        ///////////////////////////////////////////////////////////////////
      public:
        /** \name Save and restore state. */
        //@{
        void SaveState( const ResObject::Kind & kind_r );

        void RestoreState( const ResObject::Kind & kind_r );
        //@}

        ///////////////////////////////////////////////////////////////////
        //
        ///////////////////////////////////////////////////////////////////
      public:
        ResPoolProxy proxy( ResPool self ) const
        {
          checkSerial();
          if ( !_poolProxy )
          {
            _poolProxy.reset( new ResPoolProxy( self, *this ) );
          }
          return *_poolProxy;
        }

      public:
        /** Forward list of Repositories that contribute ResObjects from \ref sat::Pool */
        size_type knownRepositoriesSize() const
        { checkSerial(); return satpool().reposSize(); }

        repository_iterator knownRepositoriesBegin() const
        { checkSerial(); return satpool().reposBegin(); }

        repository_iterator knownRepositoriesEnd() const
        { checkSerial(); return satpool().reposEnd(); }

        ///////////////////////////////////////////////////////////////////
        //
        ///////////////////////////////////////////////////////////////////
      public:
        const ContainerT & store() const
        {
          checkSerial();
          if ( _storeDirty )
          {
            sat::Pool pool( satpool() );

            if ( pool.capacity() != _store.capacity() )
            {
              _store.resize( pool.capacity() );
            }

            if ( pool.capacity() )
            {
              for ( sat::detail::SolvableIdType i = pool.capacity()-1; i != 0; --i )
              {
                sat::Solvable s( i );
                PoolItem & pi( _store[i] );
                if ( ! s &&  pi )
                {
                  // the PoolItem got invalidated (e.g unloaded repo)
                  pi = PoolItem();
                }
                else if ( s && ! pi )
                {
                  // new PoolItem to add
                  pi = PoolItem::makePoolItem( s ); // the only way to create a new one!
                }
              }
            }
            _storeDirty = false;
          }
          return _store;
        }

	const Id2ItemT & id2item () const
	{
	  checkSerial();
	  if ( _id2itemDirty )
	  {
	    store();
	    _id2item = Id2ItemT( size() );
            for_( it, begin(), end() )
            {
              const sat::Solvable &s = (*it)->satSolvable();
              sat::detail::IdType id = s.ident().id();
              if ( s.isKind( ResKind::srcpackage ) )
                id = -id;
              _id2item.insert( std::make_pair( id, *it ) );
            }
            //INT << _id2item << endl;
	    _id2itemDirty = false;
          }
	  return _id2item;
	}


        ///////////////////////////////////////////////////////////////////
        //
        ///////////////////////////////////////////////////////////////////
      private:
        void checkSerial() const
        {
          if ( _watcher.remember( serial() ) )
            invalidate();
          satpool().prepare(); // always ajust dependencies.
        }

        void invalidate() const
        {
          _storeDirty = true;
	  _id2itemDirty = true;
	  _id2item.clear();
          _poolProxy.reset();
        }

      private:
        /** Watch sat pools serial number. */
        SerialNumberWatcher                   _watcher;
        mutable ContainerT                    _store;
        mutable DefaultIntegral<bool,true>    _storeDirty;
	mutable Id2ItemT		      _id2item;
        mutable DefaultIntegral<bool,true>    _id2itemDirty;

      private:
        mutable shared_ptr<ResPoolProxy>      _poolProxy;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace pool
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_POOL_POOLIMPL_H
