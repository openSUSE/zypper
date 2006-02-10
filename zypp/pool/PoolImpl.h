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

#include "zypp/pool/PoolTraits.h"
#include "zypp/ResPoolProxy.h"

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
    typedef PoolTraits::Item           Item;
    typedef PoolTraits::ContainerT     ContainerT;
    typedef PoolTraits::IndexContainerT    IndexContainerT;
    typedef PoolTraits::NameContainerT    NameContainerT;
    typedef PoolTraits::size_type      size_type;
    typedef PoolTraits::iterator       iterator;
    typedef PoolTraits::const_iterator const_iterator;
    typedef PoolTraits::indexiterator       indexiterator;
    typedef PoolTraits::const_indexiterator const_indexiterator;
    typedef PoolTraits::nameiterator       nameiterator;
    typedef PoolTraits::const_nameiterator const_nameiterator;
    typedef PoolTraits::Inserter       Inserter;
    typedef PoolTraits::Deleter        Deleter;

    public:
      /** Default ctor */
      PoolImpl();
      /** Dtor */
      ~PoolImpl();

    public:
      /**  */
      ContainerT & store()
      { return _store; }
      /**  */
      const ContainerT & store() const
      { return _store; }

      /**  */
      IndexContainerT & providesstore()
      { return _providesstore; }
      /**  */
      const IndexContainerT & providesstore() const
      { return _providesstore; }

      IndexContainerT & requiresstore()
      { return _requiresstore; }
      /**  */
      const IndexContainerT & requiresstore() const
      { return _requiresstore; }

      IndexContainerT & conflictsstore()
      { return _conflictsstore; }
      /**  */
      const IndexContainerT & conflictsstore() const
      { return _conflictsstore; }

      /**  */
      NameContainerT & namestore()
      { return _namestore; }
      /**  */
      const NameContainerT & namestore() const
      { return _namestore; }

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
      indexiterator providesbegin(const std::string & tag_r)
      { return _providesstore.lower_bound (tag_r); }
      /** */
      const_indexiterator providesbegin(const std::string & tag_r) const
      { return _providesstore.lower_bound (tag_r); }

      /** */
      indexiterator requiresbegin(const std::string & tag_r)
      { return _requiresstore.lower_bound (tag_r); }
      /** */
      const_indexiterator requiresbegin(const std::string & tag_r) const
      { return _requiresstore.lower_bound (tag_r); }

      /** */
      indexiterator conflictsbegin(const std::string & tag_r)
      { return _conflictsstore.lower_bound (tag_r); }
      /** */
      const_indexiterator conflictsbegin(const std::string & tag_r) const
      { return _conflictsstore.lower_bound (tag_r); }

      /** */
      nameiterator namebegin(const std::string & tag_r)
      { return _namestore.lower_bound (tag_r); }
      /** */
      const_nameiterator namebegin(const std::string & tag_r) const
      { return _namestore.lower_bound (tag_r); }

      /** */
      iterator end()
      { return _store.end(); }
      /** */
      const_iterator end() const
      { return _store.end(); }

      /** */
      indexiterator providesend(const std::string & tag_r)
      { return _providesstore.upper_bound (tag_r); }
      /** */
      const_indexiterator providesend(const std::string & tag_r) const
      { return _providesstore.upper_bound (tag_r); }

      /** */
      indexiterator requiresend(const std::string & tag_r)
      { return _requiresstore.upper_bound (tag_r); }
      /** */
      const_indexiterator requiresend(const std::string & tag_r) const
      { return _requiresstore.upper_bound (tag_r); }

      /** */
      indexiterator conflictsend(const std::string & tag_r)
      { return _conflictsstore.upper_bound (tag_r); }
      /** */
      const_indexiterator conflictsend(const std::string & tag_r) const
      { return _conflictsstore.upper_bound (tag_r); }

      /** */
      nameiterator nameend(const std::string & tag_r)
      { return _namestore.upper_bound (tag_r); }
      /** */
      const_nameiterator nameend(const std::string & tag_r) const
      { return _namestore.upper_bound (tag_r); }

      /** */
      void clear()
      { _store.clear();
	_providesstore.clear();
	_requiresstore.clear();
	_conflictsstore.clear();
	_namestore.clear();
	return;
      }

    public:
      /** \name Save and restore state. */
      //@{
      void SaveState( const ResObject::Kind & kind_r );

      void RestoreState( const ResObject::Kind & kind_r );
      //@}

    public:
      /** */
      ContainerT _store;
      IndexContainerT _providesstore;
      IndexContainerT _requiresstore;
      IndexContainerT _conflictsstore;
      NameContainerT _namestore;

    public:
      ResPoolProxy proxy( ResPool self ) const
      {
        if ( !_poolProxy )
          _poolProxy.reset( new ResPoolProxy( self ) );
        return *_poolProxy;
      }
      void invalidateProxy()
      { _poolProxy.reset(); }

      mutable shared_ptr<ResPoolProxy> _poolProxy;
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
