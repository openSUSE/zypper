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

#include "zypp/capability/CapTraits.h"

#include "zypp/Resolvable.h" // maybe ResTraits are sufficient?
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

      typedef CapTraitsBase::KindType  Kind;

    public:
      /** Ctor taking the kind of Resolvable \c this refers to.*/
      CapabilityImpl( const Resolvable::Kind & refers_r );

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

      /** More or less human readable representation as string. */
      virtual std::string asString() const = 0;

      /** Usg. string representation without edition range. */
      std::string index() const
      { return value(); }

      /** \todo check it. */
      std::string encode() const
      { return asString(); }

    protected: // Match helpers
      /** Implementation dependent value. */
      virtual std::string value() const
      { return asString(); }

      /** Implementation dependent value. */
      virtual Edition::Range editionRange() const
      { return Edition::Range(); }

      bool sameKind( const constPtr & rhs ) const
      { return kind() == rhs->kind(); }

      bool sameRefers( const constPtr & rhs ) const
      { return _refers == rhs->_refers; }

      bool sameKindAndRefers( const constPtr & rhs ) const
      { return sameKind( rhs ) && sameRefers( rhs ); }

      /** Match by value if \a condition_r is \c true. */
      bool matchValueIf( bool condition_r, const constPtr & rhs ) const
      { return condition_r && value() == rhs->value(); }

      /** Match by value. */
      bool matchValue( const constPtr & rhs ) const
      { return matchValueIf( true, rhs ); }

      /** Match by editionRange if \a condition_r is \c true. */
      bool matchEditionRangeIf( bool condition_r, const constPtr & rhs ) const
      { return condition_r && editionRange().overlaps( rhs->editionRange() ); }

      /** Match by editionRange. */
      bool matchEditionRange( const constPtr & rhs ) const
      { return matchEditionRangeIf( true, rhs ); }

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
       * \todo make it pure virt?
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
