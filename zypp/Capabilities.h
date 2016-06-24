/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/sat/Capabilities.h
 *
*/
#ifndef ZYPP_SAT_CAPABILITIES_H
#define ZYPP_SAT_CAPABILITIES_H

#include <iosfwd>

#include "zypp/base/DefaultIntegral.h"
#include "zypp/sat/detail/PoolMember.h"
#include "zypp/Capability.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Capabilities
  //
  /** Container of \ref Capability (currently read only).
   *
   * \note libsolv dependency lists may include internal ids
   * which must be skipped on iteration or size calculation
   * (\see \ref detail::isDepMarkerId).
   */
  class Capabilities
  {
    public:
      typedef Capability value_type;
      typedef unsigned   size_type;

      enum Mode { SKIP_TO_INTERNAL };

    public:
      /** Default ctor */
      Capabilities()
      : _begin( 0 )
      {}

      /** Ctor from Id pointer (friend \ref Solvable). */
      explicit
      Capabilities( const sat::detail::IdType * base_r )
      : _begin( base_r )
      {}

      /** Ctor from Id pointer (friend \ref Solvable).
       * Jump behind skip_r (e.g. behind prereqMarker).
       */
      Capabilities( const sat::detail::IdType * base_r, sat::detail::IdType skip_r );

    public:
      /** Whether the container is empty. */
      bool empty() const
      { return ! ( _begin && *_begin ); }

      /** Number of capabilities inside. */
      size_type size() const;

    public:
      class const_iterator;

      /** Iterator pointing to the first \ref Capability. */
      const_iterator begin() const;

      /** Iterator pointing behind the last \ref Capability. */
      const_iterator end() const;

  public:
    /** Return whether \a lhs matches at least one capability in set. */
    bool matches( const Capability & lhs ) const;

    private:
      const sat::detail::IdType * _begin;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Capabilities Stream output */
  std::ostream & operator<<( std::ostream & str, const Capabilities & obj );

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Capabilities::const_iterator
  //
  /** \ref Capabilities iterator.
   */
  class Capabilities::const_iterator : public boost::iterator_adaptor<
        const_iterator                   // Derived
      , const sat::detail::IdType *      // Base
      , const Capability                 // Value
      , boost::forward_traversal_tag     // CategoryOrTraversal
      , const Capability                 // Reference
      >
  {
    public:
      const_iterator()
      : const_iterator::iterator_adaptor_( 0 )
      {}

      explicit const_iterator( const sat::detail::IdType * _idx )
      : const_iterator::iterator_adaptor_( _idx )
      {
        if ( base_reference() && sat::detail::isDepMarkerId( *base_reference() ) )
        {
          _tagged = true;
          ++base_reference();
        }
      }

    public:
      /** Return \c true if the \ref Capability is \c tagged.
       * The meaning of \c tagged depends on the kind of dependency you
       * are processing. It is a hint that the iteratir skipped some
       * internal marker, indicating that subsequent cabailities have
       * a special property. Within a \ref Solvables requirements e.g.
       * the pre-requirements are tagged.
       * \code
       * Capabilities req( solvable.requires() );
       * for_( it, req.begin(), req.end() )
       * {
       *   if ( it.tagged() )
       *     cout << *it << " (is prereq)" << endl;
       *   else
       *     cout << *it << endl;
       * }
       * \endcode
      */
      bool tagged() const { return _tagged; }

    private:
      friend class boost::iterator_core_access;

      reference dereference() const
      { return ( base() ) ? Capability( *base() ) : Capability::Null; }

      template <class OtherDerived, class OtherIterator, class V, class C, class R, class D>
      bool equal( const boost::iterator_adaptor<OtherDerived, OtherIterator, V, C, R, D> & rhs ) const
      { // NULL pointer is eqal pointer to Id 0
        return ( base() == rhs.base() // includes both NULL...
                 || ( !rhs.base() && !*base()     )
                 || ( !base()     && !*rhs.base() ) );
      }

      void increment()
      { // jump over libsolvs internal ids.
        if ( sat::detail::isDepMarkerId( *(++base_reference()) ) )
        {
          _tagged = true;
          ++base_reference();
        }
      }

    private:
      DefaultIntegral<bool,false> _tagged;
  };
  ///////////////////////////////////////////////////////////////////

  inline Capabilities::const_iterator Capabilities::begin() const
  { return const_iterator( _begin ); }

  inline Capabilities::const_iterator Capabilities::end() const
  { return const_iterator( 0 ); }

  inline bool Capabilities::matches( const Capability & lhs ) const
  {
    for ( const Capability & rhs : *this )
      if ( lhs.matches( rhs ) == CapMatch::yes )
	return true;
      return false;
  }
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SAT_CAPABILITIES_H
