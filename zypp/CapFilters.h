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
    typedef std::unary_function<const Capability &, bool> CapabilityFilterFunctor;

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

    //@}
    /////////////////////////////////////////////////////////////////
  } // namespace capfilter
  ///////////////////////////////////////////////////////////////////^
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPFILTERS_H
