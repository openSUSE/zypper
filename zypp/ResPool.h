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
#include "zypp/ResFilters.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ResPool
  //
  /** Access to ResObject pool.
   *
   * \note Filter iterators provided by ResPool are intended to
   * operate on internal index tables for faster access. If the
   * the index is not yet implemented, they are realized as
   * an ordinary filter iterator. Do not provide filter iterators
   * here, if there is no index table for it.
  */
  class ResPool
  {
    friend std::ostream & operator<<( std::ostream & str, const ResPool & obj );

  public:
    /** \ref zypp::pool::PoolItem */
    typedef pool::PoolTraits::Item           Item;
    typedef pool::PoolTraits::size_type      size_type;
    typedef pool::PoolTraits::const_iterator const_iterator;
    typedef pool::PoolTraits::const_indexiterator const_indexiterator;
    typedef pool::PoolTraits::const_nameiterator const_nameiterator;

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

    /** \name Iterate through all ResObjects which provide tag_r. */
    //@{
    /** */
    const_indexiterator providesbegin(const std::string & tag_r) const;
    /** */
    const_indexiterator providesend(const std::string & tar_r) const;
    //@}

    /** \name Iterate through all ResObjects which require tag_r. */
    //@{
    /** */
    const_indexiterator requiresbegin(const std::string & tag_r) const;
    /** */
    const_indexiterator requiresend(const std::string & tar_r) const;
    //@}

    /** \name Iterate through all ResObjects which conflict tag_r. */
    //@{
    /** */
    const_indexiterator conflictsbegin(const std::string & tag_r) const;
    /** */
    const_indexiterator conflictsend(const std::string & tar_r) const;
    //@}

    /** \name Iterate through all ResObjects with name tag_r. */
    //@{
    /** */
    const_nameiterator namebegin(const std::string & tag_r) const;
    /** */
    const_nameiterator nameend(const std::string & tar_r) const;
    //@}

  public:
    /** \name Iterate through all ResObjects of a certain kind. */
    //@{
    typedef resfilter::ByKind ByKind;
    typedef filter_iterator<ByKind,const_iterator> byKind_iterator;

    byKind_iterator byKindBegin( const ResObject::Kind & kind_r ) const
    { return make_filter_begin( ByKind(kind_r), *this ); }

    template<class _Res>
      byKind_iterator byKindBegin() const
      { return make_filter_begin( resfilter::byKind<_Res>(), *this ); }


    byKind_iterator byKindEnd( const ResObject::Kind & kind_r ) const
    { return make_filter_end( ByKind(kind_r), *this ); }

    template<class _Res>
      byKind_iterator byKindEnd() const
      { return make_filter_end( resfilter::byKind<_Res>(), *this ); }
    //@}

  public:
    /** \name Iterate through all ResObjects with a certain name (all kinds). */
    //@{
    typedef resfilter::ByName ByName;
    typedef filter_iterator<ByName,const_iterator> byName_iterator;

    byName_iterator byNameBegin( const std::string & name_r ) const
    { return make_filter_begin( ByName(name_r), *this ); }

    byName_iterator byNameEnd( const std::string & name_r ) const
    { return make_filter_end( ByName(name_r), *this ); }
    //@}

 public:
   /** \name Iterate through all ResObjects which have at least
    *  one Capability with index \a index_r in dependency \a depType_r.
   */
   //@{
   typedef resfilter::ByCapabilityIndex ByCapabilityIndex;
   typedef filter_iterator<ByCapabilityIndex,const_iterator> byCapabilityIndex_iterator;

   byCapabilityIndex_iterator byCapabilityIndexBegin( const std::string & index_r, Dep depType_r ) const
   { return make_filter_begin( ByCapabilityIndex(index_r,depType_r), *this ); }

   byCapabilityIndex_iterator byCapabilityIndexEnd( const std::string & index_r, Dep depType_r ) const
   { return make_filter_end( ByCapabilityIndex(index_r,depType_r), *this ); }
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

  /** \todo rename class and eliminate typedef. */
  typedef ResPool ResPool_Ref;

  ///////////////////////////////////////////////////////////////////

  /** \relates ResPool Stream output */
  std::ostream & operator<<( std::ostream & str, const ResPool & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_RESPOOL_H
