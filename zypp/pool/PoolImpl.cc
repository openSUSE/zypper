/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/pool/PoolImpl.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/pool/PoolImpl.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace pool
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : PoolImpl::PoolImpl
    //	METHOD TYPE : Ctor
    //
    PoolImpl::PoolImpl()
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : PoolImpl::~PoolImpl
    //	METHOD TYPE : Dtor
    //
    PoolImpl::~PoolImpl()
    {}

    /******************************************************************
    **
    **	FUNCTION NAME : operator<<
    **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const PoolImpl & obj )
    {
      return str << "PoolImpl " << obj.size();
    }

    void PoolImplInserter::operator()( ResObject::constPtr ptr_r )
    { _poolImpl.store().insert( PoolImpl::Item( ptr_r ) ); }

    void PoolImplDeleter::operator()( ResObject::constPtr ptr_r )
    { _poolImpl.store().erase( PoolImpl::Item( ptr_r ) ); }

    /////////////////////////////////////////////////////////////////
  } // namespace pool
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
