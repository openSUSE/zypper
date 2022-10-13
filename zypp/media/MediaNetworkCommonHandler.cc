/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaNetworkCommonHandler.cc
 *
*/

#include "MediaNetworkCommonHandler.h"

#include <zypp/ZConfig.h>
#include <zypp-core/fs/PathInfo.h>
#include <zypp/base/Logger.h>
#include <zypp-core/base/Regex.h>

#include <fstream>

namespace zypp::media
{
  zypp::Url MediaNetworkCommonHandler::findGeoIPRedirect ( const zypp::Url &url )
  {
    try {
      const auto &conf = ZConfig::instance();
      if ( !conf.geoipEnabled() ) {
        MIL << "GeoIp rewrites disabled via ZConfig." << std::endl;
        return Url();
      }

      if ( !( url.getQueryParam("COUNTRY").empty() && url.getQueryParam("AVOID_COUNTRY").empty() )) {
        MIL << "GeoIp rewrites disabled since the baseurl " << url << " uses an explicit country setting." << std::endl;
        return Url();
      }

      const auto &hostname = url.getHost();
      auto geoipFile = conf.geoipCachePath() / hostname ;
      if ( PathInfo( geoipFile ).isFile() ) {

        MIL << "Found GeoIP file for host: " << hostname << std::endl;

        std::ifstream in( geoipFile.asString() );
        if (!in.is_open()) {
          MIL << "Failed to open GeoIP for host: " << hostname << std::endl;
          return Url();
        }

        try {
          std::string newHost;
          in >> newHost;

          Url newUrl = url;
          newUrl.setHost( newHost );

          MIL << "Found GeoIP rewrite: " << hostname << " -> " << newHost << std::endl;

          return newUrl;

        } catch ( const zypp::Exception &e ) {
          ZYPP_CAUGHT(e);
          MIL << "No valid GeoIP rewrite target found for " << url << std::endl;
        }
      }
    } catch ( const zypp::Exception &e ) {
      ZYPP_CAUGHT(e);
      MIL << "Failed to query GeoIP data, url rewriting disabled." << std::endl;
    }

    // no rewrite
    return Url();
  }

  Url MediaNetworkCommonHandler::getFileUrl( const Pathname & filename_r ) const
  {
    static const zypp::str::regex invalidRewrites("^.*\\/repomd.xml(.asc|.key)?$|^\\/geoip$");

    const bool canRedir = _redirTarget.isValid() && !invalidRewrites.matches(filename_r.asString());
    const auto &baseUrl = ( canRedir ) ? _redirTarget : _url;

    if ( canRedir )
      MIL << "Redirecting " << filename_r << " request to geoip location." << std::endl;

    // Simply extend the URLs pathname. An 'absolute' URL path
    // is achieved by encoding the leading '/' in an URL path:2
    //   URL: ftp://user@server		-> ~user
    //   URL: ftp://user@server/		-> ~user
    //   URL: ftp://user@server//		-> ~user
    //   URL: ftp://user@server/%2F	-> /
    //                         ^- this '/' is just a separator
    Url newurl( baseUrl );
    newurl.setPathName( ( Pathname("./"+baseUrl.getPathName()) / filename_r ).asString().substr(1) );
    return newurl;
  }
}
