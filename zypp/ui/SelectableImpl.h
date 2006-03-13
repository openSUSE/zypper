/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ui/SelectableImpl.h
 *
*/
#ifndef ZYPP_UI_SELECTABLEIMPL_H
#define ZYPP_UI_SELECTABLEIMPL_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"

#include "zypp/ui/Selectable.h"
#include "zypp/ui/SelectableTraits.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace ui
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Selectable::Impl
    //
    /** Selectable implementation.
     * \note Implementation is based in PoolItem, just the Selectable
     * inteface restricts them to ResObject::constPtr.
    */
    struct Selectable::Impl
    {
      friend std::ostream & operator<<( std::ostream & str, const Selectable::Impl & obj );

    public:

      typedef SelectableTraits::AvialableItemSet             AvialableItemSet;
      typedef SelectableTraits::availableItem_iterator       availableItem_iterator;
      typedef SelectableTraits::availableItem_const_iterator availableItem_const_iterator;
      typedef SelectableTraits::availableItem_size_type      availableItem_size_type;

    public:
      Impl( const ResObject::Kind & kind_r,
            const std::string & name_r,
            const PoolItem & installedItem_r,
            availableItem_const_iterator availableBegin_r,
            availableItem_const_iterator availableEnd_r )
      : _kind( kind_r )
      , _name( name_r )
      , _installedItem( installedItem_r )
      , _availableItems( availableBegin_r, availableEnd_r )
      {
        setCandidate( NULL );
      }

    public:
      /**  */
      ResObject::Kind kind() const
      { return _kind; }

      /**  */
      const std::string & name() const
      { return _name; }

      /**  */
      Status status() const;

      /**  */
      bool set_status( const Status state_r );

      /** Installed object. */
      PoolItem installedObj() const
      { return _installedItem; }

      /** Best among available objects.
       * The _candiate if set, or the first available.
       * \note Transacted Objects prefered, Status calculation relies on it.
      */
      PoolItem candidateObj() const
      {
        if ( _candidate )
          return _candidate;
        return( _availableItems.empty() ? PoolItem() : *_availableItems.begin() );
      }

      /** Set a userCandidate (out of available objects).
       * \return The new userCandidate or NULL if choice was invalid
       * (not among availableObjs).
       */
      PoolItem setCandidate( ResObject::constPtr byUser_r );

      /** Best among all objects. */
      PoolItem theObj() const
      {
        PoolItem ret( candidateObj() );
        return( ret ? ret : _installedItem );
      }

      /**  */
      availableItem_size_type availableObjs() const
      { return _availableItems.size(); }

      /**  */
      availableItem_const_iterator availableBegin() const
      { return _availableItems.begin(); }

      /**  */
      availableItem_const_iterator availableEnd() const
      { return _availableItems.end(); }

      /** Return who caused the modification. */
      ResStatus::TransactByValue modifiedBy() const;

      /** Return value of LicenceConfirmed bit. */
      bool hasLicenceConfirmed() const
      { return candidateObj() && candidateObj().status().isLicenceConfirmed(); }

      /** Set LicenceConfirmed bit. */
      void setLicenceConfirmed( bool val_r )
      { if ( candidateObj() ) candidateObj().status().setLicenceConfirmed( val_r ); }

     private:
      ResObject::Kind  _kind;
      std::string      _name;
      PoolItem         _installedItem;
      AvialableItemSet _availableItems;
      PoolItem         _candidate;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Selectable::Impl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const Selectable::Impl & obj )
    {
      return str << '[' << obj.kind() << ']' << obj.name() << ": " << obj.status()
                 << " (I " << obj._installedItem << ")"
                 << " (A " << obj._availableItems.size() << ")"
                 << obj._candidate;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_UI_SELECTABLEIMPL_H
