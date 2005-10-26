/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/ReferenceCounted.h
 *
*/
#ifndef ZYPP_BASE_REFERENCECOUNTED_H
#define ZYPP_BASE_REFERENCECOUNTED_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace base
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ReferenceCounted
    //
    /** Base class for reference counted objects.
     * \todo Define exceptions.
     * \todo Make counter thread safe.
     * \todo get rid of base namesapace.
    */
    class ReferenceCounted
    {
    public:
      /** Default ctor.
       * Initial reference count is zero.
      */
      ReferenceCounted()
      : _counter( 0 )
      {}

      /** Copy ctor.
       * Initial reference count is zero.
      */
      ReferenceCounted( const ReferenceCounted & rhs )
      : _counter( 0 )
      {}

      /** Dtor.
       * \throw INTERNAL if reference count is not zero.
      */
      virtual ~ReferenceCounted()
      { if ( _counter ) throw( "~ReferenceCounted: nonzero reference count" ); }

      /** Assignment.
       * Reference count remains untouched.
      */
      ReferenceCounted & operator=( const ReferenceCounted & )
      { return *this; }

    public:
      /** Reurn reference counter value. */
      unsigned refCount() const
      { return _counter; }

      /** Add a reference. */
      void ref() const
      { ++_counter; }

      /** Release a reference.
       * Deletes the object if reference count gets zero.
       * \throw INTERNAL if reference count is zero.
      */
      void unref() const
      {
        if ( !_counter )
          throw( "ReferenceCounted::unref: zero reference count" );
        if ( --_counter == 0 )
          delete this;
      }

      /** Called by zypp::base::intrusive_ptr to add a reference.
       * \see ZYPP_BASE_SMART_PTR
      */
      static void add_ref( const ReferenceCounted * ptr_r )
      { if( ptr_r ) ptr_r->ref(); }

      /** Called by zypp::base::intrusive_ptr to add a reference.
       * \see ZYPP_BASE_SMART_PTR
      */
      static void release( const ReferenceCounted * ptr_r )
      { if( ptr_r ) ptr_r->unref(); }

    private:
      /** The reference counter. */
      mutable unsigned _counter;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#define IMPL_PTR_TYPE(NAME) \
void intrusive_ptr_add_ref( const NAME * ptr_r )               \
{ zypp::base::ReferenceCounted::add_ref( ptr_r ); }                  \
void intrusive_ptr_release( const NAME * ptr_r )               \
{ zypp::base::ReferenceCounted::release( ptr_r ); }

///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_REFERENCECOUNTED_H
