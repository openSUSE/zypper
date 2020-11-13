#include "MediaHandlerFactory.h"


#include <zypp/base/Logger.h>

#include <zypp/media/MediaException.h>
#include <zypp/media/MediaHandler.h>

#include <zypp/media/MediaNFS.h>
#include <zypp/media/MediaCD.h>
#include <zypp/media/MediaDIR.h>
#include <zypp/media/MediaDISK.h>
#include <zypp/media/MediaCIFS.h>
#include <zypp/media/MediaCurl.h>
#include <zypp/media/MediaMultiCurl.h>
#include <zypp/media/MediaISO.h>
#include <zypp/media/MediaPlugin.h>
#include <zypp/media/UrlResolverPlugin.h>
#include <zypp/zyppng/media/MediaNetwork>

namespace zypp::media {

  MediaHandlerFactory::MediaHandlerFactory()
  {

  }

  std::unique_ptr<MediaHandler> MediaHandlerFactory::createHandler( const Url &o_url, const Pathname &preferred_attach_point )
  {
    if(!o_url.isValid()) {
      MIL << "Url is not valid" << std::endl;
      ZYPP_THROW(MediaBadUrlException(o_url));
    }

    std::unique_ptr<MediaHandler> _handler;

    UrlResolverPlugin::HeaderList custom_headers;
    Url url = UrlResolverPlugin::resolveUrl(o_url, custom_headers);

    std::string scheme = url.getScheme();
    MIL << "Trying scheme '" << scheme << "'" << std::endl;

    /*
    ** WARNING: Don't forget to update MediaAccess::downloads(url)
    **          if you are adding a new url scheme / handler!
    */
    if (scheme == "cd" || scheme == "dvd")
      _handler = std::make_unique<MediaCD> (url,preferred_attach_point);
    else if (scheme == "nfs" || scheme == "nfs4")
      _handler = std::make_unique<MediaNFS> (url,preferred_attach_point);
    else if (scheme == "iso")
      _handler = std::make_unique<MediaISO> (url,preferred_attach_point);
    else if (scheme == "file" || scheme == "dir")
      _handler = std::make_unique<MediaDIR> (url,preferred_attach_point);
    else if (scheme == "hd")
      _handler = std::make_unique<MediaDISK> (url,preferred_attach_point);
    else if (scheme == "cifs" || scheme == "smb")
      _handler = std::make_unique<MediaCIFS> (url,preferred_attach_point);
    else if (scheme == "ftp" || scheme == "tftp" || scheme == "http" || scheme == "https")
    {
      const char *networkenv = getenv( "ZYPPNG_MEDIANETWORK" );
      bool use_network = ( networkenv && strcmp(networkenv, "1" ) == 0 );
      if ( use_network ) {
        WAR << "network backend manually enabled." << std::endl;
        auto hdl = std::make_unique<zyppng::MediaNetwork> (url,preferred_attach_point);

        UrlResolverPlugin::HeaderList::const_iterator it;
        for ( const auto & el : custom_headers ) {
          std::string header { el.first };
          header += ": ";
          header += el.second;
          MIL << "Added custom header -> " << header << std::endl;
          hdl->settings().addHeader( std::move(header) );
        }
        _handler = std::move( hdl );

      } else {
        bool use_multicurl = true;
        std::string urlmediahandler ( url.getQueryParam("mediahandler") );
        if ( urlmediahandler == "multicurl" )
        {
          use_multicurl = true;
        }
        else if ( urlmediahandler == "curl" )
        {
          use_multicurl = false;
        }
        else
        {
          if ( ! urlmediahandler.empty() )
          {
            WAR << "unknown mediahandler set: " << urlmediahandler << std::endl;
          }
          const char *multicurlenv = getenv( "ZYPP_MULTICURL" );
          // if user disabled it manually
          if ( use_multicurl && multicurlenv && ( strcmp(multicurlenv, "0" ) == 0 ) )
          {
            WAR << "multicurl manually disabled." << std::endl;
            use_multicurl = false;
          }
          else if ( !use_multicurl && multicurlenv && ( strcmp(multicurlenv, "1" ) == 0 ) )
          {
            WAR << "multicurl manually enabled." << std::endl;
            use_multicurl = true;
          }
        }

        std::unique_ptr<MediaCurl> curl;

        if ( use_multicurl )
          curl = std::make_unique<MediaMultiCurl> (url,preferred_attach_point);
        else
          curl = std::make_unique<MediaCurl> (url,preferred_attach_point);

        for ( const auto & el : custom_headers ) {
          std::string header { el.first };
          header += ": ";
          header += el.second;
          MIL << "Added custom header -> " << header << std::endl;
          curl->settings().addHeader( std::move(header) );
        }
        _handler = std::move(curl);
      }
    }
    else if (scheme == "plugin" )
      _handler = std::make_unique<MediaPlugin> (url,preferred_attach_point);
    else
    {
      ZYPP_THROW(MediaUnsupportedUrlSchemeException(url));
    }

    // check created handler
    if ( !_handler ){
      ERR << "Failed to create media handler" << std::endl;
      ZYPP_THROW(MediaSystemException(url, "Failed to create media handler"));
    }

    MIL << "Opened: " << *_handler << std::endl;
    return _handler;
  }

}


