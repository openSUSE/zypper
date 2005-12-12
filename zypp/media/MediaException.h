/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaException.h
 *
*/
#ifndef ZYPP_MEDIA_MEDIAEXCEPTION_H
#define ZYPP_MEDIA_MEDIAEXCEPTION_H

#include <iosfwd>

#include <string>

#include "zypp/base/Exception.h"
#include "zypp/Pathname.h"
#include "zypp/Url.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  namespace media {
    ///////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : MediaException
    /** Just inherits Exception to separate media exceptions
     *
     **/
    class MediaException : public Exception
    {
    public:
      /** Ctor taking message.
       * Use \ref ZYPP_THROW to throw exceptions.
      */
      MediaException()
      : Exception( "Media Exception" )
      {}
      /** Ctor taking message.
       * Use \ref ZYPP_THROW to throw exceptions.
      */
      MediaException( const std::string & msg_r )
      : Exception( msg_r )
      {}
      /** Dtor. */
      virtual ~MediaException() throw() {};
    };

    class MediaMountException : public MediaException
    {
    public:
      /** Ctor taking message.
       * Use \ref ZYPP_DOTHROW to throw exceptions.
      */
      MediaMountException( const std::string & error_r,
			   const std::string & source_r,
			   const std::string & target_r )
      : MediaException()
      , _error(error_r)
      , _source(source_r)
      , _target(target_r)
      {}
      /** Dtor. */
      virtual ~MediaMountException() throw() {};
    protected:
      virtual std::ostream & dumpOn( std::ostream & str ) const;
    private:
      std::string _error;
      std::string _source;
      std::string _target;
    };

    class MediaUnmountException : public MediaException
    {
    public:
      /** Ctor taking message.
       * Use \ref ZYPP_DOTHROW to throw exceptions.
      */
      MediaUnmountException( const std::string & error_r,
			     const std::string & path_r )
      : MediaException()
      , _error(error_r)
      , _path(path_r)
      {}
      /** Dtor. */
      virtual ~MediaUnmountException() throw() {};
    protected:
      virtual std::ostream & dumpOn( std::ostream & str ) const;
    private:
      std::string _error;
      std::string _path;
    };

    class MediaBadFilenameException : public MediaException
    {
    public:
      MediaBadFilenameException(const std::string & filename_r)
      : MediaException()
      , _filename(filename_r)
      {}
      virtual ~MediaBadFilenameException() throw() {};
      std::string filename() const { return _filename; }
    protected:
      virtual std::ostream & dumpOn( std::ostream & str ) const;
    private:
      std::string _filename;
    };

    class MediaNotOpenException : public MediaException
    {
    public:
      MediaNotOpenException()
      : MediaException()
      {}
      virtual ~MediaNotOpenException() throw() {};
    protected:
      virtual std::ostream & dumpOn( std::ostream & str ) const;
    private:
    };

    class MediaFileNotFoundException : public MediaException
    {
    public:
      MediaFileNotFoundException(const Url & url_r,
				 const Pathname & filename_r)
      : MediaException()
      , _url(url_r.toString())
      , _filename(filename_r.asString())
      {}
      virtual ~MediaFileNotFoundException() throw() {};
    protected:
      virtual std::ostream & dumpOn( std::ostream & str ) const;
    private:
      std::string _url;
      std::string _filename;
    };

    class MediaWriteException : public MediaException
    {
    public:
      MediaWriteException(const Pathname & filename_r)
      : MediaException()
      , _filename(filename_r.asString())
      {}
      virtual ~MediaWriteException() throw() {};
    protected:
      virtual std::ostream & dumpOn( std::ostream & str ) const;
    private:
      std::string _filename;
    };

    class MediaNotAttachedException : public MediaException
    {
    public:
      MediaNotAttachedException(const Url & url_r)
      : MediaException()
      , _url(url_r.toString())
      {}
      virtual ~MediaNotAttachedException() throw() {};
    protected:
      virtual std::ostream & dumpOn( std::ostream & str ) const;
    private:
      std::string _url;
    };

    class MediaSystemException : public MediaException
    {
    public:
      MediaSystemException(const Url & url_r,
			   const std::string & message_r)
      : MediaException()
      , _url(url_r.toString())
      , _message(message_r)
      {}
      virtual ~MediaSystemException() throw() {};
    protected:
      virtual std::ostream & dumpOn( std::ostream & str ) const;
    private:
      std::string _url;
      std::string _message;
    };

    class MediaNotAFileException : public MediaException
    {
    public:
      MediaNotAFileException(const Url & url_r,
			     const Pathname & path_r)
      : MediaException()
      , _url(url_r.toString())
      , _path(path_r.asString())
      {}
      virtual ~MediaNotAFileException() throw() {};
    protected:
      virtual std::ostream & dumpOn( std::ostream & str ) const;
    private:
      std::string _url;
      std::string _path;
    };

    class MediaNotADirException : public MediaException
    {
    public:
      MediaNotADirException(const Url & url_r,
			    const Pathname & path_r)
      : MediaException()
      , _url(url_r.toString())
      , _path(path_r.asString())
      {}
      virtual ~MediaNotADirException() throw() {};
    protected:
      virtual std::ostream & dumpOn( std::ostream & str ) const;
    private:
      std::string _url;
      std::string _path;
    };

    class MediaBadUrlException : public MediaException
    {
    public:
      MediaBadUrlException(const Url & url_r)
      : MediaException()
      , _url(url_r.toString())
      {}
      virtual ~MediaBadUrlException() throw() {};
    protected:
      virtual std::ostream & dumpOn( std::ostream & str ) const;
    private:
      std::string _url;
    };

    

  /////////////////////////////////////////////////////////////////
  } // namespace media
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_MEDIA_MEDIAEXCEPTION_H
