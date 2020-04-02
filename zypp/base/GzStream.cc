/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                         (C) SuSE Linux Products GmbH |
\----------------------------------------------------------------------/

  File:       GzStream.cc

  Author:     Michael Andres <ma@suse.de>
  Maintainer: Michael Andres <ma@suse.de>

  Purpose: Streams reading and writing gzip files.

/-*/

#include <cerrno>
#include <iostream>
#include <zypp/base/LogControl.h>
#include <zypp/base/LogTools.h>
using std::endl;

#include <zypp/base/GzStream.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace gzstream_detail
  { /////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ZlibError
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : ZlibError::strerror
    //	METHOD TYPE : std::string
    //
    std::string
    ZlibError::strerror() const
    {
      std::string ret = ( _zError ? ::zError( _zError ) : "OK" );
      if ( _zError == Z_ERRNO )
        ret += std::string("(") + ::strerror( _errno ) + ")";
      return ret;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : fgzstreambuf
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : gzstreambufimpl::openImpl
    //	METHOD TYPE : bool
    //
    bool
    gzstreambufimpl::openImpl(const char *name_r, std::ios_base::openmode mode_r)
    {
      bool ret = false;
      if ( ! isOpen() )
      {
        // we expect gzdopen to handle errors of ::open
        if ( mode_r == std::ios_base::in )
        {
          _fd = ::open( name_r, O_RDONLY | O_CLOEXEC );
          _file = gzdopen( _fd, "rb" );
        }
        else if ( mode_r == std::ios_base::out )
        {
          _fd = ::open( name_r, O_WRONLY|O_CREAT|O_CLOEXEC, 0666 );
          _file = gzdopen( _fd, "wb" );
        }
        // else: not supported

        if ( isOpen() )
        {
          // Store mode
          _mode = mode_r;
          ret   = true;
        }
        else
          setZError();
      }
      return ret;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : gzstreambufimpl::closeImpl
    //	METHOD TYPE : bool
    //
    bool
    gzstreambufimpl::closeImpl()
    {
      bool ret = false;
      if ( isOpen() )
        {
          bool failed = false;

	  // it also closes _fd, fine
          int r = gzclose( _file );
          if ( r != Z_OK )
            {
              failed = true;
              // DONT call setZError() here, as _file is no longer valid
              _error._zError = r;
              _error._errno = errno;
            }

          // Reset everything
	  _fd = -1;
          _file = NULL;
          _mode = std::ios_base::openmode(0);
          if ( ! failed )
            ret = true;
        }
      return ret;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : gzstreambufimpl::readData
    //	METHOD TYPE : std::streamsize
    //
    std::streamsize
    gzstreambufimpl::readData( char * buffer_r, std::streamsize maxcount_r )
    {
      int read = gzread( _file, buffer_r, maxcount_r );
      if ( read < 0 )
        setZError();
      return read;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : gzstreambufimpl::writeData
    //	METHOD TYPE : bool
    //
    bool
    gzstreambufimpl::writeData( const char * buffer_r, std::streamsize count_r )
    {
      int written = 0;
      if ( count_r )
        {
          if ( (written = gzwrite( _file, buffer_r, count_r )) == 0 )
            setZError();
        }
      return( written == count_r );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : gzstreambufimpl::seekTo
    //	METHOD TYPE : off_t
    //
    off_t
    gzstreambufimpl::seekTo( off_t off_r, std::ios_base::seekdir way_r, std::ios_base::openmode )
    {
      z_off_t ret = gzseek( _file, off_r, way_r );
      if ( ret == -1 )
        setZError();
      return ret;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : gzstreambufimpl::tell
    //	METHOD TYPE : off_t
    //
    off_t
    gzstreambufimpl::tell() const
    {
      z_off_t ret = gztell( _file );
      if ( ret == -1 )
        setZError();
      return ret;
    }

    off_t
    gzstreambufimpl::compressed_tell() const
    {
	off_t pos = lseek (_fd, 0, SEEK_CUR);
	// hopefully the conversion is ok
        return pos;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace gzstream_detail
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

