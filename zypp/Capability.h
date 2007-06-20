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
#include "zypp/capability/CapTraits.h"
#include "zypp/Resolvable.h"
#include "zypp/CapMatch.h"

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
   *
   * Capability is created by a Factory class. Only a default ctor
   * creating a dummy capability is provided.
   * \code
   *   Capability cap;
   *   try
   *     {
   *       cap = CapFactory().parse( ResTraits<Patch>::kind,
   *                                 parsed.name,
   *                                 parsed.op,
   *                                 Edition( parsed.ver,
   *                                          parsed.rel,
   *                                          parsed.epoch ) );
   *     }
   *   catch ( Exception & excpt_r )
   *     {
   *       ERR << excpt_r << endl;
   *       ... Or maybe just WAR, or ?
   *     }
   * \endcode
   * \see CapFactory: Factory creating Capability.
   *
   * \invariant Nonzero \c _pimpl
   * \invariant Unified \c _pimpl asserted by CapFactory.
   *
   * \todo Need a trival return from matches. E.g. Conditional
   * capabilities must be able to indicate that they should be
   * treated as if they were not present at all, if the precondition
   * does no apply. Same for the default Capability.
  */
  class Capability
  {
    /** Ordering for use in CapSet */
    friend class CapOrder;
    friend bool operator==( const Capability & lhs, const Capability & rhs );
    friend std::ostream & operator<<( std::ostream & str, const Capability & obj );

  public:
    /** */
    typedef capability::CapabilityTraits::KindType  Kind;

  public:
    /** DefaultCtor creating \ref noCap. */
    Capability();

    /** Dtor */
    virtual ~Capability();

    /** Constant representing no Capabiliy.
     * It refers to no kind of Resolvable, and matches returns
     *  returns \c CapMatch::irrelevant.
    */
    static const Capability noCap;

  public:
    /** Kind of Capability.  */
    const Kind & kind() const;

    /** Kind of Resolvable the Capability refers to. */
    const Resolvable::Kind & refers() const;

    /** Whether to consider this Capability.
     * Evaluates the Capabilities pre-condition (if any), and
     * returns whether the condition applies. If not, the Capability
     * is to be ignored.
    */
    bool relevant() const;

    /** Return whether the Capabilities match.
     * If either Capability is not \ref relevant, CapMatch::irrelevant
     * is returned.
    */
    CapMatch matches( const Capability & rhs ) const;

    /** More or less human readable representation as string. */
    std::string asString() const;

    /** accessors needed by solver/zmd  */
    /** Deprecated */
    std::string index() const;

  private:
    typedef capability::CapabilityImpl          Impl;
    typedef capability::CapabilityImpl_Ptr      Impl_Ptr ;
    typedef capability::CapabilityImpl_constPtr Impl_constPtr;

    /** Factory */
    friend class CapFactory;

    /** Factory ctor */
    explicit
    Capability( Impl_Ptr impl_r );

  private:
    /** */
    friend class capability::CapabilityImpl;
    /** Pointer to implementation */
    RW_pointer<Impl,rw_pointer::Intrusive<Impl> > _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  template<class _Cap>
    inline bool isKind( const Capability & cap )
    { return cap.kind() == capability::CapTraits<_Cap>::kind; }

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
