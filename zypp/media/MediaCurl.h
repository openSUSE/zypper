/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaCurl.h
 *
*/
#ifndef ZYPP_MEDIA_MEDIACURL_H
#define ZYPP_MEDIA_MEDIACURL_H

#include "zypp/media/MediaHandler.h"
#include "zypp/ZYppCallbacks.h"

#include <curl/curl.h>

namespace zypp {
  namespace media {

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : MediaCurl
/**
 * @short Implementation class for FTP, HTTP and HTTPS MediaHandler
 * @see MediaHandler
 **/
class MediaCurl : public MediaHandler {

  protected:

    MEDIA_HANDLER_API;
    /**
     *
     * \throws MediaException
     *
     */
    virtual void disconnectFrom();
    /**
     *
     * \throws MediaException
     *
     */
    virtual void getFileCopy( const Pathname & srcFilename, const Pathname & targetFilename) const;

    /**
     *
     * \throws MediaException
     *
     */
    virtual void doGetFileCopy( const Pathname & srcFilename, const Pathname & targetFilename, callback::SendReport<DownloadProgressReport> & _report) const;


    virtual bool checkAttachPoint(const Pathname &apoint) const;

  public:

    MediaCurl( const Url &      url_r,
	       const Pathname & attach_point_hint_r );

    virtual ~MediaCurl() { try { release(); } catch(...) {} }

    static void setCookieFile( const Pathname & );

    class Callbacks
    {
      public:
	virtual ~Callbacks() {}
        virtual bool progress( int percent ) = 0;
    };

  protected:

    static int progressCallback( void *clientp, double dltotal, double dlnow,
                                 double ultotal, double ulnow );

  private:
    CURL *_curl;
    char _curlError[ CURL_ERROR_SIZE ];

    std::string _userpwd;
    std::string _proxy;
    std::string _proxyuserpwd;
    std::string _currentCookieFile;
    std::string _ca_path;
    long        _xfer_timeout;

    static Pathname _cookieFile;
    static std::string _agent;
};

///////////////////////////////////////////////////////////////////

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_MEDIACURL_H
