/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* You have been warned!
*
*/
#ifndef ZYPP_NG_BASE_BASE_H_INCLUDED
#define ZYPP_NG_BASE_BASE_H_INCLUDED

#include <zypp/zyppng/base/zyppglobal.h>
#include <zypp/zyppng/base/signals.h>
#include <memory>
#include <unordered_set>
#include <vector>
#include <thread>

namespace zyppng {

  class BasePrivate;

  /*!
   * The Base class is used as a common base class for objects that emit signals,
   * it also supports a parent/child relationship where the parent object tracks keeps
   * a reference for all its children.
   */
  class LIBZYPP_NG_EXPORT Base : public sigc::trackable, public std::enable_shared_from_this<Base>
  {
    NON_COPYABLE(Base);
    ZYPP_DECLARE_PRIVATE(Base)
  public:

    using Ptr = std::shared_ptr<Base>;
    using WeakPtr = std::weak_ptr<Base>;

    Base ();
    virtual ~Base();

    /*!
     * Returns the parent object if there is one, otherwise
     * returns a zero WeakPtr
     */
    WeakPtr parent() const;

    /*!
     * Adds a new object to the child list, the object
     * will keep a reference for its entire lifetime or until the object is removed
     */
    void addChild ( Base::Ptr child );

    /*!
     * Removes a child object from the internal child list
     */
    void removeChild (Ptr child );

    /*!
     * Returns all child objects of this object
     */
    const std::unordered_set<Ptr> &children() const;

    /*!
     * Returns the thread ID this object was created in
     */
    std::thread::id threadId () const;

    /*!
     * Returns all children that can be casted to type T
     */
    template<typename T>
    std::vector< std::weak_ptr<T> > findChildren () const {
      std::vector< std::weak_ptr<T> > result;
      for ( Ptr p : children() ) {
        std::shared_ptr<T> casted = std::dynamic_pointer_cast<T>(p);
        if ( casted )
          result.push_back( std::weak_ptr<T>(casted) );
      }
      return result;
    }

    template<typename T>
    inline std::shared_ptr<T> shared_this () const {
      return std::static_pointer_cast<T>( shared_from_this() );
    }

    template<typename T>
    inline std::shared_ptr<T> shared_this () {
      return std::static_pointer_cast<T>( shared_from_this() );
    }

    template<typename T>
    inline std::weak_ptr<T> weak_this () const {
      return std::static_pointer_cast<T>( weak_from_this().lock() );
    }

    template<typename T>
    inline std::weak_ptr<T> weak_this () {
      return std::static_pointer_cast<T>( weak_from_this().lock() );
    }

  protected:
    Base ( BasePrivate &dd );
    std::unique_ptr<BasePrivate> d_ptr;
  };

} // namespace zyppng

#endif // ZYPP_NG_CORE_BASE_H_INCLUDED
