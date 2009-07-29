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
#include "zypp/media/MediaCurl.h"
#include "zypp/media/TransferSettings.h"
#include "zypp/ZYppCallbacks.h"

namespace zypp {
  namespace media {

/**
 * @short Implementation class for FTP, HTTP and HTTPS MediaHandler using an external program (aria2c) to retrive files
 *
 * @author gfarrasb (gfarrasb@gmail.com)
 * @author Duncan Mac-Vicar <dmacvicar@suse.de>
 *
 * @see MediaHandler
 **/
class MediaAria2c : public MediaCurl {

  public:
   /**
    * check if aria2c command line is present in the system
    **/
    static bool existsAria2cmd();

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

    virtual bool checkAttachPoint(const Pathname &apoint) const;

  public:

    MediaAria2c( const Url &      url_r,
	       const Pathname & attach_point_hint_r );

    virtual ~MediaAria2c() { try { release(); } catch(...) {} }

    //static void setCookieFile( const Pathname & );

    class Callbacks
    {
      public:
	virtual ~Callbacks() {}
        virtual bool progress( int percent ) = 0;
    };

  protected:

    static const char *const agentString();

  private:

    bool authenticate(const std::string & availAuthTypes, bool firstTry) const;

    std::string _currentCookieFile;
    std::string _ca_path;
    static Pathname _cookieFile;

    /** External process to get aria2c version */
    std::string getAria2cVersion();
    static std::string _aria2cVersion;
};

///////////////////////////////////////////////////////////////////

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_MEDIAARIA2C_H
