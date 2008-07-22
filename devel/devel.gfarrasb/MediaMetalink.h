/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaMetalink.h
 *
*/
#ifndef ZYPP_MEDIA_MEDIAMETALINK_H
#define ZYPP_MEDIA_MEDIAMETALINK_H

#include "zypp/media/MediaHandler.h"
#include "zypp/ZYppCallbacks.h"

#include <curl/curl.h>

extern "C" {   
   #include <metalink/metalink_parser.h>
 }

namespace zypp {
  namespace media {

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : MediaMetalink
/**
 * @short Implementation class for FTP, HTTP and HTTPS MediaHandler
 * @see MediaHandler
 **/
class MediaMetalink : public MediaHandler {

  protected:

    virtual void attachTo (bool next = false);
    virtual void releaseFrom( const std::string & ejectDev );
    virtual void getFile( const Pathname & filename ) const;
    virtual void getDir( const Pathname & dirname, bool recurse_r ) const;
    virtual void getDirInfo( std::list<std::string> & retlist,
                             const Pathname & dirname, bool dots = true ) const;
    virtual void getDirInfo( filesystem::DirContent & retlist,
                             const Pathname & dirname, bool dots = true ) const;
    /**
     * Repeatedly calls doGetDoesFileExist() until it successfully returns,
     * fails unexpectedly, or user cancels the operation. This is used to
     * handle authentication or similar retry scenarios on media level.
     */
    virtual bool getDoesFileExist( const Pathname & filename ) const;

    /**
     * \see MediaHandler::getDoesFileExist
     */
    virtual bool doGetDoesFileExist( const Pathname & filename ) const;

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


    bool doGetMetalinkFileCopy( const Pathname & filename , const Pathname & target, callback::SendReport<DownloadProgressReport> & report) const;
    bool getMetalinkFileCopy( const Pathname & filename , const Pathname & target) const;


    virtual bool checkAttachPoint(const Pathname &apoint) const;

  public:

    MediaMetalink( const Url &      url_r,
	       const Pathname & attach_point_hint_r );

    virtual ~MediaMetalink() { try { release(); } catch(...) {} }

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

    /** The user agent string */
    static const char *const agentString();

  private:
    /**
     * Return a comma separated list of available authentication methods
     * supported by server.
     */
    std::string getAuthHint() const;

  private:
    CURL *_curl;
    char _curlError[ CURL_ERROR_SIZE ];
    long _curlDebug;

    mutable std::string _userpwd;
    std::string _proxy;
    std::string _proxyuserpwd;
    std::string _currentCookieFile;
    std::string _ca_path;
    long        _xfer_timeout;

    static Pathname _cookieFile;

   
    public:
	mutable Url alternativeUrl[100];
	mutable int numberofmirrors;
	mutable int mirrortotry;
};

///////////////////////////////////////////////////////////////////

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_MEDIAMETALINK_H
