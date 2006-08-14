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
    typedef pool::PoolTraits::Item		         Item;
    typedef pool::PoolTraits::size_type		         size_type;
    typedef pool::PoolTraits::const_iterator	         const_iterator;
    typedef pool::PoolTraits::byName_iterator            byName_iterator;
    typedef pool::PoolTraits::byCapabilityIndex_iterator byCapabilityIndex_iterator;
    typedef pool::PoolTraits::AdditionalCapSet		 AdditionalCapSet;

  public:
    /** Default ctor: empty pool */
    ResPool();
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
    byName_iterator byNameBegin( const std::string & name_r ) const;

    byName_iterator byNameEnd( const std::string & name_r ) const;
    //@}

 public:
   /** \name Iterate through all ResObjects which have at least
    *  one Capability with index \a index_r in dependency \a depType_r.
   */
   //@{
   byCapabilityIndex_iterator byCapabilityIndexBegin( const std::string & index_r, Dep depType_r ) const;

   byCapabilityIndex_iterator byCapabilityIndexEnd( const std::string & index_r, Dep depType_r ) const;
   //@}

 public:
   /** \name Handling addition capabilities in the pool in order for solving it in
    *  a solver run. This is used for tasks like needing a package with the name "foo".
    *  The solver has to evaluate a proper package by his own.
    *
    *  CAUTION: This has another semantic in the solver. The required resolvable has
    *  been set for installation (in the pool) only AFTER a solver run.
   */

   /**
    *  Handling additional requirement. E.G. need package "foo" and package 
    *  "foo1" which has a greater version than 1.0:
    *
    *  CapSet capset;
    *  capset.insert (CapFactory().parse( ResTraits<Package>::kind, "foo"));    
    *  capset.insert (CapFactory().parse( ResTraits<Package>::kind, "foo1 > 1.0"));
    *
    *  // The user is setting this capablility
    *  ResPool::AdditionalCapSet aCapSet;
    *  aCapSet[ResStatus::USER] = capset;
    *
    *  setAdditionalRequire( aCapSet );
    */
   void setAdditionalRequire( const AdditionalCapSet & capset ) const;
   AdditionalCapSet & additionalRequire() const;

   /**
    *  Handling additional conflicts. E.G. do not install anything which provides "foo":
    *
    *  CapSet capset;    
    *  capset.insert (CapFactory().parse( ResTraits<Package>::kind, "foo"));
    *
    *  // The user is setting this capablility
    *  ResPool::AdditionalCapSet aCapSet;
    *  aCapSet[ResStatus::USER] = capset;
    *
    *  setAdditionalConflict( aCapSet );    
    */      
   void setAdditionalConflict( const AdditionalCapSet & capset ) const;
   AdditionalCapSet & additionaConflict() const;
      
   /**
    *  Handling additional provides. This is used for ignoring a requirement.
    *  e.G. Do ignore the requirement "foo":
    *
    *  CapSet capset;    
    *  capset.insert (CapFactory().parse( ResTraits<Package>::kind, "foo"));
    *
    *  // The user is setting this capablility
    *  ResPool::AdditionalCapSet aCapSet;
    *  aCapSet[ResStatus::USER] = capset;
    *
    *  setAdditionalProvide( aCapSet );    
    */      
   void setAdditionalProvide( const AdditionalCapSet & capset ) const;
   AdditionalCapSet & additionaProvide() const;                  

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
