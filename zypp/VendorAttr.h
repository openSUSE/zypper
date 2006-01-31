/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/VendorAttr.h
 *
*/
#ifndef ZYPP_VENDORATTR_H
#define ZYPP_VENDORATTR_H

#include <iosfwd>
#include <map>
#include <list>
#include <string>

#include "zypp/NeedAType.h"

///////////////////////////////////////////////////////////////////
namespace zypp {
//////////////////////////////////////////////////////////////////

class VendorAttr {

    private:
	typedef std::map<Vendor,bool> TrustMap;

	TrustMap _trustMap;

	typedef std::list<std::string> VendorList;
	VendorList _trustedVendors;

    private:

	VendorAttr ();
	~VendorAttr ();

	bool trusted ( const Vendor & vendor_r );

    public:
	static VendorAttr *vendorAttr (void);

	/**
	 * Return whether it's a known vendor
	 **/
	bool isKnown( const Vendor & vendor_r );

	/**
	 * Return whether this vendors packages should be protected by
	 * default.
	 **/
	bool autoProtect( const Vendor & vendor_r );
};

///////////////////////////////////////////////////////////////////
}; // namespace zypp
///////////////////////////////////////////////////////////////////

#endif // ZYPP_VENDORATTR_H
