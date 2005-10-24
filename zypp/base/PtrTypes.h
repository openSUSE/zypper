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
     * boostsmart pointer library.
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
