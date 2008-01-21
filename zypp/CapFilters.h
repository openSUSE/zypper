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

#include "zypp/base/Deprecated.h"
#include "zypp/base/Functional.h"
#include "zypp/Capability.h"
#include "zypp/ResObject.h"

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
    struct ZYPP_DEPRECATED ByRefers : public CapabilityFilterFunctor
    {
      ZYPP_DEPRECATED bool operator()( const Capability & c ) const
      {
        return false; //c.refers() == _refers;
      }

      ZYPP_DEPRECATED ByRefers( Resolvable::Kind refers_r )
      : _refers( refers_r )
      {}
      ZYPP_DEPRECATED ByRefers( ResObject::constPtr p )
      : _refers( p->kind() )
      {}
      ZYPP_DEPRECATED ByRefers( const Capability & cap_r )
      //: _refers( cap_r.refers() )
      {}
      Resolvable::Kind _refers;
    };

    /** */
    struct ZYPP_DEPRECATED ByIndex : public CapabilityFilterFunctor
    {
      ZYPP_DEPRECATED bool operator()( const Capability & c ) const
      {
        return false; //c.index() == _index;
      }

      ZYPP_DEPRECATED ByIndex( const std::string & index_r )
      : _index( index_r )
      {}
      ZYPP_DEPRECATED ByIndex( const Capability & cap_r )
      //: _index( cap_r.index() )
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
      Capability _lhs;
      CapMatch   _expect;
    };

    //@}
    /////////////////////////////////////////////////////////////////
  } // namespace capfilter
  ///////////////////////////////////////////////////////////////////^
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPFILTERS_H
