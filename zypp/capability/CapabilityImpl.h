/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/capability/CapabilityImpl.h
 *
*/
#ifndef ZYPP_CAPABILITY_CAPABILITYIMPL_H
#define ZYPP_CAPABILITY_CAPABILITYIMPL_H

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"

#include "zypp/ResolvableFwd.h"
#include "zypp/SolverContextFwd.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////
    DEFINE_PTR_TYPE(CapabilityImpl)

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : CapabilityImpl
    //
    /** */
    class CapabilityImpl : public base::ReferenceCounted, private base::NonCopyable
    {
    public:
      /** Dtor */
      virtual ~CapabilityImpl();

    public:
      /**  */
      virtual bool matches( constResolvablePtr resolvable_r,
                            const SolverContext & colverContext_r ) = 0;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates CapabilityImpl Stream output */
    extern std::ostream & operator<<( std::ostream & str, const CapabilityImpl & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPABILITY_CAPABILITYIMPL_H
