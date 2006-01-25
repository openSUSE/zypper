/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/CapFilters.h
 *
*/
#ifndef ZYPP_CAPFILTERS_H
#define ZYPP_CAPFILTERS_H

#include "zypp/base/Functional.h"
#include "zypp/Capability.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capfilter
  { /////////////////////////////////////////////////////////////////

    /** \defgroup CAPFILTERS Filter functors operating on Capability.
     * \ingroup g_Functor
    */
    //@{

    /** */
    typedef std::unary_function<Capability, bool> CapabilityFilterFunctor;

    /** */
    struct ByRefers : public CapabilityFilterFunctor
    {
      bool operator()( const Capability & c ) const
      {
        return c.refers() == _refers;
      }

      ByRefers( Resolvable::Kind refers_r )
      : _refers( refers_r )
      {}
      ByRefers( ResObject::constPtr p )
      : _refers( p->kind() )
      {}
      ByRefers( const Capability & cap_r )
      : _refers( cap_r.refers() )
      {}
      Resolvable::Kind _refers;
    };

    /** */
    struct ByIndex : public CapabilityFilterFunctor
    {
      bool operator()( const Capability & c ) const
      {
        return c.index() == _index;
      }

      ByIndex( const std::string & index_r )
      : _index( index_r )
      {}
      ByIndex( const Capability & cap_r )
      : _index( cap_r.index() )
      {}
      std::string _index;
    };

    /** */
    struct ByCapMatch : public CapabilityFilterFunctor
    {
      bool operator()( const Capability & c ) const
      {
        return _lhs.matches( c ) == _expect;
      }

      ByCapMatch( const Capability & cap_r, CapMatch expect_r = CapMatch::yes )
      : _lhs( cap_r )
      , _expect( expect_r )
      {}
      const Capability & _lhs;
      CapMatch           _expect;
    };

    //@}
    /////////////////////////////////////////////////////////////////
  } // namespace capfilter
  ///////////////////////////////////////////////////////////////////^
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPFILTERS_H
