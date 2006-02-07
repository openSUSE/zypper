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

#include "zypp/ResPool.h"
#include "zypp/ResObject.h"
#include "zypp/ui/Selectable.h"

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
    /** Selectable implementation. */
    struct Selectable::Impl
    {
      friend std::ostream & operator<<( std::ostream & str, const Selectable::Impl & obj );

    public:

      /** This iterates PoolItems. Dont mix it with available_iterator,
       * which transforms the PoolItems to ResObject::constPtr.
      */
      typedef AvialableItemSet::const_iterator availableItem_iterator;

    public:
      Impl( const ResObject::Kind & kind_r,
            const std::string & name_r,
            const PoolItem & installedItem_r,
            availableItem_iterator availableBegin_r,
            availableItem_iterator availableEnd_r )
      : _kind( kind_r )
      , _name( name_r )
      , _installedItem( installedItem_r )
      , _availableItems( availableBegin_r, availableEnd_r )
      {}

    public:
      /**  */
      ResObject::Kind kind() const
      { return _kind; }
      /**  */
      const std::string & name() const
      { return _name; }
      /**  */
      Status status() const
      { return S_Taboo; }
      /**  */
      bool set_status( const Status state_r )
      { return false; }

      /** Installed object. */
      ResObject::constPtr installedObj() const
      { return _installedItem; }

      /** Best among available objects. */
      ResObject::constPtr candidateObj() const
      { return 0; }

      /** Best among all objects. */
      ResObject::constPtr theObj() const
      { return 0; }

      /** . */
      size_type availableObjs() const
      { return _availableItems.size(); }

      available_iterator availableBegin() const
      { return make_transform_iterator( _availableItems.begin(), ui_detail::TransformToResObjectPtr() ); }

      available_iterator availableEnd() const
      { return make_transform_iterator( _availableItems.end(), ui_detail::TransformToResObjectPtr() ); }

    private:
      ResObject::Kind  _kind;
      std::string      _name;
      PoolItem         _installedItem;
      AvialableItemSet _availableItems;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates Selectable::Impl Stream output */
    inline std::ostream & operator<<( std::ostream & str, const Selectable::Impl & obj )
    {
      return str << '[' << obj.kind() << ']' << obj.name() << ": " << obj.status()
                 << " (I " << obj._installedItem << ")"
                 << " (A " << obj._availableItems.size() << ")";
    }

    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_UI_SELECTABLEIMPL_H
