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

#include "zypp/base/Flags.h"
#include "zypp/media/TransferSettings.h"
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
class MediaCurl : public MediaHandler
{
  public:
    enum RequestOption
    {
        /** Defaults */
        OPTION_NONE = 0x0,
        /** retrieve only a range of the file */
        OPTION_RANGE = 0x1,
        /** only issue a HEAD (or equivalent) request */
        OPTION_HEAD = 0x02,
        /** to not add a IFMODSINCE header if target exists */
        OPTION_NO_IFMODSINCE = 0x04,
        /** do not send a start ProgressReport */
        OPTION_NO_REPORT_START = 0x08,
    };
    ZYPP_DECLARE_FLAGS(RequestOptions,RequestOption);

  protected:

    Url clearQueryString(const Url &url) const;

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
    virtual void doGetFileCopy( const Pathname & srcFilename, const Pathname & targetFilename, callback::SendReport<DownloadProgressReport> & _report, RequestOptions options = OPTION_NONE ) const;


    virtual bool checkAttachPoint(const Pathname &apoint) const;

  public:

    MediaCurl( const Url &      url_r,
	       const Pathname & attach_point_hint_r );

    virtual ~MediaCurl() { try { release(); } catch(...) {} }

    TransferSettings & settings();

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
    static CURL *progressCallback_getcurl( void *clientp );
    /**
     * check the url is supported by the curl library
     * \throws MediaBadUrlException if there is a problem
     **/
    void checkProtocol(const Url &url) const;

    /**
     * initializes the curl easy handle with the data from the url
     * \throws MediaCurlSetOptException if there is a problem
     **/
    virtual void setupEasy();
    /**
     * concatenate the attach url and the filename to a complete
     * download url
     **/
    Url getFileUrl(const Pathname & filename) const;

    /**
     * Evaluates a curl return code and throws the right MediaException
     * \p filename Filename being downloaded
     * \p code Code curl returnes
     * \p timeout Whether we reached timeout, which we need to differentiate
     *    in case the codes aborted-by-callback or timeout are returned by curl
     *    Otherwise we can't differentiate abort from timeout. Here you may
     *    want to pass the progress data object timeout-reached value, or
     *    just true if you are not doing user interaction.
     *
     * \throws MediaException If there is a problem
     */
    void evaluateCurlCode( const zypp::Pathname &filename, CURLcode code, bool timeout ) const;

    void doGetFileCopyFile( const Pathname & srcFilename, const Pathname & dest, FILE *file, callback::SendReport<DownloadProgressReport> & _report, RequestOptions options = OPTION_NONE ) const;

  private:
    /**
     * Return a comma separated list of available authentication methods
     * supported by server.
     */
    std::string getAuthHint() const;

    bool authenticate(const std::string & availAuthTypes, bool firstTry) const;

    bool detectDirIndex() const;

  private:
    long _curlDebug;

    std::string _currentCookieFile;
    static Pathname _cookieFile;

  protected:
    CURL *_curl;
    char _curlError[ CURL_ERROR_SIZE ];
    curl_slist *_customHeaders;
    TransferSettings _settings;
};
ZYPP_DECLARE_OPERATORS_FOR_FLAGS(MediaCurl::RequestOptions);

///////////////////////////////////////////////////////////////////

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_MEDIACURL_H
