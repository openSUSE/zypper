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
#include <zypp/media/TransferSettings.h>

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
      {}

    public:
      TransferSettings & settings()
      { return _settings; }

    protected:
      mutable TransferSettings _settings;
    };

  } // namespace media
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_MEDIA_MEDIANETWORKCOMMONHANDLER_H
