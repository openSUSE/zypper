/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ui/Selectable.h
 *
*/
#ifndef ZYPP_UI_SELECTABLE_H
#define ZYPP_UI_SELECTABLE_H

#include <iosfwd>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"
#include "zypp/base/Iterator.h"

#include "zypp/ui/SelectableTraits.h"
#include "zypp/ui/Status.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace ui
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Selectable
    //
    /** Collects PoolItems of same kind and name.
     *
     * Selectable is a status wrapper. That's why it offers the
     * PoolItems ResObjects but hides their individual ResStatus.
     * The ui::Status is calculated from (and transated to)
     * PoolItems individual ResStatus values.
     *
     * \note There's one Selectable per installed item, in case more
     * than one item is intalled.
     *
     * \todo Make it a _Ref.
    */
    class Selectable : public base::ReferenceCounted, private base::NonCopyable
    {
      friend std::ostream & operator<<( std::ostream & str, const Selectable & obj );

    public:
      typedef intrusive_ptr<Selectable>        Ptr;
      typedef intrusive_ptr<const Selectable>  constPtr;

      /** Iterates over ResObject::constPtr */
      typedef SelectableTraits::available_iterator      available_iterator;
      typedef SelectableTraits::availableItem_size_type size_type;

    public:
      /** The ResObjects kind. */
      ResObject::Kind kind() const;

      /** The ResObjects name.  */
      const std::string & name() const;

      /** Installed object. */
      ResObject::constPtr installedObj() const;

      /** Best among available objects. */
      ResObject::constPtr candidateObj() const;

      /** Best among all objects. */
      ResObject::constPtr theObj() const;

      /** Number of available objects. */
      size_type availableObjs() const;

      /** */
      available_iterator availableBegin() const;

      /** */
      available_iterator availableEnd() const;

    public:
      /** \name Query for objects within this Selectable.
      */
      //@{
      /** True if either installed or candidate object is present */
      bool hasObject() const
      { return installedObj() || candidateObj(); }

      /** True if installed object is present. */
      bool hasInstalledObj() const
      { return installedObj(); }

      /** True if candidate object is present. */
      bool hasCandidateObj() const
      { return candidateObj(); }

      /** True if installed and candidate object is present */
      bool hasBothObjects() const
      { return installedObj() && candidateObj(); }

      /** True if installed object is present but no candidate. */
      bool hasInstalledObjOnly() const
      { return installedObj() && ! candidateObj(); }

      /** True if candidate object is present but no installed. */
      bool hasCandidateObjOnly() const
      { return ! installedObj() && candidateObj(); }
      //@}

    public:
      /** \name Query objects fate in case of commit.
      */
      //@{
      enum Fate {
        TO_DELETE  = -1,
        UNMODIFIED = 0,
        TO_INSTALL = 1
      };
      /**  */
      Fate fate() const { return UNMODIFIED; } //TBI

      /** True if either to delete or to install */
      bool unmodified() const
      { return fate() == UNMODIFIED; }

      /** True if either to delete or to install */
      bool toModify() const
      { return fate() != UNMODIFIED; }

      /** True if to delete */
      bool toDelete() const
      { return fate() == TO_DELETE; }

      /** True if to install */
      bool toInstall() const
      { return fate() == TO_INSTALL; }

      /** Return who caused the modification. */
      ResStatus::TransactByValue modifiedBy() const { return ResStatus::USER; } //TBI

      /** True if modification was caused by by_r. */
      bool isModifiedBy( ResStatus::TransactByValue by_r ) const;

      /** True if the object won't be present on the targetSystem after commit. */
      bool isOffSystem() const { return false; } //TBI

      /** True if the object will be present on the targetSystem after commit. */
      bool isOnSystem() const { return false; } //TBI
      //@}

    public:
      /** \name Status manipulation.
       *
       * \note Every status manipulation will fail, if it
       * contradicts an action with higher ResStatus::TransactByValue.
      */
      //@{
      bool setFate( Fate fate_r, ResStatus::TransactByValue by_r )
      { return false; } //TBI

      /** */
      bool setUnmodified( ResStatus::TransactByValue by_r )
      { return setFate( UNMODIFIED, by_r ); }

      /** Request to delete an installed object.
       * Also fails if no installed object is present.
      */
      bool setToDelete( ResStatus::TransactByValue by_r )
      { return setFate( TO_DELETE, by_r ); }

      /** Request to install an available object.
       * Also fails if no available object is present.
      */
      bool setToInstall( ResStatus::TransactByValue by_r )
      { return setFate( TO_INSTALL, by_r ); }

       /** Request to take care the object is not present after commit.
        * Delete if installed, and do not install.
       */
      bool setOffSystem( ResStatus::TransactByValue by_r )
      { return false; } //TBI

      /** Request to take care the object is present after commit.
       * Install if not installed, and do not delete.
      */
      bool setOnSystem( ResStatus::TransactByValue by_r )
      { return false; } //TBI

      /** Request to take care the object is present after commit,
       * replacing the installed one, if a newer candidate is available.
       * Install if not installed, and do not delete.
      */
      bool setOnSystemNew( ResStatus::TransactByValue by_r )
      { return false; } //TBI

      //@}

    public:
      /** \name Special inteface for Y2UI.
       * \note This interface acts on \ref ResStatus::USER level.
       * The \ref Status enum, and allowed state transitions are
       * tightly related to the Y2UI. It might be not verry usefull
       * outside the Y2UI.
      */
      //@{
      /** Return the current Status */
      Status status() const;
      /** Try to set a new Status.
       * Returns \c false if the transitions is not allowed.
      */
      bool set_status( const Status state_r );
      //@}

    public:
      /** Implementation  */
      class Impl;
      typedef shared_ptr<Impl> Impl_Ptr;
      /** Default ctor */
      Selectable( Impl_Ptr pimpl_r );
    private:
      /** Dtor */
      ~Selectable();
    private:
      /** Pointer to implementation */
      RW_pointer<Impl> _pimpl;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Selectable Stream output */
    std::ostream & operator<<( std::ostream & str, const Selectable & obj );

    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_UI_SELECTABLE_H
