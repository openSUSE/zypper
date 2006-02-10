/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ResPoolManager.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp/ResPoolManager.h"
#include "zypp/pool/PoolImpl.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResPoolManager::ResPoolManager
  //	METHOD TYPE : Ctor
  //
  ResPoolManager::ResPoolManager()
  : _pimpl( new pool::PoolImpl )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : ResPoolManager::~ResPoolManager
  //	METHOD TYPE : Dtor
  //
  ResPoolManager::~ResPoolManager()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  // Forward to impementation:
  //
  ///////////////////////////////////////////////////////////////////

  void ResPoolManager::clear()
  { _pimpl->clear(); }

  /** \todo FIXIT */
  ResPoolProxy ResPoolManager::proxy() const
  { return _pimpl->proxy( accessor() ); }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const ResPoolManager & obj )
  {
    return str << *obj._pimpl;
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
