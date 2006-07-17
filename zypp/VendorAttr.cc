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
#include <set>
#include <map>

#include "zypp/base/LogTools.h"

#include "zypp/base/String.h"
#include "zypp/VendorAttr.h"

using namespace std;

#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::VendorAttr"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace
  { /////////////////////////////////////////////////////////////////

    typedef std::map<Vendor,bool> TrustMap;
    TrustMap _trustMap;

    typedef std::set<std::string> VendorList;
    VendorList _trustedVendors;

    bool trusted( const Vendor & vendor_r )
    {
      TrustMap::value_type val( vendor_r, false );
      pair<TrustMap::iterator, bool> res = _trustMap.insert( val );

      if ( res.second )
        {
          // check the new vendor in map
          for ( VendorList::const_iterator it = _trustedVendors.begin();
                it != _trustedVendors.end(); ++it )
            {
              if ( str::toLower( res.first->first.substr( 0, it->size() ) )
                   == str::toLower( *it ) )
                {
                  // match
                  res.first->second = true;
                  break;
                }
            }
        }
      return res.first->second;
    }


    /////////////////////////////////////////////////////////////////
  } // namespace
  ///////////////////////////////////////////////////////////////////

  const VendorAttr & VendorAttr::instance()
  {
    static VendorAttr _val;
    return _val;
  }

  VendorAttr::VendorAttr ()
  {
    char * vendors[] = {
      "jpackage project",
      "novell",
      "opensuse",
      "sgi",
      "silicon graphics",
      "suse",
      "ati technologies inc.",
      "nvidia"
   };
    _trustedVendors.insert( vendors, vendors+(sizeof(vendors)/sizeof(char *)) );
    MIL << "Trusted Vendors: " << _trustedVendors << endl;
  }

  bool VendorAttr::isKnown( const Vendor & vendor_r ) const
  { return trusted( vendor_r ); }


  bool VendorAttr::autoProtect( const Vendor & vendor_r ) const
  { return ! trusted( vendor_r ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

