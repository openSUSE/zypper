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
    // Forward to implementation.
    // Restrict PoolItems to ResObject::constPtr!
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

    ResObject::constPtr Selectable::setCandidate( ResObject::constPtr byUser_r )
    { return _pimpl->setCandidate( byUser_r ); }

    ResObject::constPtr Selectable::theObj() const
    { return _pimpl->theObj(); }

    Selectable::size_type Selectable::availableObjs() const
    { return _pimpl->availableObjs(); }

    Selectable::available_iterator Selectable::availableBegin() const
    { return make_transform_iterator( _pimpl->availableBegin(),
                                      SelectableTraits::TransformToResObjectPtr() ); }

    Selectable::available_iterator Selectable::availableEnd() const
    { return make_transform_iterator( _pimpl->availableEnd(),
                                      SelectableTraits::TransformToResObjectPtr() ); }

    ResStatus::TransactByValue Selectable::modifiedBy() const
    { return _pimpl->modifiedBy(); }

    bool Selectable::hasLicenceConfirmed() const
    { return _pimpl->hasLicenceConfirmed(); }

    void Selectable::setLicenceConfirmed( bool val_r )
    { _pimpl->setLicenceConfirmed( val_r ); }


    Selectable::Fate Selectable::fate() const
    {
      switch ( status() ) {
      case S_Update:
      case S_Install:
      case S_AutoUpdate:
      case S_AutoInstall:
        return TO_INSTALL;
        break;

      case S_Del:
      case S_AutoDel:
        return TO_DELETE;
        break;

      case S_Protected:
      case S_Taboo:
      case S_KeepInstalled:
      case S_NoInst:
        break;
      }
      return UNMODIFIED;
    };


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
