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
#include "zypp/base/LogTools.h"

#include "zypp/pool/PoolImpl.h"
#include "zypp/pool/PoolStats.h"
#include "zypp/Package.h"
#include "zypp/VendorAttr.h"

#include "zypp/sat/Pool.h"
#include "zypp/sat/Repo.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  std::ostream & operator<<( std::ostream & str, const CapAndItem & obj )
  {
    return str << "{" << obj.cap << ", " << obj.item << "}";
  }

  ///////////////////////////////////////////////////////////////////
  namespace pool
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	Class PoolImpl::PoolImpl
    //
    ///////////////////////////////////////////////////////////////////

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
      return dumpPoolStats( str << "ResPool ",
                            obj.begin(), obj.end() );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace pool
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
