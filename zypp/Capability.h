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
#include <functional>

#include "zypp/base/PtrTypes.h"

#include "zypp/Resolvable.h"
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
   * \invariant Unified \c _pimpl asserted by CapFactory
  */
  class Capability
  {
    /** Factory */
    friend class CapFactory;
    /** Ordering for use in std::container */
    friend class CapOrder;
    friend bool operator==( const Capability & lhs, const Capability & rhs );
  private:
    typedef capability::CapabilityImplPtr      ImplPtr;
    typedef capability::constCapabilityImplPtr constImplPtr;
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
    bool matches( Resolvable::constPtr resolvable_r,
                  const SolverContext & colverContext_r ) const;
    /**  */
    bool matches( Resolvable::constPtr resolvable_r ) const;

  private:
    /** Pointer to implementation */
    ImplPtr _pimpl;
  public:
    /** Avoid a bunch of friend decl. */
    constImplPtr sayFriend() const;
  };
  ///////////////////////////////////////////////////////////////////

  struct CapOrder : public std::binary_function<Capability, Capability, bool>
  {
    bool operator()( const Capability & lhs, const Capability & rhs ) const
    { return lhs._pimpl < rhs._pimpl; }
  };

  ///////////////////////////////////////////////////////////////////

  /** \relates Capability  */
  inline bool operator==( const Capability & lhs, const Capability & rhs )
  { return lhs._pimpl == rhs._pimpl; }

  /** \relates Capability  */
  inline bool operator!=( const Capability & lhs, const Capability & rhs )
  { return ! (lhs == rhs); }

  /** \relates Capability Stream output */
  extern std::ostream & operator<<( std::ostream & str, const Capability & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPABILITY_H
