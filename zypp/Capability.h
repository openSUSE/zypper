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

  class CapFactory;

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Capability
  //
  /** Resolvable capabilitiy.
   * \invariant Nonzero \c _pimpl
  */
  class Capability
  {
  private:
    typedef capability::CapabilityImplPtr      ImplPtr;
    typedef capability::constCapabilityImplPtr constImplPtr;
    friend class CapFactory;
    /** Factory ctor */
    explicit
    Capability( ImplPtr impl_r );
  public:
    /** Factory */
    typedef CapFactory Factory;
    /** Dtor */
    virtual ~Capability();

  public:
    /**  */
    const ResKind & refers() const;
    /**  */
    std::string asString() const;
    /**  */
    bool matches( constResolvablePtr resolvable_r,
                  const SolverContext & colverContext_r ) const;
    /**  */
    bool matches( constResolvablePtr resolvable_r ) const;

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
