/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/Capability.h
 *
*/
#ifndef ZYPP_CAPABILITY_H
#define ZYPP_CAPABILITY_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"

#include "zypp/ResolvableFwd.h"
#include "zypp/SolverContextFwd.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////
    DEFINE_PTR_TYPE(CapabilityImpl);
    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Capability
  //
  /** */
  class Capability
  {
    typedef capability::CapabilityImplPtr      ImplPtr;
    typedef capability::constCapabilityImplPtr constImplPtr;

  public:
    /** Factory ctor */
    explicit
    Capability( ImplPtr impl_r );
    /** Dtor */
    virtual ~Capability();

  public:
    /**  */
    bool matches( constResolvablePtr resolvable_r,
                  const SolverContext & colverContext_r );
    /**  */
    bool matches( constResolvablePtr resolvable_r );

  private:
    /** Pointer to implementation */
    ImplPtr _pimpl;
  public:
    /** Avoid a bunch of friend decl. */
    constImplPtr sayFriend() const;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Capability Stream output */
  extern std::ostream & operator<<( std::ostream & str, const Capability & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPABILITY_H
