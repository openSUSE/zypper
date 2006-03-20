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
     * \todo Make counter thread safe.
    */
    class ReferenceCounted
    {
      /** Stream output via dumpOn. */
      friend std::ostream & operator<<( std::ostream & str, const ReferenceCounted & obj );

    public:
      /** Default ctor.
       * Initial reference count is zero.
      */
      ReferenceCounted();

      /** Copy ctor.
       * Initial reference count is zero.
      */
      ReferenceCounted( const ReferenceCounted & rhs );

      /** Dtor.
       * \throw std::out_of_range if reference count is not zero.
      */
      virtual ~ReferenceCounted();

      /** Assignment.
       * Reference count remains untouched.
      */
      ReferenceCounted & operator=( const ReferenceCounted & )
      { return *this; }

    public:
      /** Return reference counter value. */
      unsigned refCount() const
      { return _counter; }

      /** Add a reference. */
      void ref() const
      { ref_to( ++_counter ); }

      /** Release a reference.
       * Deletes the object if reference count gets zero.
       * \throw std::out_of_range if reference count is zero.
      */
      void unref() const
      {
        if ( !_counter )
          unrefException(); // will throw!
        if ( --_counter )
          unref_to( _counter );
        else
          delete this;
      }

      /** Called by zypp::intrusive_ptr to add a reference.
       * \see ZYPP_SMART_PTR
      */
      static void add_ref( const ReferenceCounted * ptr_r )
      { if( ptr_r ) ptr_r->ref(); }

      /** Called by zypp::intrusive_ptr to add a reference.
       * \see ZYPP_SMART_PTR
      */
      static void release( const ReferenceCounted * ptr_r )
      { if( ptr_r ) ptr_r->unref(); }

    protected:
      /** Overload to realize std::ostream & operator\<\<. */
      virtual std::ostream & dumpOn( std::ostream & str ) const;

      /** Trigger derived classes after refCount was increased. */
      virtual void ref_to( unsigned /* rep_cnt_r */ ) const {}

      /** Trigger derived classes after refCount was decreased.
       * No trigger is sent, if refCount got zero (i.e. the
       * object is deleted).
       **/
      virtual void unref_to( unsigned /* rep_cnt_r */ ) const {}

    private:
      /** The reference counter. */
      mutable unsigned _counter;

      /** Throws Exception on unref. */
      void unrefException() const;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates ReferenceCounted intrusive_ptr hook to add_ref. */
    inline void intrusive_ptr_add_ref( const ReferenceCounted * ptr_r )
    { ReferenceCounted::add_ref( ptr_r ); }

    /** \relates ReferenceCounted intrusive_ptr hook to release. */
    inline void intrusive_ptr_release( const ReferenceCounted * ptr_r )
    { ReferenceCounted::release( ptr_r ); }

    /** \relates ReferenceCounted Stream output. */
    inline std::ostream & operator<<( std::ostream & str, const ReferenceCounted & obj )
    { return obj.dumpOn( str ); }

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
