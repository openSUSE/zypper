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

#include "zypp/pool/PoolTraits.h"
#include "zypp/base/Iterator.h"
#include "zypp/ResTraits.h"
#include "zypp/ResFilters.h"

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
    /** \ref zypp::pool::PoolItem */
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

  public:
    /** \name Iterate through all ResObjects (all kinds). */
    //@{
    /** */
    const_iterator begin() const;
    /** */
    const_iterator end() const;
    //@}

  public:
    /** \name Iterate through all ResObjects of a certain kind. */
    //@{
    typedef functor::ByKind ByKind;
    typedef filter_iterator<ByKind,const_iterator> byKind_iterator;

    byKind_iterator byKindBegin( const ResObject::Kind & kind_r ) const
    { return make_filter_begin( ByKind(kind_r), *this ); }

    template<class _Res>
      byKind_iterator byKindBegin() const
      { return make_filter_begin( functor::byKind<_Res>(), *this ); }


    byKind_iterator byKindEnd( const ResObject::Kind & kind_r ) const
    { return make_filter_end( ByKind(kind_r), *this ); }

    template<class _Res>
      byKind_iterator byKindEnd() const
      { return make_filter_end( functor::byKind<_Res>(), *this ); }
    //@}

  public:
    /** \name Iterate through all ResObjects with a certain name. */
    //@{
    typedef functor::ByName ByName;
    typedef filter_iterator<ByName,const_iterator> byName_iterator;

    byName_iterator byNameBegin( const std::string & name_r ) const
    { return make_filter_begin( ByName(name_r), *this ); }

    byName_iterator byNameEnd( const std::string & name_r ) const
    { return make_filter_end( ByName(name_r), *this ); }
    //@}

 public:
   /** \name Iterate through dependency tables. */
   //@{
   //@}

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
