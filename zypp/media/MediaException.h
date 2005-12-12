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
      MediaMountException( const std::string & msg_r,
			   const std::string & source_r,
			   const std::string & target_r )
      : MediaException( msg_r )
      , _source(source_r)
      , _target(target_r)
      {}
      /** Dtor. */
      virtual ~MediaMountException() throw() {};
      std::string source() const { return _source; }
      std::string target() const { return _target; }
    private:
      std::string _source;
      std::string _target;
    };

    class MediaUnmountException : public MediaException
    {
    public:
      /** Ctor taking message.
       * Use \ref ZYPP_DOTHROW to throw exceptions.
      */
      MediaUnmountException( const std::string & msg_r,
			     const std::string & path_r )
      : MediaException( msg_r )
      , _path(path_r)
      {}
      /** Dtor. */
      virtual ~MediaUnmountException() throw() {};
      std::string path() const { return _path; }
    private:
      std::string _path;
    };

  /////////////////////////////////////////////////////////////////
  } // namespace media
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_MEDIA_MEDIAEXCEPTION_H
