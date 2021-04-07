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
      enum WhichHandler { choose, curl, multicurl, network };
      WhichHandler which = choose;
      // Leagcy: choose handler in UUrl query
      if ( const std::string & queryparam = url.getQueryParam("mediahandler"); ! queryparam.empty() ) {
	if ( queryparam == "network" )
	  which = network;
	else if ( queryparam == "multicurl" )
	  which = multicurl;
	else if ( queryparam == "curl" )
	  which = curl;
	else
	  WAR << "Unknown mediahandler='" << queryparam << "' in URL; Choosing the default" << std::endl;
      }
      // Otherwise choose handler through ENV
      if ( which == choose ) {
	auto getenvIs = []( std::string_view var, std::string_view val )->bool {
	  const char * v = ::getenv( var.data() );
	  return v && v == val;
	};

	if ( getenvIs( "ZYPP_MEDIANETWORK", "1" ) ) {
	  WAR << "network backend manually enabled." << std::endl;
	  which = network;
	}
	else if ( getenvIs( "ZYPP_MULTICURL", "0" ) ) {
	  WAR << "multicurl manually disabled." << std::endl;
	  which = curl;
	}
	else
	  which = multicurl;
      }
      // Finally use the default
      std::unique_ptr<MediaNetworkCommonHandler> handler;
      switch ( which ) {
	case network:
	  handler = std::make_unique<zyppng::MediaNetwork>( url, preferred_attach_point );
	  break;

	default:
	case multicurl:
	  handler = std::make_unique<MediaMultiCurl>( url, preferred_attach_point );
	  break;

	case curl:
	  handler = std::make_unique<MediaCurl>( url, preferred_attach_point );
	  break;
      }
      // Set up the handler
      for ( const auto & el : custom_headers ) {
	std::string header { el.first };
	header += ": ";
	header += el.second;
	MIL << "Added custom header -> " << header << std::endl;
	handler->settings().addHeader( std::move(header) );
      }
      _handler = std::move(handler);

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


