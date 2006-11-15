/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/CapMatchHelper.h
 *
*/
#ifndef ZYPP_CAPMATCHHELPER_H
#define ZYPP_CAPMATCHHELPER_H

#include "zypp/base/Algorithm.h"
#include "zypp/base/Function.h"
#include "zypp/ResPool.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /** Functor testing whether argument matches a certain Capability.
   * \ingroup g_Functor
   * \ingroup CAPFILTERS
  */
  class MatchesCapability
  {
  public:
    MatchesCapability( const Capability & lhs_r )
    : _lhs( lhs_r )
    {}

    bool operator()( const CapAndItem & capitem_r ) const
    { return operator()( capitem_r.cap ); }

    bool operator()( const Capability & rhs_r ) const
    { return( _lhs.matches( rhs_r ) == CapMatch::yes ); }

  private:
    const Capability & _lhs;
  };

  /** \defgroup CAPMATCHHELPER Find matching Capabilities.
   * \ingroup g_Algorithm
  */
  //@{

  /** Algorithm invoking \c action_r on each \ref CapSet entry
   * that matches a given \ref Capability.
  */
  inline int forEachMatchIn( CapSet::const_iterator begin_r,
                             CapSet::const_iterator end_r,
                             const Capability & lhs_r,
                             function<bool(const Capability &)> action_r )
  {
    std::string index( lhs_r.index() );
    return invokeOnEach( begin_r, end_r,
                         MatchesCapability( lhs_r ), // filter
                         action_r );
  }

  /** Algorithm invoking \c action_r on each \ref CapSet entry
   * that matches a given \ref Capability.
  */
  inline int forEachMatchIn( const CapSet & capset_r,
                             const Capability & lhs_r,
                             function<bool(const Capability &)> action_r )
  {
    return invokeOnEach( capset_r.begin(), capset_r.end(),
                         MatchesCapability( lhs_r ), // filter
                         action_r );
  }

  /** Algorithm invoking \c action_r on each matching \ref Capability
   * in a \ref ResPool.
   *
   * \code
   * // Returns wheter willing to collect more items.
   * bool consume( const CapAndItem & cai_r );
   *
   * ResPool    _pool;
   * Capability _cap;
   * // Invoke consume on all provides that match _cap
   * forEachMatchIn( _pool, Dep::PROVIDES, _cap, consume );
   * \endcode
   *
   * \relates ResPool.
   * \see ForEachMatchInPool
  */
  inline int forEachMatchIn( const ResPool & pool_r, const Dep & dep_r,
                             const Capability & lhs_r,
                             function<bool(const CapAndItem &)> action_r )
  {
    std::string index( lhs_r.index() );
    return invokeOnEach( pool_r.byCapabilityIndexBegin( index, dep_r ),
                         pool_r.byCapabilityIndexEnd( index, dep_r ),
                         MatchesCapability( lhs_r ), // filter
                         action_r );
  }

  /** Functor invoking \c action_r on each matching \ref Capability
   *  in a \ref ResPool.
   *
   * Functor is provided to ease using \ref forEachMatchIn as action
   * in other algorithms.
   *
   * \code
   * bool consume( const CapAndItem & cai_r );
   *
   * ResPool  _pool;
   * PoolItem _pi;
   *
   * // Invoke consume on all PoolItems obsoleted by pi.
   * // short: forEachPoolItemMatchedBy( _pool, _pi, Dep::OBSOLETES, consume );
   * for_each( _pi->dep(Dep::OBSOLETES).begin(),
   *           _pi->dep(Dep::OBSOLETES).end(),
   *           ForEachMatchInPool( _pool, Dep::PROVIDES, consume ) );
   *
   * // Invoke consume on all PoolItems obsoleting pi.
   * // short: forEachPoolItemMatching( _pool, Dep::OBSOLETES, _pi, consume );
   * for_each( pi->dep(Dep::PROVIDES).begin(),
   *           pi->dep(Dep::PROVIDES).end(),
   *           ForEachMatchInPool( _pool, Dep::OBSOLETES, consume ) );
   *
   * \endcode
   *
   * \ingroup g_Functor
   * \ingroup CAPFILTERS
   * \relates ResPool.
   * \see forEachPoolItemMatchedBy
   * \see forEachPoolItemMatching
  */
  class ForEachMatchInPool
  {
  public:
    typedef function<bool(const CapAndItem &)> Action;

  public:
    ForEachMatchInPool( const ResPool & pool_r,
                        const Dep &     dep_r,
                        const Action &  action_r )
    : _pool  ( pool_r )
    , _dep   ( dep_r )
    , _action( action_r )
    {}

    bool operator()( const Capability & cap_r ) const
    {
      return( forEachMatchIn( _pool, _dep, cap_r, _action )
              >= 0 ); // i.e. _action did not return false
    }

  private:
    ResPool _pool;
    Dep     _dep;
    Action  _action;
  };

  /** Find all items in a ResPool matched by a certain PoolItems
   *  dependency set.
   *
   * Iterates <tt>poolitem_r->dep(poolitemdep_r)</tt>
   * and invokes \c action_r on each item in \c pool_r,
   * that provides a match.
   * \code
   * bool consume( const CapAndItem & cai_r );
   *
   * ResPool  _pool;
   * PoolItem _pi;
   *
   * // Invoke consume on all PoolItems obsoleted by pi.
   * forEachPoolItemMatchedBy( _pool, _pi, Dep::OBSOLETES, consume );
   * \endcode
   *
   * \note \c action_r is invoked for each matching Capability. So if
   * the same PoolItem provides multiple matches, \c action_r refers
   * to the same PoolItem multiple times. It may as well be that
   * \c poolitem_r provides a matching Capability. Use \ref OncePerPoolItem
   * to compensate this if neccessary.
  */
  inline void forEachPoolItemMatchedBy( const ResPool &  pool_r,
                                        const PoolItem & poolitem_r,
                                        const Dep &      poolitemdep_r,
                                        function<bool(const CapAndItem &)> action_r )
  {
    for_each( poolitem_r->dep(poolitemdep_r).begin(),
              poolitem_r->dep(poolitemdep_r).end(),
              ForEachMatchInPool( pool_r, Dep::PROVIDES, action_r ) );
  }

  /** Find all items in a ResPool matching a certain PoolItem.
   *
   * Iterates <tt>poolitem_r->dep(Dep::PROVIDES)</tt>
   * and invoking \c action_r on each item in \c pool_r,
   * that provides a match.
   * \code
   * bool consume( const CapAndItem & cai_r );
   *
   * ResPool  _pool;
   * PoolItem _pi;
   *
   * // Invoke consume on all PoolItems obsoleting pi.
   * forEachPoolItemMatching( _pool, Dep::OBSOLETES, _pi, consume );
   * \endcode
   *
   * \note \c action_r is invoked for each matching Capability. So if
   * the same PoolItem provides multiple matches, \c action_r refers
   * to the same PoolItem multiple times. It may as well be that
   * \c poolitem_r provides a matching Capability. Use \ref OncePerPoolItem
   * to compensate this if neccessary.
  */
  inline void forEachPoolItemMatching( const ResPool &  pool_r,
                                       const Dep &      pooldep_r,
                                       const PoolItem & poolitem_r,
                                       function<bool(const CapAndItem &)> action_r )
  {
    for_each( poolitem_r->dep(Dep::PROVIDES).begin(),
              poolitem_r->dep(Dep::PROVIDES).end(),
              ForEachMatchInPool( pool_r, pooldep_r, action_r ) );
  }

  /** Functor translating \ref CapAndItem actions into \ref PoolItem
   *  actions avoiding multiple invocations for the same \ref PoolItem.
   *
   * Additionally you may omit invocation of \a action_r for a
   * specific PoolItem.
   *
   * \code
   * bool consume( const CapAndItem & cai_r );
   * bool consumePi( const PoolItem & pi_r );
   *
   * ResPool  _pool;
   * PoolItem _pi;
   *
   * // Invoke consume on all PoolItems obsoleted by pi.
   * // Once for each matching Capability, thus the same PoolItem
   * // might be involved mutiple times.
   * forEachPoolItemMatchedBy( _pool, _pi, Dep::OBSOLETES,
   *                           consume );
   *
   * // Invoke consume on all PoolItems obsoleted by pi.
   * // Once for each PoolItem, still including _pi in case
   * // it provides a match by itself.
   * forEachPoolItemMatchedBy( _pool, _pi, Dep::OBSOLETES,
   *                           OncePerPoolItem( consumePi ) );
   *
   * // Invoke consume on all PoolItems obsoleted by pi.
   * // Once for each PoolItem, omitting invokation for
   * // _pi (in case it obsoletes itself).
   * forEachPoolItemMatchedBy( _pool, _pi, Dep::OBSOLETES,
   *                           OncePerPoolItem( consumePi, _pi ) );
   * \endcode
   * \ingroup g_Functor
   * \ingroup CAPFILTERS
  */
  struct OncePerPoolItem
  {
  public:
    typedef function<bool(const PoolItem &)> Action;

  public:
    OncePerPoolItem( const Action & action_r,
                     const PoolItem & self_r = PoolItem() )
    : _action( action_r )
    , _uset  ( new std::set<PoolItem> )
    {
      if ( self_r )
        _uset->insert( self_r );
    }

    bool operator()( const CapAndItem & cai_r ) const
    {
      if ( _uset->insert( cai_r.item ).second )
        return _action( cai_r.item );
      return true;
    }

  private:
    Action   _action;
    shared_ptr<std::set<PoolItem> > _uset;
  };


  //@}

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPMATCHHELPER_H
