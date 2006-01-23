/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ResPool.h
 *
*/
#ifndef ZYPP_RESPOOL_H
#define ZYPP_RESPOOL_H

#include <iostream>
#include "zypp/base/Logger.h"

#include <iosfwd>
#include <set>

#include "zypp/base/Iterator.h"
#include "zypp/pool/PoolItem.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ResPool
  //
  /** */
  class ResPool
  {
    friend std::ostream & operator<<( std::ostream & str, const ResPool & obj );

  public:
    /** */
    typedef pool::PoolItem Item;

  private:
    /** */
    typedef std::set<Item>  ContainerT;

  public:

    typedef ContainerT::size_type      size_type;
    typedef ContainerT::iterator       iterator;
    typedef ContainerT::const_iterator const_iterator;

  public:
    /** Default ctor */
    ResPool();
    /** Dtor */
    ~ResPool();

  public: // qeries based on Item.
    /**  */
    bool empty() const
    { return store().empty(); }
    /**  */
    size_type size() const
    { return store().size(); }

    /** */
    const_iterator begin() const
    { return store().begin(); }
    /** */
    const_iterator end() const
    { return store().end(); }

    /** */
    template<class _Filter>
      filter_iterator<_Filter, const_iterator> begin() const
      { return make_filter_iterator( _Filter(), begin(), end() ); }
    /** */
    template<class _Filter>
      filter_iterator<_Filter, const_iterator> begin( _Filter f ) const
      { return make_filter_iterator( f, begin(), end() ); }

    /** */
    template<class _Filter>
      filter_iterator<_Filter, const_iterator> end() const
      { return make_filter_iterator( _Filter(), end(), end() ); }
    /** */
    template<class _Filter>
      filter_iterator<_Filter, const_iterator> end( _Filter f ) const
      { return make_filter_iterator( f, end(), end() ); }

  public: // insert/erase based on ResObject::constPtr.
    /**  */
    void insert( ResObject::constPtr ptr_r )
    { _inserter( ptr_r ); }

    /**  */
    template <class _InputIterator>
      void insert( _InputIterator first_r, _InputIterator last_r )
      { std::for_each( first_r, last_r, _inserter ); }

    /**  */
    void erase( ResObject::constPtr ptr_r )
    { _deleter( ptr_r ); }

    /**  */
    void erase( iterator first_r, iterator last_r )
    { std::for_each( first_r, last_r, _deleter ); }

    /**  */
    void clear()
    { store().clear(); }

  private:
    struct Inserter
    {
      void operator()( ResObject::constPtr ptr_r )
      { INT << "+++ " << *ptr_r << std::endl; }

      Inserter( ContainerT & store_r )
      : _store( store_r )
      {}
      ContainerT & _store;
    };

    struct Deleter
    {
      void operator()( ResObject::constPtr ptr_r )
      { SEC << "--- " << *ptr_r << std::endl; }

      Deleter( ContainerT & store_r )
      : _store( store_r )
      {}
      ContainerT & _store;
    };

    /**  */
    ContainerT & store()
    { return _store; }
    /**  */
    const ContainerT & store() const
    { return _store; }

  private:
    /**  */
    ContainerT _store;
    /**  */
    Inserter _inserter;
    /**  */
    Deleter  _deleter;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ResPool Stream output */
  std::ostream & operator<<( std::ostream & str, const ResPool & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESPOOL_H
