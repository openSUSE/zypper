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
#include "zypp/ui/SelectableImpl.h"
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

    ResObject::Kind Selectable::kind() const
    { return _pimpl->kind(); }

    const std::string & Selectable::name() const
    { return _pimpl->name(); }

    Status Selectable::status() const
    { return _pimpl->status(); }

    bool Selectable::set_status( const Status state_r )
    { return _pimpl->set_status( state_r ); }

    ResObject::constPtr Selectable::installedObj() const
    { return _pimpl->installedObj(); }

    ResObject::constPtr Selectable::candidateObj() const
    { return _pimpl->candidateObj(); }

    ResObject::constPtr Selectable::theObj() const
     { return _pimpl->theObj(); }

    Selectable::size_type Selectable::availableObjs() const
    { return _pimpl->availableObjs(); }

    Selectable::available_iterator Selectable::availableBegin() const
    { return _pimpl->availableBegin(); }

    Selectable::available_iterator Selectable::availableEnd() const
    { return _pimpl->availableEnd(); }

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
