/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/CapAndItem.h
 *
*/
#ifndef ZYPP_CAPANDITEM_H
#define ZYPP_CAPANDITEM_H

#include "zypp/base/Deprecated.h"

#include "zypp/PoolItem.h"
#include "zypp/Capability.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : CapAndItem
    //
    /**
     *
     * \deprecated no longer supported
    */
    struct ZYPP_DEPRECATED CapAndItem
    {
	friend std::ostream & operator<<( std::ostream & str, const CapAndItem & obj );
	public:
	    Capability cap;
	    PoolItem item;

	    CapAndItem( Capability c, PoolItem i )
		: cap( c )
		, item( i )
	    { }
    };

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPANDITEM_H
