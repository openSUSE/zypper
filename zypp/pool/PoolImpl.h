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
#include "zypp/APIConfig.h"

#include "zypp/pool/PoolTraits.h"
#include "zypp/ResPoolProxy.h"
#include "zypp/PoolQueryResult.h"

#include "zypp/sat/Pool.h"
#include "zypp/Product.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  namespace resstatus
  {
    /** Manipulator for \ref ResStatus::UserLockQueryField.
     * Field is not public available. It is intended to remember the
     * initial lock status usually derived from /etc/zypp/locks. So
     * we are able to detect changes we have to write back on commit.
    */
    struct UserLockQueryManip
    {
      /** Set lock and UserLockQuery bit according to \c yesno_r. */
      static void setLock( ResStatus & status_r, bool yesno_r )
      {
        status_r.setLock( yesno_r, ResStatus::USER );
        status_r.setUserLockQueryMatch( yesno_r );
      }

      /** Update lock and UserLockQuery bit IFF the item gained the bit. */
      static void reapplyLock( ResStatus & status_r, bool yesno_r )
      {
        if ( yesno_r && ! status_r.isUserLockQueryMatch() )
        {
          status_r.setLock( yesno_r, ResStatus::USER );
          status_r.setUserLockQueryMatch( yesno_r );
        }
      }

      /** Test whether the lock status differs from the remembered UserLockQuery bit. */
      static int diffLock( const ResStatus & status_r )
      {
        bool userLock( status_r.isUserLocked() );
        if ( userLock == status_r.isUserLockQueryMatch() )
          return 0;
        return userLock ? 1 : -1;
      }

    };
  }

  namespace
  {
    inline PoolQuery makeTrivialQuery( IdString ident_r )
    {
      sat::Solvable::SplitIdent ident( ident_r );

      PoolQuery q;
      q.addAttribute( sat::SolvAttr::name, ident.name().asString() );
      q.addKind( ident.kind() );
      q.setMatchExact();
      q.setCaseSensitive(true);
      return q;
   }

    inline bool hardLockQueriesRemove( pool::PoolTraits::HardLockQueries & activeLocks_r, IdString ident_r )
    {
      unsigned s( activeLocks_r.size() );
      activeLocks_r.remove( makeTrivialQuery( ident_r ) );
      return( activeLocks_r.size() != s );
    }

    inline bool hardLockQueriesAdd( pool::PoolTraits::HardLockQueries & activeLocks_r, IdString ident_r )
    {
      PoolQuery q( makeTrivialQuery( ident_r ) );
      for_( it, activeLocks_r.begin(), activeLocks_r.end() )
      {
        if ( *it == q )
          return false;
      }
      activeLocks_r.push_back( q );
      return true;
    }
  }

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

        Repository reposFind( const std::string & alias_r ) const
        { checkSerial(); return satpool().reposFind( alias_r ); }

        ///////////////////////////////////////////////////////////////////
        //
        ///////////////////////////////////////////////////////////////////
      public:
        typedef PoolTraits::HardLockQueries           HardLockQueries;
        typedef PoolTraits::hardLockQueries_iterator  hardLockQueries_iterator;

        const HardLockQueries & hardLockQueries() const
        { return _hardLockQueries; }

        void reapplyHardLocks() const
        {
          // It is assumed that reapplyHardLocks is called after new
          // items were added to the pool, but the _hardLockQueries
          // did not change since. Action is to be performed only on
          // those items that gained the bit in the UserLockQueryField.
          MIL << "Re-apply " << _hardLockQueries.size() << " HardLockQueries" << endl;
          PoolQueryResult locked;
          for_( it, _hardLockQueries.begin(), _hardLockQueries.end() )
          {
            locked += *it;
          }
          MIL << "HardLockQueries match " << locked.size() << " Solvables." << endl;
          for_( it, begin(), end() )
          {
            resstatus::UserLockQueryManip::reapplyLock( it->status(), locked.contains( *it ) );
          }
        }

        void setHardLockQueries( const HardLockQueries & newLocks_r )
        {
          MIL << "Apply " << newLocks_r.size() << " HardLockQueries" << endl;
          _hardLockQueries = newLocks_r;
          // now adjust the pool status
          PoolQueryResult locked;
          for_( it, _hardLockQueries.begin(), _hardLockQueries.end() )
          {
            locked += *it;
          }
          MIL << "HardLockQueries match " << locked.size() << " Solvables." << endl;
          for_( it, begin(), end() )
          {
            resstatus::UserLockQueryManip::setLock( it->status(), locked.contains( *it ) );
          }
        }

        bool getHardLockQueries( HardLockQueries & activeLocks_r )
        {
          activeLocks_r = _hardLockQueries; // current queries
          // Now diff to the pool collecting names only.
          // Thus added and removed locks are not necessarily
          // disjoint. Added locks win.
          typedef std::tr1::unordered_set<IdString> IdentSet;
          IdentSet addedLocks;
          IdentSet removedLocks;
          for_( it, begin(), end() )
          {
            switch ( resstatus::UserLockQueryManip::diffLock( it->status() ) )
            {
              case 0:  // unchanged
                break;
              case 1:
                addedLocks.insert( it->satSolvable().ident() );
                break;
              case -1:
                removedLocks.insert( it->satSolvable().ident() );
               break;
            }
          }
          // now the bad part - adjust the queries
          bool setChanged = false;
          for_( it, removedLocks.begin(), removedLocks.end() )
          {
            if ( addedLocks.find( *it ) != addedLocks.end() )
              continue; // Added locks win
            if ( hardLockQueriesRemove( activeLocks_r, *it ) && ! setChanged )
              setChanged = true;
          }
          for_( it, addedLocks.begin(), addedLocks.end() )
          {
            if ( hardLockQueriesAdd( activeLocks_r, *it ) && ! setChanged )
              setChanged = true;
          }
          return setChanged;
       }

      public:
        typedef PoolTraits::AutoSoftLocks          AutoSoftLocks;
        typedef PoolTraits::autoSoftLocks_iterator autoSoftLocks_iterator;

        const AutoSoftLocks & autoSoftLocks() const
        { return _autoSoftLocks; }

        bool autoSoftLockAppliesTo( sat::Solvable solv_r ) const
        { return( _autoSoftLocks.find( solv_r.ident() ) != _autoSoftLocks.end() ); }

        void setAutoSoftLocks( const AutoSoftLocks & newLocks_r )
        {
          MIL << "Apply " << newLocks_r.size() << " AutoSoftLocks: " << newLocks_r << endl;
          _autoSoftLocks = newLocks_r;
          // now adjust the pool status
          for_( it, begin(), end() )
          {
            if ( ! it->status().isKept() )
              continue;

            if ( autoSoftLockAppliesTo( it->satSolvable() ) )
              it->status().setSoftLock( ResStatus::USER );
            else
              it->status().resetTransact( ResStatus::USER );
          }
        }

        void getActiveSoftLocks( AutoSoftLocks & activeLocks_r )
        {
          activeLocks_r = _autoSoftLocks; // current soft-locks
          AutoSoftLocks todel;            // + names to be deleted
          AutoSoftLocks toins;            // - names to be installed

          for_( it, begin(), end() )
          {
            ResStatus & status( it->status() );
            if ( ! ( status.isByUser() || status.isByApplLow() ) )
              continue; // ignore non-user requests; ApplLow means selected
                        // by solver, but on behalf of a user request.

            switch ( status.getTransactValue() )
            {
              case ResStatus::KEEP_STATE:
                // Filter only items included in the last recommended set.
                if ( status.isRecommended() )
                  activeLocks_r.insert( it->satSolvable().ident() );
                break;
              case ResStatus::LOCKED:
                //  NOOP
                break;
              case ResStatus::TRANSACT:
                (status.isInstalled() ? todel : toins).insert( it->satSolvable().ident() );
                break;
            }
          }
          for_( it, todel.begin(), todel.end() )
          {
            activeLocks_r.insert( *it );
          }
          for_( it, toins.begin(), toins.end() )
          {
            activeLocks_r.erase( *it );
          }
        }

      public:
        const ContainerT & store() const
        {
          checkSerial();
          if ( _storeDirty )
          {
            sat::Pool pool( satpool() );
            bool addedItems = false;
            std::list<PoolItem> addedProducts;

	    _store.resize( pool.capacity() );

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
                  // remember products for buddy processing (requires clean store)
                  if ( s.isKind( ResKind::product ) )
                    addedProducts.push_back( pi );
                  // and on the fly check for weak locks...
                  if ( autoSoftLockAppliesTo( s ) )
                  {
                    pi.status().setSoftLock( ResStatus::USER );
                  }
                  if ( !addedItems )
                    addedItems = true;
                }
              }
            }
            _storeDirty = false;

            // Now, as the pool is adjusted, ....

            // .... we check for product buddies.
            if ( ! addedProducts.empty() )
            {
              for_( it, addedProducts.begin(), addedProducts.end() )
              {
                it->setBuddy( asKind<Product>(*it)->referencePackage() );
              }
            }

            // .... we must reapply those query based hard locks.
            if ( addedItems )
            {
              reapplyHardLocks();
            }
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

      private:
        /** Set of solvable idents that should be soft locked per default. */
        AutoSoftLocks                         _autoSoftLocks;
        /** Set of queries that define hardlocks. */
        HardLockQueries                       _hardLockQueries;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace pool
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_POOL_POOLIMPL_H
