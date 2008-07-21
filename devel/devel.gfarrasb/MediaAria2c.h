/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaAria2c.h
 *
*/
#ifndef ZYPP_MEDIA_MEDIAARIA2C_H
#define ZYPP_MEDIA_MEDIAARIA2C_H

#include "zypp/media/MediaHandler.h"
#include "zypp/ZYppCallbacks.h"

namespace zypp {
  namespace media {

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : MediaAria2c
/**
 * @short Implementation class for FTP, HTTP and HTTPS MediaHandler using an external program (aria2c) to retrive files
 * @author gfarrasb (gfarrasb@gmail.com)
 * @see MediaHandler
 **/
class MediaAria2c : public MediaHandler {

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
    //virtual bool doGetDoesFileExist( const Pathname & filename ) const;

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
    //virtual void doGetFileCopy( const Pathname & srcFilename, const Pathname & targetFilename, callback::SendReport<DownloadProgressReport> & _report) const;


    virtual bool checkAttachPoint(const Pathname &apoint) const;

  public:

    MediaAria2c( const Url &      url_r,
	       const Pathname & attach_point_hint_r );

    virtual ~MediaAria2c() { try { release(); } catch(...) {} }

    //static void setCookieFile( const Pathname & );

    /* External process to get aria2c version - TODO */
    std::string getAria2cVersion();

    class Callbacks
    {
      public:
	virtual ~Callbacks() {}
        virtual bool progress( int percent ) = 0;
    };

  protected:

    //static int progressCallback( void *clientp, double dltotal, double dlnow,
      //                           double ultotal, double ulnow );

    /** The user agent string */
    static const char *const agentString();	
    

  private:
    /**
     * Return a comma separated list of available authentication methods
     * supported by server.
     */
    //std::string getAuthHint() const;
    
    

  private:
    //CURL *_curl;
    //char _curlError[ CURL_ERROR_SIZE ];
    //long _curlDebug;

    mutable std::string _userpwd;
    std::string _proxy;
    std::string _proxyuserpwd;
    std::string _currentCookieFile;
    std::string _ca_path;
    long        _xfer_timeout;

    static Pathname _cookieFile;

    /** Aria2c path */
    static Pathname _aria2cPath;
};

///////////////////////////////////////////////////////////////////

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_MEDIAARIA2C_H
