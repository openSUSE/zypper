/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/base/PtrTypes.h
 *  \ingroup ZYPP_BASE_SMART_PTR
 *  \see ZYPP_BASE_SMART_PTR
*/
#ifndef ZYPP_BASE_PTRTYPES_H
#define ZYPP_BASE_PTRTYPES_H

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/intrusive_ptr.hpp>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace base
  { /////////////////////////////////////////////////////////////////

    /** \defgroup ZYPP_BASE_SMART_PTR ZYPP_BASE_SMART_PTR
     *  Smart pointer types.
     *
     * Namespace zypp::base provides 3 smart pointer types \b using the
     * boost smart pointer library.
     *
     * \li \c scoped_ptr Simple sole ownership of single objects. Noncopyable.
     *
     * \li \c shared_ptr Object ownership shared among multiple pointers
     *
     * \li \c weak_ptr Non-owning observers of an object owned by shared_ptr.
    */
    /*@{*/

    /** */
    using boost::scoped_ptr;

    /** */
    using boost::shared_ptr;

    /** */
    using boost::weak_ptr;

    /** Use boost::intrusive_ptr as Ptr type*/
    using boost::intrusive_ptr;
    using boost::static_pointer_cast;
    using boost::const_pointer_cast;
    using boost::dynamic_pointer_cast;

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ImplPtr
    //
    /** Wrapper for \c const correct access via \ref ZYPP_BASE_SMART_PTR.
     *
     * zypp::base::ImplPtr<tt>\<_D,_Ptr></tt> stores a \ref ZYPP_BASE_SMART_PTR
     * of type \c _Ptr, which must be convertible into a <tt>_D *</tt>. Pointer
     * style access (via \c -> and \c *) offers a <tt>const _D *</tt> in const
     * a context, otherwise a <tt>_D *</tt>.
     *
     * Forwarding access from an interface to an implemantation class, an
     * ImplPtr prevents const interface methods from accidentally calling
     * nonconst implementation methods. In case you have to do so, call
     * unconst to get the <tt>_D *</tt>.
     *
     * The second template argument defaults to <tt>_Ptr = scoped_ptr<_D></tt>.
     *
     * \todo refine ctor and assign.
    */
    template<class _D, class _Ptr = scoped_ptr<_D> >
      struct ImplPtr
      {
        typedef _D element_type;

        explicit
        ImplPtr( typename _Ptr::element_type * dptr = 0 )
        : _dptr( dptr )
        {}

        _D & operator*() { return *_dptr; }
        const _D & operator*() const { return *_dptr; };
        _D * operator->() { return _dptr.get(); }
        const _D * operator->() const { return _dptr.get(); }

        _D * unconst() const { return _dptr.get(); }


        _Ptr _dptr;
      };
    ///////////////////////////////////////////////////////////////////
    /** Wrapper for \c const correct access via pointer.
     *
     * Template specialization of ImplPtr, storing a raw <tt>_P *</tt>,
     * which must be convertible into a <tt>_D *</tt>.
     *
     * \note The object pointed to will \b not be deleted. If you need
     * automatic cleanup, use a \ref ZYPP_BASE_SMART_PTR instead of a
     * raw pointer.
    */
    template<class _D,class _P>
      struct ImplPtr<_D,_P*>
      {
        typedef _D element_type;

        explicit
        ImplPtr( _P * dptr = 0 )
        : _dptr( dptr )
        {}

        _D & operator*() { return *_dptr; }
        const _D & operator*() const { return *_dptr; };
        _D * operator->() { return _dptr; }
        const _D * operator->() const { return _dptr; }

        _D * unconst() const { return _dptr; }


        _P * _dptr;
      };
    /////////////////////////////////////////////////////////////////

    /*@}*/

    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

/** Forward declaration of Ptr types */
#define DEFINE_PTR_TYPE(NAME) \
class NAME;                                                      \
extern void intrusive_ptr_add_ref( const NAME * );               \
extern void intrusive_ptr_release( const NAME * );               \
typedef zypp::base::intrusive_ptr<NAME>       NAME##Ptr;         \
typedef zypp::base::intrusive_ptr<const NAME> const##NAME##Ptr;

///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_PTRTYPES_H
