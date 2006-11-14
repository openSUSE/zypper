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
   * \code
   * // Returns wheter willing to collect more items.
   * bool consume( const CapAndItem & cai_r );
   *
   * ResPool    _pool;
   * Capability _cap;
   * // Invoke consume on all provides that match _cap
   * forEachMatchIn( _pool, Dep::PROVIDES, _cap, consume );
   * \endcode
   * \relates ResPool.
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

  //@}

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPMATCHHELPER_H
