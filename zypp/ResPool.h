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

#include <iosfwd>

#include "zypp/base/Iterator.h"
#include "zypp/pool/PoolTraits.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ResPool
  //
  /** Access to ResObject pool. */
  class ResPool
  {
    friend std::ostream & operator<<( std::ostream & str, const ResPool & obj );

  public:
    /** */
    typedef pool::PoolTraits::Item           Item;
    typedef pool::PoolTraits::size_type      size_type;
    typedef pool::PoolTraits::const_iterator const_iterator;

  public:
    /** Dtor */
    ~ResPool();

  public:
    /**  */
    bool empty() const;
    /**  */
    size_type size() const;
    /** */
    const_iterator begin() const;
    /** */
    const_iterator end() const;

  public:
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

  private:
    /** */
    friend class ResPoolManager;
    /** Ctor */
    ResPool( pool::PoolTraits::Impl_constPtr impl_r );

  private:
    /** Const access to implementation. */
    pool::PoolTraits::Impl_constPtr _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates ResPool Stream output */
  std::ostream & operator<<( std::ostream & str, const ResPool & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESPOOL_H
