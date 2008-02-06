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

  namespace pool
  {
    class PoolImpl;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : PoolItem
  //
  /** Reference to a PoolItem connecting ResObject and ResStatus.
   *
   * The "real" PoolItem is usg. somewhere in the ResPool. This is
   * a reference to it. All copies made will reference (and modify)
   * the same PoolItem. All changes via a PoolItem are immediately
   * visible in all copies (now COW).
   *
   * \note Constness: Like pointer types, a <tt>const PoolItem</tt>
   * does \b not refer to a <tt>const PoolItem</tt>. The reference is
   * \c const, i.e. you can't change the refered PoolItem. The PoolItem
   * (i.e. the status) is always mutable.
   *
  */
  class PoolItem
  {
    friend std::ostream & operator<<( std::ostream & str, const PoolItem & obj );

    public:
      /** Implementation */
      class Impl;

    public:
      /** Default ctor for use in std::container. */
      PoolItem();

      /** Dtor */
      ~PoolItem();

    public:
      /** Returns the current status. */
      ResStatus & status() const;

      /** Reset status (applies autoprotection). */
      ResStatus & statusReset() const;
      
      /** Returns the ResObject::constPtr.
       * \see \ref operator->
       */
      ResObject::constPtr resolvable() const;

      sat::Solvable satSolvable() const
      { return resolvable() ? resolvable()->satSolvable() : sat::Solvable::nosolvable; }

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
      friend class pool::PoolImpl;
      /** ctor */
      explicit PoolItem( const sat::Solvable & solvable_r );
      /** Pointer to implementation */
      RW_pointer<Impl> _pimpl;

    private:
      /** \name tmp hack for save/restore state. */
      /** \todo get rid of it. */
      //@{
      friend class PoolItemSaver;
      void saveState() const;
      void restoreState() const;
      bool sameState() const;
      //@}
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates PoolItem Stream output */
  std::ostream & operator<<( std::ostream & str, const PoolItem & obj );

  /** \relates PoolItem */
  inline bool operator==( const PoolItem & lhs, const PoolItem & rhs )
  { return lhs.resolvable() == rhs.resolvable(); }

  /** \relates PoolItem */
  inline bool operator==( const PoolItem & lhs, const ResObject::constPtr & rhs )
  { return lhs.resolvable() == rhs; }

  /** \relates PoolItem */
  inline bool operator==( const ResObject::constPtr & lhs, const PoolItem & rhs )
  { return lhs == rhs.resolvable(); }


  /** \relates PoolItem */
  inline bool operator!=( const PoolItem & lhs, const PoolItem & rhs )
  { return ! (lhs==rhs); }

  /** \relates PoolItem */
  inline bool operator!=( const PoolItem & lhs, const ResObject::constPtr & rhs )
  { return ! (lhs==rhs); }

  /** \relates PoolItem */
  inline bool operator!=( const ResObject::constPtr & lhs, const PoolItem & rhs )
  { return ! (lhs==rhs); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace std
{ /////////////////////////////////////////////////////////////////

  /** \relates zypp::PoolItem Order in std::container follows ResObject::constPtr.*/
  template<>
    inline bool less<zypp::PoolItem>::operator()( const zypp::PoolItem & lhs, const zypp::PoolItem & rhs ) const
    { return lhs.resolvable() < rhs.resolvable(); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_POOLITEM_H
