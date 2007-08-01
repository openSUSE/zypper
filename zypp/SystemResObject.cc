/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SystemResObject.cc
 *
*/
//#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/SystemResObject.h"
#include "zypp/CapFactory.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE(SystemResObject);

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SystemResObject::SystemResObject
  //	METHOD TYPE : Ctor
  //
  SystemResObject::SystemResObject( const NVRAD & nvrad_r )
  : ResObject( TraitsType::kind, nvrad_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SystemResObject::~SystemResObject
  //	METHOD TYPE : Dtor
  //
  SystemResObject::~SystemResObject()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SystemResObject::instance()
  //	METHOD TYPE : SystemResObject::Ptr
  //
  SystemResObject::Ptr SystemResObject::instance()
  {
    static Ptr _ptr;
    if ( ! _ptr )
      {
        NVRAD dataCollect( "system" );
        dataCollect[Dep::PROVIDES].insert( CapFactory().halEvalCap() );
        dataCollect[Dep::PROVIDES].insert( CapFactory().modaliasEvalCap() );
        dataCollect[Dep::PROVIDES].insert( CapFactory().filesystemEvalCap() );

        detail::ResImplTraits<detail::SystemResObjectImplIf>::Ptr sysImpl;
        _ptr = detail::makeResolvableAndImpl( dataCollect, sysImpl );
      }

    return _ptr;
  }
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
