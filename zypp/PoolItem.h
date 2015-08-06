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

#include "zypp/sat/SolvableType.h"
#include "zypp/ResStatus.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  class ResPool;
  namespace pool
  {
    class PoolImpl;
  }
  ///////////////////////////////////////////////////////////////////
  /// \class PoolItem
  /// \brief Combining \ref sat::Solvable and \ref ResStatus.
  ///
  /// The "real" PoolItem is usually somewhere in the ResPool. This is
  /// a reference to it. All copies made will reference (and modify)
  /// the same PoolItem. All changes via a PoolItem are immediately
  /// visible in all copies (now COW).
  ///
  /// \note PoolItem is a SolvableType, which provides direct access to
  /// many of the underlying sat::Solvables properties.
  /// \see \ref sat::SolvableType
  ///
  /// \note Constness: Like pointer types, a <tt>const PoolItem</tt>
  /// does \b not refer to a <tt>const PoolItem</tt>. The reference is
  /// \c const, i.e. you can't change the refered PoolItem. The PoolItem
  /// (i.e. the status) is always mutable.
  ///////////////////////////////////////////////////////////////////
  class PoolItem : public sat::SolvableType<PoolItem>
  {
    friend std::ostream & operator<<( std::ostream & str, const PoolItem & obj );
    public:
      /** Default ctor for use in std::container. */
      PoolItem();

      /** Ctor looking up the \ref sat::Solvable in the \ref ResPool. */
      explicit PoolItem( const sat::Solvable & solvable_r );

      /** Ctor looking up the \ref sat::Solvable in the \ref ResPool. */
      template <class Derived>
      explicit PoolItem( const SolvableType<Derived> & solvable_r )
      : PoolItem( solvable_r.satSolvable() )
      {}

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

      /** This is a \ref sat::SolvableType. */
      explicit operator sat::Solvable() const
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

    private:
      friend class pool::PoolImpl;
      /** \ref PoolItem generator for \ref pool::PoolImpl. */
      static PoolItem makePoolItem( const sat::Solvable & solvable_r );
      /** Buddies are set by \ref pool::PoolImpl.*/
      void setBuddy( const sat::Solvable & solv_r );
      /** internal ctor */
    public:
      class Impl;	///< Expose type only
    private:
      explicit PoolItem( Impl * implptr_r );
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

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_POOLITEM_H
