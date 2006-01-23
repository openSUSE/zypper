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
  , _inserter( _pimpl->store() )
  , _deleter( _pimpl->store() )
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

#warning IMPLEMENT IT
  void ResPoolManager::Inserter::operator()( ResObject::constPtr ptr_r )
  { INT << "+++ " << *ptr_r << endl; }

  void ResPoolManager::Deleter::operator()( ResObject::constPtr ptr_r )
  { SEC << "--- " << *ptr_r << endl; }

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
