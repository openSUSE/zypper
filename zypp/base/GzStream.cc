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
#include "zypp/base/LogControl.h"
#include "zypp/base/LogTools.h"
using std::endl;

#include "zypp/base/GzStream.h"

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
    //	METHOD NAME : fgzstreambuf::open
    //	METHOD TYPE : fgzstreambuf *
    //
    fgzstreambuf *
    fgzstreambuf::open( const char * name_r, std::ios_base::openmode mode_r )
    {
      fgzstreambuf * ret = NULL;
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
              // Store mode and initialize the internal buffer.
              _mode = mode_r;
              if ( inReadMode() )
                {
                  setp( NULL, NULL );
                  setg( &(_buffer[0]), &(_buffer[0]), &(_buffer[0]) );
                }
              else
                {
                  setp( &(_buffer[0]), &(_buffer[_buffer.size()-1]) );
                  setg( NULL, NULL, NULL );
                }
              ret = this;
            }
          else
            setZError();
        }
      return ret;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : fgzstreambuf::close
    //	METHOD TYPE : fgzstreambuf *
    //
    fgzstreambuf *
    fgzstreambuf::close()
    {
      fgzstreambuf * ret = NULL;
      if ( isOpen() )
        {
          bool failed = false;
          if ( sync() != 0 )
            failed = true;
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
          setp( NULL, NULL );
          setg( NULL, NULL, NULL );
          if ( ! failed )
            ret = this;
        }
      return ret;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : fgzstreambuf::sync
    //	METHOD TYPE : int
    //
    int
    fgzstreambuf::sync()
    {
      int ret = 0;
      if ( pbase() < pptr() ) {
        const int_type res = overflow();
        if ( traits_type::eq_int_type( res, traits_type::eof() ) )
          ret = -1;
      }
      return ret;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : fgzstreambuf::overflow
    //	METHOD TYPE : fgzstreambuf::int_type
    //
    fgzstreambuf::int_type
    fgzstreambuf::overflow( int_type c )
    {
      int_type ret = traits_type::eof();
      if ( inWriteMode() )
        {
          if ( ! traits_type::eq_int_type( c, traits_type::eof() ) )
            {
              *pptr() = traits_type::to_char_type( c );
              pbump(1);
            }
          if ( pbase() <= pptr() )
            {
              if ( zWriteFrom( pbase(), pptr() - pbase() ) )
                {
                  setp( &(_buffer[0]), &(_buffer[_buffer.size()-1]) );
                  ret = traits_type::not_eof( c );
                }
              // else: error writing the file
            }
        }
      return ret;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : fgzstreambuf::underflow
    //	METHOD TYPE : fgzstreambuf::int_type
    //
    fgzstreambuf::int_type
    fgzstreambuf::underflow()
    {
      int_type ret = traits_type::eof();
      if ( inReadMode() )
        {
          if ( gptr() < egptr() )
            return traits_type::to_int_type( *gptr() );

          const std::streamsize got = zReadTo( &(_buffer[0]), _buffer.size() );
          if ( got > 0 )
            {
              setg( &(_buffer[0]), &(_buffer[0]), &(_buffer[got]) );
              ret = traits_type::to_int_type( *gptr() );
            }
          else if ( got == 0 )
            {
              // EOF
              setg( &(_buffer[0]), &(_buffer[0]), &(_buffer[0]) );
            }
          // else: error reading the file
        }
      return ret;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : fgzstreambuf::zReadTo
    //	METHOD TYPE : std::streamsize
    //
    std::streamsize
    fgzstreambuf::zReadTo( char * buffer_r, std::streamsize maxcount_r )
    {
      int read = gzread( _file, buffer_r, maxcount_r );
      if ( read < 0 )
        setZError();
      return read;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : fgzstreambuf::zWriteFrom
    //	METHOD TYPE : bool
    //
    bool
    fgzstreambuf::zWriteFrom( const char * buffer_r, std::streamsize count_r )
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
    //	METHOD NAME : fgzstreambuf::zSeekTo
    //	METHOD TYPE : fgzstreambuf::pos_type
    //
    fgzstreambuf::pos_type
    fgzstreambuf::zSeekTo( off_type off_r, std::ios_base::seekdir way_r )
    {
      z_off_t ret = gzseek( _file, off_r, way_r );
      if ( ret == -1 )
        setZError();
      return ret;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : fgzstreambuf::zTell
    //	METHOD TYPE : fgzstreambuf::pos_type
    //
    fgzstreambuf::pos_type
    fgzstreambuf::zTell()
    {
      z_off_t ret = gztell( _file );
      if ( ret == -1 )
        setZError();
      return ret;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : fgzstreambuf::seekTo
    //	METHOD TYPE : fgzstreambuf::pos_type
    //
    fgzstreambuf::pos_type
    fgzstreambuf::seekTo( off_type off_r, std::ios_base::seekdir way_r )
    {
      pos_type ret = pos_type(off_type(-1));
      if ( isOpen() )
        {
          if ( inWriteMode() )
            {
              if ( sync() == 0 )
                ret = zSeekTo( off_r, way_r );
            }
          else
            {
              off_type zegptr = zTell();
              if ( zegptr != off_type(-1) )
                {
                  if ( way_r == std::ios_base::end )
                    {
                      // Invalidate buffer and seek.
                      // XXX improve by transformation into ios_base::beg
                      // to see whether we stay inside the buffer.
                      setg( &(_buffer[0]), &(_buffer[0]), &(_buffer[0]) );
                      ret = zSeekTo( off_r, way_r );
                    }
                  else
                    {
                      // Transform into ios_base::beg and seek.
                      off_type zeback = zegptr - ( egptr() - eback() );
                      off_type zgptr  = zegptr - ( egptr() - gptr() );
                      off_type zngptr = off_r;
                      if ( way_r == std::ios_base::cur )
                        {
                          zngptr += zgptr;
                          way_r = std::ios_base::beg;
                        }

                      if ( way_r == std::ios_base::beg )
                        {
                          if ( zeback <= zngptr && zngptr <= zegptr )
                            {
                              // Still inside buffer, adjust gptr and
                              // calculate new position.
                              setg( eback(),
                                    eback() + (zngptr-zeback),
                                    egptr() );
                              ret = pos_type(zngptr);
                            }
                          else
                            {
                              // Invalidate buffer and seek.
                              setg( &(_buffer[0]), &(_buffer[0]), &(_buffer[0]) );
                              ret = zSeekTo( off_r, way_r );
                            }
                        }
                    }
                }
            }
        }
      return ret;
    }

    fgzstreambuf::pos_type
    fgzstreambuf::compressed_tell() const
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

