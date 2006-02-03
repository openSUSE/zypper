/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ui/Selectable.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/ui/Selectable.h"
#include "zypp/ResPool.h"
#include "zypp/PoolItem.h"

using std::endl;

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
      typedef ResPool::Item                    PoolItem;
      typedef std::set<PoolItem>               AvialableItemSet;
      typedef AvialableItemSet::const_iterator available_iterator;
      typedef AvialableItemSet::size_type      size_type;

    public:
      Impl( const Object::Kind & kind_r,
            const std::string & name_r,
            const PoolItem & installedItem_r,
            available_iterator availableBegin_r,
            available_iterator availableEnd_r )
      : _kind( kind_r )
      , _name( name_r )
      , _installedItem( installedItem_r )
      , _availableItems( availableBegin_r, availableEnd_r )
      {}

    public:
      /**  */
      Object::Kind kind() const
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
      Object_Ptr installedObj() const
      { return _installedItem; }

      /** Best among available objects. */
      Object_Ptr candidateObj() const
      { return 0; }

      /** Best among all objects. */
      Object_Ptr theObj() const
      { return 0; }

      /** . */
      size_type availableObjs() const
      { return 0; }

    private:
    public:
      Object::Kind _kind;
      std::string _name;
      PoolItem _installedItem;
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

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : Selectable
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : Selectable::Selectable
    //	METHOD TYPE : Ctor
    //
    Selectable::Selectable( Impl_Ptr pimpl_r )
    : _pimpl( pimpl_r )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : Selectable::~Selectable
    //	METHOD TYPE : Dtor
    //
    Selectable::~Selectable()
    {}

    ///////////////////////////////////////////////////////////////////
    //
    // Forward to implementation
    //
    ///////////////////////////////////////////////////////////////////

    Selectable::Object::Kind Selectable::kind() const
    { return _pimpl->kind(); }

    const std::string & Selectable::name() const
    { return _pimpl->name(); }

    Status Selectable::status() const
    { return _pimpl->status(); }

    bool Selectable::set_status( const Status state_r )
    { return _pimpl->set_status( state_r ); }

    Selectable::Object_Ptr Selectable::installedObj() const
    { return _pimpl->installedObj(); }

    Selectable::Object_Ptr Selectable::candidateObj() const
    { return _pimpl->candidateObj(); }

    Selectable::Object_Ptr Selectable::theObj() const
     { return _pimpl->theObj(); }

    Selectable::size_type Selectable::availableObjs() const
    { return _pimpl->availableObjs(); }

    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const Selectable & obj )
    {
      return str << *obj._pimpl;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace ui
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
