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
#include <string>

#include "zypp/base/NonCopyable.h"
#include "zypp/NeedAType.h"

///////////////////////////////////////////////////////////////////
namespace zypp {
//////////////////////////////////////////////////////////////////

class VendorAttr : private base::NonCopyable
{
  public:
    /** Singleton */
    static const VendorAttr & instance();

    /** Enable autoprotection of foreign vendor packages.
     * This is the default.
     * \note This will \b not change the status of already
     * loaded pool items.
    */
    static void enableAutoProtect();
    /** Disable autoprotection of foreign vendor packages.
     * Autoprotection is on per defult.
     * \note This will \b not change the status of already
     * loaded pool items.
    */
    static void disableAutoProtect();

    /**
     * Return whether it's a known vendor
     **/
    bool isKnown( const Vendor & vendor_r ) const;

    /**
     * Return whether this vendors packages should be
     * protected by default.
     **/
    bool autoProtect( const Vendor & vendor_r ) const;

  private:
    VendorAttr();
};

///////////////////////////////////////////////////////////////////
}; // namespace zypp
///////////////////////////////////////////////////////////////////

#endif // ZYPP_VENDORATTR_H
