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

    /** Ordering for use in CapSet */
    friend class CapOrder;
    friend bool operator==( const Capability & lhs, const Capability & rhs );
    friend std::ostream & operator<<( std::ostream & str, const Capability & obj );

  private:
    typedef capability::CapabilityImpl          Impl;
    typedef capability::CapabilityImpl_Ptr      Impl_Ptr ;
    typedef capability::CapabilityImpl_constPtr Impl_constPtr;
    /** Factory ctor */
    explicit
    Capability( Impl_Ptr impl_r );
  public:
    /** Factory */
    typedef CapFactory Factory;
    /** Dtor */
    virtual ~Capability();

  public:
    /** Kind of Resolvable the Capability refers to. */
    const Resolvable::Kind & refers() const;
    /** More or less human readable representation as string. */
    std::string asString() const;
    /** */
    bool matches( Resolvable::constPtr resolvable_r,
                  const SolverContext & colverContext_r ) const;
    /**  */
    bool matches( Resolvable::constPtr resolvable_r ) const;

  private:
    /** Pointer to implementation */
    base::RW_pointer<Impl,Impl_Ptr> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** Ordering relation used by ::CapSet. */
  struct CapOrder : public std::binary_function<Capability, Capability, bool>
  {
    bool operator()( const Capability & lhs, const Capability & rhs ) const
    { return lhs._pimpl.get() < rhs._pimpl.get(); }
  };

  ///////////////////////////////////////////////////////////////////

  /** \relates Capability  */
  inline bool operator==( const Capability & lhs, const Capability & rhs )
  { return lhs._pimpl.get() == rhs._pimpl.get(); }

  /** \relates Capability  */
  inline bool operator!=( const Capability & lhs, const Capability & rhs )
  { return ! (lhs == rhs); }

  /** \relates Capability Stream output */
  extern std::ostream & operator<<( std::ostream & str, const Capability & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPABILITY_H
