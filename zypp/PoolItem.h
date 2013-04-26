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
#include "zypp/base/SafeBool.h"
#include "zypp/ResObject.h"
#include "zypp/ResStatus.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class ResPool;

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
  class PoolItem : protected base::SafeBool<PoolItem>
  {
    friend std::ostream & operator<<( std::ostream & str, const PoolItem & obj );

    public:
      /** Implementation */
      class Impl;

    public:
      /** Default ctor for use in std::container. */
      PoolItem();

      /** Ctor looking up the \ref sat::Solvable in the \ref ResPool. */
      explicit PoolItem( const sat::Solvable & solvable_r );

      /** Ctor looking up the \ref ResObject in the \ref ResPool. */
      explicit PoolItem( const ResObject::constPtr & resolvable_r );

      /** Dtor */
      ~PoolItem();

    public:
      /** \name Status related methods. */
      //@{
      /** Returns the current status. */
      ResStatus & status() const;

      /** Reset status. */
      ResStatus & statusReset() const;


      /** \name Status validation.
       * Performed for non-packages.
      */
      //@{
      /** No validation is performed for packages. */
      bool isUndetermined() const;

      /** Returns true if the solvable is relevant which means e.g. for patches
       *  that at least one package of the patch is installed.
       */
      bool isRelevant() const;

      /** Whether a relevant items requirements are met. */
      bool isSatisfied() const;

      /** Whether a relevant items requirements are broken. */
      bool isBroken() const;

      /** This includes \c unlocked broken patches, as well as those already
       * selected to be installed (otherwise classified as \c satisfied).
       */
      bool isNeeded() const;

      /** Broken (needed) but locked patches. */
      bool isUnwanted() const;
      //@}

      //@}
    public:
      /** Return the \ref ResPool the item belongs to. */
      ResPool pool() const;

      /** Return the corresponding \ref sat::Solvable. */
      sat::Solvable satSolvable() const
      { return resolvable() ? resolvable()->satSolvable() : sat::Solvable::noSolvable; }

      /** Return the buddy we share our status object with.
       * A \ref Product e.g. may share it's status with an associated reference \ref Package.
      */
      sat::Solvable buddy() const;

    public:
      /** Returns the ResObject::constPtr.
       * \see \ref operator->
       */
      ResObject::constPtr resolvable() const;

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
      using base::SafeBool<PoolItem>::operator bool_type;

    private:
      friend class Impl;
      friend class pool::PoolImpl;
      /** \ref PoolItem generator for \ref pool::PoolImpl. */
      static PoolItem makePoolItem( const sat::Solvable & solvable_r );
      /** Buddies are set by \ref pool::PoolImpl.*/
      void setBuddy( sat::Solvable solv_r );
      /** internal ctor */
      explicit PoolItem( Impl * implptr_r );
      /** Pointer to implementation */
      RW_pointer<Impl> _pimpl;

  private:
    friend SafeBool<PoolItem>::operator bool_type() const;
    bool boolTest() const
    { return !!resolvable(); }

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


  /** \relates PoolItem Test for same content. */
  inline bool identical( const PoolItem & lhs, const PoolItem & rhs )
  { return lhs == rhs || lhs.satSolvable().identical( rhs.satSolvable() ); }

  /** \relates PoolItem Test for same content. */
  inline bool identical( const PoolItem & lhs, sat::Solvable rhs )
  { return lhs.satSolvable().identical( rhs ); }

  /** \relates PoolItem Test for same content. */
  inline bool identical( sat::Solvable lhs, const PoolItem & rhs )
  { return lhs.identical( rhs.satSolvable() ); }


  /** \relates PoolItem Test for same name version release and arch. */
  inline bool sameNVRA( const PoolItem & lhs, const PoolItem & rhs )
  { return lhs == rhs || lhs.satSolvable().sameNVRA( rhs.satSolvable() ); }

  /** \relates PoolItem Test for same name version release and arch. */
  inline bool sameNVRA( const PoolItem & lhs, sat::Solvable rhs )
  { return lhs.satSolvable().sameNVRA( rhs ); }

  /** \relates PoolItem Test for same name version release and arch. */
  inline bool sameNVRA( sat::Solvable lhs, const PoolItem & rhs )
  { return lhs.sameNVRA( rhs.satSolvable() ); }

  /** Solvable to PoolItem transform functor.
   * \relates PoolItem
   * \relates sat::SolvIterMixin
   */
  struct asPoolItem
  {
    typedef PoolItem result_type;

    PoolItem operator()( const sat::Solvable & solv_r ) const
    { return PoolItem( solv_r ); }
  };

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
