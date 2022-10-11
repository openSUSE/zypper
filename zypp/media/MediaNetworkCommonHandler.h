/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/media/MediaNetworkCommonHandler.h
 */
#ifndef ZYPP_MEDIA_MEDIANETWORKCOMMONHANDLER_H
#define ZYPP_MEDIA_MEDIANETWORKCOMMONHANDLER_H

#include <zypp/media/MediaHandler.h>
#include <zypp-curl/TransferSettings>

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace media
  {
    ///////////////////////////////////////////////////////////////////
    /// \class MediaNetworkCommonHandler
    /// \brief Common baseclass for MediaCurl and MediaNetwork
    ///
    /// Access to commonly used stuff like \ref TransferSettings mainly
    //  to avoid duplicated code.
    ///////////////////////////////////////////////////////////////////
    class MediaNetworkCommonHandler : public MediaHandler
    {
    public:
      MediaNetworkCommonHandler( const Url &      url_r,
                                 const Pathname & attach_point_r,
                                 const Pathname & urlpath_below_attachpoint_r,
                                 const bool       does_download_r )
      : MediaHandler( url_r, attach_point_r, urlpath_below_attachpoint_r, does_download_r )
      , _redirTarget( findGeoIPRedirect(url_r) )

      {}

    public:
      TransferSettings & settings()
      { return _settings; }

    protected:

      /**
       * concatenate the attach url and the filename to a complete
       * download url
       **/
      Url getFileUrl(const Pathname & filename) const;


      /**
       * Rewrites the baseURL to the geoIP target if one is found in the metadata cache,
       * otherwise simply returns the url again.
       */
      static zypp::Url findGeoIPRedirect ( const zypp::Url &url );

    protected:
      mutable TransferSettings _settings;
      Url _redirTarget;
    };

  } // namespace media
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_MEDIA_MEDIANETWORKCOMMONHANDLER_H
