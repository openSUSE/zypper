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
#include "zypp/PathInfo.h"

///////////////////////////////////////////////////////////////////
namespace zypp {
//////////////////////////////////////////////////////////////////

class VendorAttr : private base::NonCopyable
{
  public:
    /** Singleton */
    static const VendorAttr & instance();

    /**
     * Adding new equivalent vendors described in a directory
     **/
    bool addVendorDirectory( const Pathname & dirname ) const;

    /**
     * Adding new equivalent vendors described in a file
     **/
    bool addVendorFile( const Pathname & filename) const;    

    /** Return whether two vendor strings shold be treated as the same vendor.
     * Usually the solver is allowed to automatically select a package of an
     * equivalent vendor when updating. Replacing a package with one of a
     * different vendor usually must be confirmed by the user.
    */
    bool equivalent( const Vendor & lVendor, const Vendor & rVendor ) const;

  private:
    VendorAttr();
};

///////////////////////////////////////////////////////////////////
}; // namespace zypp
///////////////////////////////////////////////////////////////////

#endif // ZYPP_VENDORATTR_H
