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

#include "zypp/Capability.h"
#include "zypp/Resolvable.h"
#include "zypp/CapMatch.h"

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
    /** Abstract base for Capability implementations.
    */
    class CapabilityImpl : public base::ReferenceCounted, private base::NonCopyable
    {
    public:
      typedef CapabilityImpl           Self;
      typedef CapabilityImpl_Ptr       Ptr;
      typedef CapabilityImpl_constPtr  constPtr;

      typedef CapabilityTraits::KindType  Kind;

    public:
      /** Kind of capabiliy.  */
      virtual const Kind & kind() const = 0;

      /** Kind of Resolvable \c this refers to. */
      const Resolvable::Kind & refers() const
      { return _refers; }

      /** Relevant per default. */
      virtual bool relevant() const
      { return true; }

      /** Return whether the Capabilities match.
       * \note We rely on Capability passing non NULL pointers.
      */
      virtual CapMatch matches( const constPtr & rhs ) const = 0;

      /** The string representation that enables \ref CapFactory
       * to recreate this capability.
       * \todo check it!
      */
      virtual std::string encode() const = 0;

      /** More or less human readable representation as string.
       * Suitable for displaying it at the UI. Defaults to
       * \ref encode.
      */
      virtual std::string asString() const
      { return encode(); }

      /** \deprecated A string representation usg. without
       * edition range. All Capabilities that may match each other
       * must have the same index. That's ugly, but the way the
       * solver currently uses it.
      */
      virtual std::string index() const
      { return encode(); }
      /** \deprecated, defaults to Rel::NONE */
      virtual Rel op() const
      { return Rel::NONE; }
      /** \deprecated, defaults to Edition::noedition */
      virtual Edition edition() const
      { return Edition::noedition; }

    public:
      /** Solver hack. */
      struct SplitInfo
      {
        std::string name;
        std::string path;
      };
      /** Solver hack. */
      static SplitInfo getSplitInfo( const Capability & cap );

      /** Access to Capability details. */
      static constPtr backdoor( const Capability & cap )
      { return cap._pimpl.getPtr(); }

    protected:
      /** Ctor taking the kind of Resolvable \c this refers to.*/
      CapabilityImpl( const Resolvable::Kind & refers_r );

    protected: // Match helpers
      bool sameKind( const constPtr & rhs ) const
      { return kind() == rhs->kind(); }

      bool sameRefers( const constPtr & rhs ) const
      { return _refers == rhs->_refers; }

      bool sameKindAndRefers( const constPtr & rhs ) const
      { return sameKind( rhs ) && sameRefers( rhs ); }

    protected:
      /** Helper for stream output. */
      virtual std::ostream & dumpOn( std::ostream & str ) const;

    private:
      /** Kind of Resolvable \c this refers to. */
      Resolvable::Kind _refers;

    private:
      friend struct CapImplOrder;
      /** Helper for CapImplOrder to define an order relation.
       * \invariant CapImplOrder asserts that \a rhs \c refers
       * and \c kind values are equal to \c this. Implementation
       * may concentrate on the remaining values.
       *
       * Default compares \ref encoded.
      */
      virtual bool capImplOrderLess( const constPtr & rhs ) const;
    };
    ///////////////////////////////////////////////////////////////////

    /** Test whether a CapabilityImpl is of a certain Kind.
     * \code
     * isKind<FileCap>(cap);
     * \endcode
    */
    template<class _Cap>
      inline bool isKind( const CapabilityImpl::constPtr & cap )
      { return cap && cap->kind() == CapTraits<_Cap>::kind; }

    /** Short for dynamic_pointer_cast. */
    template<class _Cap>
      inline intrusive_ptr<const _Cap> asKind( const CapabilityImpl::constPtr & cap )
      { return dynamic_pointer_cast<const _Cap>(cap); }

    /** Access to Capability details. */
    template<class _Cap>
      inline intrusive_ptr<const _Cap> asKind( const Capability & cap )
      { return dynamic_pointer_cast<const _Cap>( CapabilityImpl::backdoor(cap) ); }

    ///////////////////////////////////////////////////////////////////

    /** Ordering relation used by ::CapFactory to unify CapabilityImpl. */
    struct CapImplOrder : public std::binary_function<CapabilityImpl::constPtr, CapabilityImpl::constPtr, bool>
    {
      /** */
      bool operator()( const CapabilityImpl::constPtr & lhs,
                       const CapabilityImpl::constPtr & rhs ) const
      {
        if ( lhs->refers() != rhs->refers() )
          return lhs->refers() < rhs->refers();
        if ( lhs->kind() != rhs->kind() )
          return lhs->kind() < rhs->kind();
        return lhs->capImplOrderLess( rhs );
      }
    };

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPABILITY_CAPABILITYIMPL_H
