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
    */
    class Selectable : public base::ReferenceCounted, private base::NonCopyable
    {
      friend std::ostream & operator<<( std::ostream & str, const Selectable & obj );

    public:
      typedef intrusive_ptr<Selectable>        Ptr;
      typedef intrusive_ptr<const Selectable>  constPtr;

      /** Iterates over ResObject::constPtr */
      typedef SelectableTraits::available_iterator      available_iterator;
      typedef SelectableTraits::availableItem_iterator  availablePoolItem_iterator;
      typedef SelectableTraits::availableItem_size_type size_type;

    public:
      /** The ResObjects kind. */
      ResObject::Kind kind() const;

      /** The ResObjects name.  */
      const std::string & name() const;

      /** Installed object. */
      ResObject::constPtr installedObj() const;

      /** PoolItem corresponding to the installed object. */
      PoolItem installedPoolItem() const;

      /** Best among available objects.
       + The user selected candiate, or a default.
      */
      ResObject::constPtr candidateObj() const;

      /** PoolItem corresponding to the candidate object. */
      PoolItem candidatePoolItem() const;

      /** Set a candidate (out of available objects).
       * \return The new candidate, or NULL if choice was invalid
       * (NULL or not among availableObjs). An invalid choice
       * selects the default candidate.
       */
      ResObject::constPtr setCandidate( ResObject::constPtr byUser_r );

      /** Best among all objects. */
      ResObject::constPtr theObj() const;

      /** Number of available objects. */
      size_type availableObjs() const;

      /** */
      available_iterator availableBegin() const;

      /** */
      available_iterator availableEnd() const;

      /** */
      availablePoolItem_iterator availablePoolItemBegin() const;

      /** */
      availablePoolItem_iterator availablePoolItemEnd() const;

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
      Fate fate() const;

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

      /** Return who caused the modification. */
      ResStatus::TransactByValue modifiedBy() const;

      /** Return value of LicenceConfirmed bit. */
      bool hasLicenceConfirmed() const;

      /** Set LicenceConfirmed bit. */
      void setLicenceConfirmed( bool val_r = true );
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
