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
#include <fstream>
#include <set>
#include <map>

#include "zypp/base/LogTools.h"
#include "zypp/base/IOStream.h"
#include "zypp/base/String.h"

#include "zypp/PathInfo.h"
#include "zypp/VendorAttr.h"
#include "zypp/ZYppFactory.h"

#include "zypp/ZConfig.h"

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

    bool addTrustedVendor( const std::string & str_r )
    {
      std::string line( str::trim( str_r ) );
      if ( ! line.empty() && line[0] != '#')
        {
          _trustedVendors.insert( str::toLower( line ) );
        }
      return true;
    }

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
    const char * vendors[] = {
      "jpackage project",
      "novell",
      "opensuse",
      "sgi",
      "silicon graphics",
      "suse",
      "ati technologies inc.",
      "nvidia"
    };
    _trustedVendors.insert( vendors, vendors+(sizeof(vendors)/sizeof(const char *)) );

    Pathname vendorrcPath( "/etc/zypp/trustedVendors" );
    try
      {
        Target_Ptr trg( getZYpp()->target() );
        if ( trg )
          vendorrcPath = trg->root() / vendorrcPath;
      }
    catch ( ... )
      {
        // noop: Someone decided to let target() throw if the ptr is NULL ;(
      }

    PathInfo vendorrc( vendorrcPath );
    if ( vendorrc.isFile() )
      {
        MIL << "Reading " << vendorrc << endl;
        ifstream inp( vendorrc.asString().c_str() );
        iostr::forEachLine( inp, addTrustedVendor );
      }
    MIL << "Trusted Vendors: " << _trustedVendors << endl;
  }

  void VendorAttr::enableAutoProtect()
  {
    MIL << "FIXME: Not implemented." << endl;
    // FIXME use ZConfig
  }

  void VendorAttr::disableAutoProtect()
  {
    MIL << "FIXME: Not implemented." << endl;
    // FIXME use ZConfig
  }

  bool VendorAttr::isKnown( const Vendor & vendor_r ) const
  { return trusted( vendor_r ); }


  bool VendorAttr::autoProtect( const Vendor & vendor_r ) const
  { return( ZConfig::instance().autolock_untrustedvendor() && ! trusted( vendor_r ) ); }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

