/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/PoolItem.h
 *
*/
#ifndef ZYPP_POOLITEM_H
#define ZYPP_POOLITEM_H

#include <iosfwd>
#include <functional>

#include "zypp/base/PtrTypes.h"
#include "zypp/ResObject.h"
#include "zypp/ResStatus.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : PoolItem_Ref
  //
  /** Reference to a PoolItem conecting ResObject and ResStatus.
   *
   * The "real" PoolItem is usg. somwhere in the ResPool. This is
   * a reference to it. All copies made will reference (and modify)
   * the same PoolItem. All changes via a PoolItem_Ref are immediately
   * visible in all copies (now COW).
   *
   * \note Constnes: Like pointer types, a <tt>const PoolItem_Ref</tt>
   * does \b not refer to a <tt>const PoolItem</tt>. The reference is
   * \c const, i.e. you can't change the refered PoolItem. The PoolItem
   * (i.e. the status) is always mutable.
   *
  */
  class PoolItem_Ref
  {
    friend std::ostream & operator<<( std::ostream & str, const PoolItem_Ref & obj );

  public:
    /** Implementation  */
    class Impl;

  public:
    /** Default ctor for use in std::container. */
    PoolItem_Ref();

    /** Ctor */
    explicit
    PoolItem_Ref( ResObject::constPtr res_r );

    /** Ctor */
    PoolItem_Ref( ResObject::constPtr res_r, const ResStatus & status_r );

    /** Dtor */
    ~PoolItem_Ref();

  public:
    /** Returns */
    ResStatus & status() const;

    /** Returns the ResObject::constPtr.
     * \see \ref operator->
    */
    ResObject::constPtr resolvable() const;

  public:
    /** Implicit conversion into ResObject::constPtr to
     *  support query filters operating on ResObject.
    */
    operator ResObject::constPtr() const
    { return resolvable(); }

    /** Forward \c -> access to ResObject. */
    ResObject::constPtr operator->() const
    { return resolvable(); }

    /** Conversion to bool to allow pointer style tests
     *  for nonNULL \ref resolvable. */
    operator ResObject::constPtr::unspecified_bool_type() const
    { return resolvable(); }

  private:
    /** Pointer to implementation */
    RW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  typedef PoolItem_Ref PoolItem;

  /** \relates PoolItem_Ref Stream output */
  std::ostream & operator<<( std::ostream & str, const PoolItem_Ref & obj );

  /** \relates PoolItem_Ref */
  inline bool operator==( const PoolItem_Ref & lhs, const PoolItem_Ref & rhs )
  { return lhs.resolvable() == rhs.resolvable(); }

  /** \relates PoolItem_Ref */
  inline bool operator==( const PoolItem_Ref & lhs, const ResObject::constPtr & rhs )
  { return lhs.resolvable() == rhs; }

  /** \relates PoolItem_Ref */
  inline bool operator==( const ResObject::constPtr & lhs, const PoolItem_Ref & rhs )
  { return lhs == rhs.resolvable(); }


  /** \relates PoolItem_Ref */
  inline bool operator!=( const PoolItem_Ref & lhs, const PoolItem_Ref & rhs )
  { return ! (lhs==rhs); }

  /** \relates PoolItem_Ref */
  inline bool operator!=( const PoolItem_Ref & lhs, const ResObject::constPtr & rhs )
  { return ! (lhs==rhs); }

  /** \relates PoolItem_Ref */
  inline bool operator!=( const ResObject::constPtr & lhs, const PoolItem_Ref & rhs )
  { return ! (lhs==rhs); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace std
{ /////////////////////////////////////////////////////////////////

  /** \relates zypp::PoolItem_Ref Order in std::container follows ResObject::constPtr.*/
  template<>
    inline bool less<zypp::PoolItem_Ref>::operator()( const zypp::PoolItem_Ref & lhs, const zypp::PoolItem_Ref & rhs ) const
    { return lhs.resolvable() < rhs.resolvable(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_POOLITEM_H
