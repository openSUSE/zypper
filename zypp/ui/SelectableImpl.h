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

      typedef SelectableTraits::AvailableItemSet         AvailableItemSet;
      typedef SelectableTraits::available_iterator       available_iterator;
      typedef SelectableTraits::available_const_iterator available_const_iterator;
      typedef SelectableTraits::available_size_type      available_size_type;

      typedef SelectableTraits::InstalledItemSet         InstalledItemSet;
      typedef SelectableTraits::installed_iterator       installed_iterator;
      typedef SelectableTraits::installed_const_iterator installed_const_iterator;
      typedef SelectableTraits::installed_size_type      installed_size_type;


    public:
      Impl( const ResObject::Kind & kind_r,
            const std::string & name_r,
            installed_const_iterator installedBegin_r,
            installed_const_iterator installedEnd_r ,
            available_const_iterator availableBegin_r,
            available_const_iterator availableEnd_r )
      : _kind( kind_r )
      , _name( name_r )
      , _installedItems( installedBegin_r, installedEnd_r )
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
      bool setStatus( const Status state_r );

      /** Installed object. */
      PoolItem installedObj() const
      {
          if (!installedEmpty())
              return *_installedItems.begin();
          return PoolItem();
      }

      /** Best among available objects.
       * The transacting candidate or the one scheduled to receive
       * the transact bit.
      */
      PoolItem candidateObj() const
      {
        PoolItem ret( transactingCandidate() );
        if ( ret )
          return ret;

        if ( _candidate )
          return _candidate;
        return defaultCandidate();
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
        if (ret)
            return ret;

        if ( ! _installedItems.empty() )
            return  (*_installedItems.begin());

        return PoolItem();
      }

      ////////////////////////////////////////////////////////////////////////

      bool availableEmpty() const
      { return _availableItems.empty(); }

      available_size_type availableSize() const
      { return _availableItems.size(); }

      available_const_iterator availableBegin() const
      { return _availableItems.begin(); }


      available_const_iterator availableEnd() const
      { return _availableItems.end(); }

      ////////////////////////////////////////////////////////////////////////

      bool installedEmpty() const
      { return _installedItems.empty(); }

      installed_size_type installedSize() const
      { return _installedItems.size(); }

      installed_iterator installedBegin() const
      { return _installedItems.begin(); }

      installed_iterator installedEnd() const
      { return _installedItems.end(); }

      ////////////////////////////////////////////////////////////////////////

      bool isUnmaintained() const
      { return availableEmpty(); }

      /** Return who caused the modification. */
      ResStatus::TransactByValue modifiedBy() const;

      /** Return value of LicenceConfirmed bit. */
      bool hasLicenceConfirmed() const
      { return candidateObj() && candidateObj().status().isLicenceConfirmed(); }

      /** Set LicenceConfirmed bit. */
      void setLicenceConfirmed( bool val_r )
      { if ( candidateObj() ) candidateObj().status().setLicenceConfirmed( val_r ); }

    private:
      PoolItem transactingCandidate() const
      {
        for ( available_const_iterator it = availableBegin();
              it != availableEnd(); ++it )
          {
            if ( (*it).status().transacts() )
              return (*it);
          }
        return PoolItem();
      }

      PoolItem defaultCandidate() const
      {
        if ( !installedEmpty() )
          {
            // prefer the installed objects arch.
            for ( installed_const_iterator iit = installedBegin();
                  iit != installedEnd(); ++iit )
            {
                for ( available_const_iterator it = availableBegin();
                      it != availableEnd(); ++it )
                {
                    if ( (*iit)->arch() == (*it)->arch() )
                    {
                        return (*it);
                    }
                }
            }
          }
        if ( _availableItems.empty() )
            return PoolItem();

        return *_availableItems.begin();
      }

      bool allCandidatesLocked() const
      {
        for ( available_const_iterator it = availableBegin();
              it != availableEnd(); ++it )
          {
            if ( ! (*it).status().isLocked() )
              return false;
          }
        return( ! _availableItems.empty() );
      }

    private:
      ResObject::Kind  _kind;
      std::string      _name;
      InstalledItemSet _installedItems;
      AvailableItemSet _availableItems;
      PoolItem         _candidate;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Selectable::Impl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const Selectable::Impl & obj )
    {
      return str << '[' << obj.kind() << ']' << obj.name() << ": " << obj.status()
                 << " (I " << obj._installedItems.size() << ")"
                 << " (A " << obj._availableItems.size() << ")"
                 << obj.candidateObj();
    }

    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_UI_SELECTABLEIMPL_H
