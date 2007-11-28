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
#include "zypp/pool/PoolTraits.h"
#include "zypp/ResPoolProxy.h"
#include "zypp/ZYppFactory.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace pool
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : NameHash
    //
    /** */
    class NameHash
    {
    public:
      /** Default ctor */
      NameHash();
      /** Dtor */
      ~NameHash();

      public:

      typedef PoolTraits::ItemContainerT	 ItemContainerT;
      typedef PoolTraits::NameItemContainerT	 ContainerT;
      typedef PoolTraits::size_type		 size_type;
      typedef PoolTraits::iterator		 iterator;
      typedef PoolTraits::const_iterator	 const_iterator;

      private:
	ItemContainerT & getItemContainer( const std::string & tag_r );
	const ItemContainerT & getConstItemContainer( const std::string & tag_r ) const;

      public:
      /**  */
      ContainerT & store()
      { return _store; }
      /**  */
      const ContainerT & store() const
      { return _store; }

      /**  */
      bool empty() const
      { return _store.empty(); }
      /**  */
      size_type size() const
      { return _store.size(); }

      /** */
      iterator begin( const std::string & tag_r )
      { return getItemContainer( tag_r ).begin(); }
      /** */
      const_iterator begin( const std::string & tag_r ) const
      { return getConstItemContainer( tag_r ).begin(); }

      /** */
      iterator end( const std::string & tag_r )
      { return getItemContainer( tag_r ).end(); }
      /** */
      const_iterator end( const std::string & tag_r ) const
      { return getConstItemContainer( tag_r ).end(); }

      /** */
      void clear()
      { _store.clear(); }

      /** */
      void insert( const PoolItem & item_r );
      /** */
      void erase( const PoolItem & item_r );

      private:
	ContainerT _store;
	ItemContainerT _empty;	// for begin(), end() if tag_r can't be found
    };

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : CapHash
    //
    /** */
    class CapHash
    {
    public:
      /** Default ctor */
      CapHash();
      /** Dtor */
      ~CapHash();

      public:

      typedef PoolTraits::DepCapItemContainerT	ContainerT;
      typedef PoolTraits::capitemsize_type	size_type;
      typedef PoolTraits::capitemiterator	iterator;
      typedef PoolTraits::const_capitemiterator	const_iterator;

      private:

      typedef PoolTraits::CapItemStoreT		CapItemStoreT;
      typedef PoolTraits::CapItemContainerT	CapItemContainerT;

      // Dep -> CapItemStoreT
      const CapItemStoreT & capItemStore ( Dep cap_r ) const;

      // CapItemStoreT, index -> CapItemContainerT
      const CapItemContainerT & capItemContainer( const CapItemStoreT & cis, const std::string & tag_r ) const;

      public:

      /**  */
      ContainerT & store()
      { return _store; }
      /**  */
      const ContainerT & store() const
      { return _store; }

      /**  */
      bool empty() const
      { return _store.empty(); }
      /**  */
      size_type size() const
      { return _store.size(); }

      /** */
      iterator begin( const std::string & tag_r, Dep cap_r )
      { return _store[cap_r][tag_r].begin(); }
      /** */
      const_iterator begin( const std::string & tag_r, Dep cap_r ) const
      { const CapItemStoreT & capitemstore = capItemStore( cap_r );
	const CapItemContainerT & capcontainer = capItemContainer ( capitemstore, tag_r );
	return capcontainer.begin(); }

      /** */
      iterator end( const std::string & tag_r, Dep cap_r )
      { return _store[cap_r][tag_r].begin(); }
      /** */
      const_iterator end( const std::string & tag_r, Dep cap_r ) const
      { const CapItemStoreT & capitemstore = capItemStore( cap_r );
	const CapItemContainerT & capcontainer = capItemContainer ( capitemstore, tag_r );
	return capcontainer.end(); }

      /** */
      void clear()
      { _store.clear(); }

      /** */
      void insert( const PoolItem & item_r );
      /** */
      void erase( const PoolItem & item_r );

      private:
	PoolTraits::DepCapItemContainerT _store;
    };

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
    typedef PoolTraits::Item			Item;
    typedef PoolTraits::ItemContainerT		ContainerT;
    typedef PoolTraits::iterator		iterator;
    typedef PoolTraits::const_iterator		const_iterator;
    typedef PoolTraits::size_type		size_type;
    typedef PoolTraits::Inserter		Inserter;
    typedef PoolTraits::Deleter			Deleter;
    typedef PoolTraits::AdditionalCapSet 	AdditionalCapSet;
    typedef PoolTraits::RepoContainerT          KnownRepositories;

    public:
      /** Default ctor */
      PoolImpl();
      /** Dtor */
      ~PoolImpl();

      /** \todo no poll, but make ZYpp distribute it. */
      Arch targetArch() const
      { return getZYpp()->architecture(); }

    public:
      /**  */
      ContainerT & store()
      { return _store; }
      /**  */
      const ContainerT & store() const
      { return _store; }

      /**  */
      bool empty() const
      { return _store.empty(); }
      /**  */
      size_type size() const
      { return _store.size(); }

      /** */
      iterator begin()
      { return _store.begin(); }
      /** */
      const_iterator begin() const
      { return _store.begin(); }

      /** */
      iterator end()
      { return _store.end(); }
      /** */
      const_iterator end() const
      { return _store.end(); }

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
	void setAdditionalRequire( const AdditionalCapSet & capset ) const
	    { _additionalRequire = capset; }
	AdditionalCapSet & additionalRequire() const
	    { return _additionalRequire; }

       /**
	*  Handling additional conflicts. E.G. do not install anything which provides "foo":
	*
	*  Capset capset;
	*  capset.insert (CapFactory().parse( ResTraits<Package>::kind, "foo"));
	*
	*  setAdditionalConflict( capset );
	*/
	void setAdditionalConflict( const AdditionalCapSet & capset ) const
	    { _additionaConflict = capset; }
	AdditionalCapSet & additionaConflict() const
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
	void setAdditionalProvide( const AdditionalCapSet & capset ) const
	    { _additionaProvide = capset; }
	AdditionalCapSet & additionaProvide() const
	    { return _additionaProvide; }

      /** */
      void clear()
      {
        _store.clear();
	_caphash.clear();
	_namehash.clear();
        _additionalRequire.clear();
	_additionaConflict.clear();
	_additionaProvide.clear();
	// don't miss to invalidate ResPoolProxy
	invalidateProxy();
	return;
      }

      /** erase all resolvables coming from the target  */
      void eraseInstalled() const;

    public:
      /** Access list of Repositories that contribute ResObjects.
       * Built on demand.
      */
      const KnownRepositories & knownRepositories() const
      {
	if ( ! _knownRepositoriesPtr )
	{
	  _knownRepositoriesPtr.reset( new KnownRepositories );
	  for_( it, _store.begin(), _store.end() )
	  {
	    if ( (*it)->repository() != Repository::noRepository )
	    {
	      _knownRepositoriesPtr->insert( (*it)->repository() );
	    }
	  }
	}

	return *_knownRepositoriesPtr;
      }

    public:
      /** \name Save and restore state. */
      //@{
      void SaveState( const ResObject::Kind & kind_r );

      void RestoreState( const ResObject::Kind & kind_r );
      //@}

    public:
      /** Serial number changing whenever the content
       * (Resolvables or Dependencies) changes. */
      const SerialNumber & serial() const;

      /** Wheter in sync with sat-pool. */
      bool satSynced() const
      { return _satSyncRequired.isClean( _serial ); }

      /** Sync with sat-pool. */
      void satSync() const;

      /** Return the corresponding \ref PoolItem.
       * Pool and sat pool should be in sync. Returns an empty
       * \ref PoolItem if there is no corresponding \ref PoolItem.
       * \see \ref PoolItem::satSolvable.
       */
      PoolItem find( const sat::Solvable & slv_r ) const;

    private:
      /** Serial number. */
      SerialNumber        _serial;
      /** Watch for changes in /etc/sysconfig/storage. */
      SerialNumberWatcher _watchFilesystemSysconfigStorage;
      /** Watch for changes \c _serial. */
      SerialNumberWatcher _satSyncRequired;

    public:
      ContainerT   _store;
      NameHash     _namehash;
      CapHash      _caphash;
      mutable AdditionalCapSet _additionalRequire;
      mutable AdditionalCapSet _additionaConflict;
      mutable AdditionalCapSet _additionaProvide;

    public:
      ResPoolProxy proxy( ResPool self ) const
      {
        if ( !_poolProxy )
          _poolProxy.reset( new ResPoolProxy( self ) );
        return *_poolProxy;
      }

      /** Invalidate all data we build on demand.
       * To be called whenever the pools content changes
       */
      void invalidateProxy()
      {
        _serial.setDirty();
	_poolProxy.reset();
	_knownRepositoriesPtr.reset();
      }

      mutable shared_ptr<ResPoolProxy> _poolProxy;

    private:
      /** Set of known repositories built on demand.
       * Invalidated on any Pool content change. Rebuilt on next access.
       */
      mutable scoped_ptr<KnownRepositories> _knownRepositoriesPtr;
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
