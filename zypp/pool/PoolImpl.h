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
#include <map>

#include "zypp/base/Easy.h"
#include "zypp/base/SerialNumber.h"
#include "zypp/base/Deprecated.h"

#include "zypp/pool/PoolTraits.h"
#include "zypp/ResPoolProxy.h"

#include "zypp/sat/Pool.h"

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

        typedef sat::detail::SolvableIdType		SolvableIdType;

        typedef PoolTraits::AdditionalCapabilities	AdditionalCapabilities;
        typedef PoolTraits::RepoContainerT		KnownRepositories;

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

        /** */
        const_iterator begin() const
        { return make_map_value_begin( store() ); }

        /** */
        const_iterator end() const
        { return make_map_value_end( store() ); }

      public:
        /** Return the corresponding \ref PoolItem.
         * Pool and sat pool should be in sync. Returns an empty
         * \ref PoolItem if there is no corresponding \ref PoolItem.
         * \see \ref PoolItem::satSolvable.
         */
        PoolItem find( const sat::Solvable & slv_r ) const
        {
          const ContainerT & c( store() );
          ContainerT::const_iterator it( c.find( slv_r ) );
          if ( it != c.end() )
            return it->second;
          return PoolItem();
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
        /**
         *  Handling additional requirement. E.G. need package "foo" and package
         *  "foo1" which has a greater version than 1.0:
         *
         *  Capset capset;
         *  capset.insert (CapFactory().parse( ResTraits<Package>::kind, "foo"));
         *  capset.insert (CapFactory().parse( ResTraits<Package>::kind, "foo1 > 1.0"));
         *
         *  setAdditionalRequire( capset );
         */
        void setAdditionalRequire( const AdditionalCapabilities & capset ) const
        { _additionalRequire = capset; }
        AdditionalCapabilities & additionalRequire() const
        { return _additionalRequire; }

        /**
         *  Handling additional conflicts. E.G. do not install anything which provides "foo":
         *
         *  Capset capset;
         *  capset.insert (CapFactory().parse( ResTraits<Package>::kind, "foo"));
         *
         *  setAdditionalConflict( capset );
         */
        void setAdditionalConflict( const AdditionalCapabilities & capset ) const
        { _additionaConflict = capset; }
        AdditionalCapabilities & additionaConflict() const
        { return _additionaConflict; }

	/**
         *  Handling additional provides. This is used for ignoring a requirement.
	 *  e.G. Do ignore the requirement "foo":
         *
         *  Capset capset;
         *  capset.insert (CapFactory().parse( ResTraits<Package>::kind, "foo"));
         *
         *  setAdditionalProvide( cap );
         */
        void setAdditionalProvide( const AdditionalCapabilities & capset ) const
        { _additionaProvide = capset; }
        AdditionalCapabilities & additionaProvide() const
        { return _additionaProvide; }

        ///////////////////////////////////////////////////////////////////
        //
        ///////////////////////////////////////////////////////////////////
      public:
        ResPoolProxy proxy( ResPool self ) const
        {
          checkSerial();
          if ( !_poolProxy )
            _poolProxy.reset( new ResPoolProxy( self ) );
          return *_poolProxy;
        }

      public:
        /** Access list of Repositories that contribute ResObjects.
         * Built on demand.
         */
        const KnownRepositories & knownRepositories() const
        {
          checkSerial();
          if ( ! _knownRepositoriesPtr )
          {
            _knownRepositoriesPtr.reset( new KnownRepositories );
            const ContainerT & c( store() );
            for_( it, c.begin(), c.end() )
            {
              if ( (*it).second->repository() != Repository::noRepository )
              {
                _knownRepositoriesPtr->insert( (*it).second->repository() );
              }
            }
          }
          return *_knownRepositoriesPtr;
        }

        ///////////////////////////////////////////////////////////////////
        //
        ///////////////////////////////////////////////////////////////////
      private:
        const ContainerT & store() const
        {
          checkSerial();
          if ( _storeDirty )
          {
            // pass 1: delete no longer existing solvables
            for ( ContainerT::iterator it = _store.begin(); it != _store.end(); /**/ )
            {
              if ( ! it->first ) // solvable became invalid
                _store.erase( it++ ); // postfix! Incrementing before erase
              else
                ++it;
            }

            // pass 2: add new solvables
            sat::Pool pool( satpool() );
            if ( _store.size() != pool.solvablesSize() )
            {
              for_( it, pool.solvablesBegin(), pool.solvablesEnd() )
              {
                PoolItem & pi( _store[*it] );
                if ( ! pi ) // newly created
                {
                  pi = PoolItem( *it );
                }
              }
            }

            _storeDirty = false;
          }
          return _store;
        }

        ///////////////////////////////////////////////////////////////////
        //
        ///////////////////////////////////////////////////////////////////
      private:
        void checkSerial() const
        {
          if ( _watcher.remember( serial() ) )
            invalidate();
        }

        void invalidate() const
        {
          _storeDirty = true;
          _poolProxy.reset();
          _knownRepositoriesPtr.reset();
        }

      private:
        /** Watch sat pools serial number. */
        SerialNumberWatcher                   _watcher;
        mutable ContainerT                    _store;
        mutable DefaultIntegral<bool,true>    _storeDirty;

      private:
        mutable AdditionalCapabilities        _additionalRequire;
        mutable AdditionalCapabilities        _additionaConflict;
        mutable AdditionalCapabilities        _additionaProvide;

        mutable shared_ptr<ResPoolProxy>      _poolProxy;
        mutable scoped_ptr<KnownRepositories> _knownRepositoriesPtr;

      public:
        /** \bug FAKE capindex */
        const PoolTraits::CapItemContainerT   _caphashfake;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates PoolImpl Stream output */
    std::ostream & operator<<( std::ostream & str, const PoolImpl & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace pool
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_POOL_POOLIMPL_H
