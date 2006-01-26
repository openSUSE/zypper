/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/zypp_detail/ZYppImpl.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/zypp_detail/ZYppImpl.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace zypp_detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ZYppImpl::ZYppImpl
    //	METHOD TYPE : Constructor
    //
    ZYppImpl::ZYppImpl()
    {
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ZYppImpl::~ZYppImpl
    //	METHOD TYPE : Destructor
    //
    ZYppImpl::~ZYppImpl()
    {
    }

    void ZYppImpl::addResolvables (const ResStore& store)
    {
	_pool.insert(store.begin(), store.end());
    }
    
    void ZYppImpl::removeResolvables (const ResStore& store)
    {
        for (ResStore::iterator it = store.begin(); it != store.end(); it++)
	{
    	    _pool.erase(*it);
	}
    }
    
    /******************************************************************
     **
     **	FUNCTION NAME : operator<<
     **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const ZYppImpl & obj )
    {
      return str << "ZYppImpl";
    }

    /////////////////////////////////////////////////////////////////
  } // namespace zypp_detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
