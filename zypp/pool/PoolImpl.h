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
    typedef PoolTraits::size_type      size_type;
    typedef PoolTraits::iterator       iterator;
    typedef PoolTraits::const_iterator const_iterator;
    typedef PoolTraits::indexiterator       indexiterator;
    typedef PoolTraits::const_indexiterator const_indexiterator;
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
      void clear()
      { _store.clear();
	_providesstore.clear();
	return;
      }

    public:
      /** */
      ContainerT _store;
      IndexContainerT _providesstore;
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
