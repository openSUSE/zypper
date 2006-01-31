/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/*
  File:       VendorAttr.cc

  Author:     Michael Andres <ma@suse.de>
  Maintainer: Michael Andres <ma@suse.de>

  Purpose: Manage vendor attributes

/-*/

#include <iostream>

#include "zypp/base/String.h"
#include "zypp/VendorAttr.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp {
//////////////////////////////////////////////////////////////////

bool
VendorAttr::trusted ( const Vendor & vendor_r )
{
    TrustMap::value_type val( vendor_r, false );
    pair<TrustMap::iterator, bool> res = _trustMap.insert( val );

    if ( res.second ) {
	// check the new vendor in map
	for ( VendorList::const_iterator it = _trustedVendors.begin();
					  it != _trustedVendors.end(); ++it )
	{
	    if ( str::toLower( res.first->first.substr( 0, it->size() ) )
		 == str::toLower( *it ) ) {
	  // match
	  res.first->second = true;
	  break;
	}
    }
  }

  return res.first->second;
}

VendorAttr::VendorAttr ()
{
    char *vendors[] = {
	"jpackage project",
	"novell",
	"sgi",
	"silicon graphics",
	"suse",
	NULL
    };

    char **vptr = vendors;
    while (*vptr) {
	_trustedVendors.push_back (string(*vptr));
	++vptr;
    }
}

VendorAttr::~VendorAttr ()
{}


VendorAttr *
VendorAttr::vendorAttr (void)
{
    static VendorAttr va;

    return &va;
}


/**
 * Return whether it's a known vendor
 **/
bool
VendorAttr::isKnown( const Vendor & vendor_r )
{
  return trusted( vendor_r );
}


/**
 * Return whether this vendors packages should be protected by
 * default.
 **/
bool
VendorAttr::autoProtect( const Vendor & vendor_r )
{
  return ! trusted( vendor_r );
}

///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
