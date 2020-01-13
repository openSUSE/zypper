/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#include "zypp/base/ZckStream.h"
#include "zypp/base/String.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

extern "C" {
#include <zck.h>
}

namespace zypp {

  namespace detail {

    zckstreambufimpl::~zckstreambufimpl()
    {
      closeImpl();
    }

    bool zckstreambufimpl::openImpl( const char *name_r, std::ios_base::openmode mode_r )
    {
      if ( isOpen() )
        return false;

      if ( mode_r == std::ios_base::in ) {
        _fd = ::open( name_r, O_RDONLY | O_CLOEXEC );
        _isReading = true;

      } else if ( mode_r == std::ios_base::out ) {
        _fd = ::open( name_r, O_WRONLY|O_CREAT|O_CLOEXEC, 0666 );
        _isReading = false;
      } else {
        //unsupported mode
        _lastErr = str::Format("ZChunk backend does not support the given open mode.");
        return false;
      }

      if ( _fd < 0 ) {
        const int errSrv = errno;
        _lastErr = str::Format("Opening file failed: %1%") % ::strerror( errSrv );
        return false;
      }

      _zContext = ::zck_create();
      if ( !_zContext ) {
        setError();
        return false;
      }

      if ( _isReading ) {
        if ( !::zck_init_read( _zContext, _fd ) ) {
          setError();
          return false;
        }
      } else {
        if ( !::zck_init_write( _zContext, _fd ) ) {
          setError();
          return false;
        }
      }

      _currfp = 0;
      return true;
    }

    bool zckstreambufimpl::closeImpl()
    {
      if ( !isOpen() )
        return true;

      bool success = true;

      if ( !zck_close( _zContext ) ) {
        setError();
        success = false;
      }

      zck_free( &_zContext );
      _zContext = nullptr;
      ::close( _fd );
      _fd = -1;
      return success;
    }

    void zckstreambufimpl::setError()
    {
      if ( !zck_is_error( _zContext ) )
        return;
      _lastErr = zck_get_error( _zContext );
      zck_clear_error( _zContext );
    }

    std::streamsize zckstreambufimpl::readData(char *buffer_r, std::streamsize maxcount_r)
    {
      if ( !isOpen() || !canRead() )
        return -1;

      ssize_t read = zck_read( _zContext, buffer_r, maxcount_r );
      if ( read > 0 )
        _currfp += read;
      else
        setError();

      return read;
    }

    bool zckstreambufimpl::writeData(const char *buffer_r, std::streamsize count_r)
    {
      if ( !isOpen() || !canWrite() )
        return false;

      ssize_t wrote = zck_write( _zContext, buffer_r, count_r );
      if ( wrote > 0 )
        _currfp += wrote;
      else
        setError();

      return wrote;
    }

    bool zckstreambufimpl::isOpen() const
    {
      return ( _fd >= 0 );
    }

    bool zckstreambufimpl::canRead() const
    {
      return _isReading;
    }

    bool zckstreambufimpl::canWrite() const
    {
      return !_isReading;
    }

    bool zckstreambufimpl::canSeek( std::ios_base::seekdir ) const
    {
      return false;
    }

    off_t zckstreambufimpl::seekTo(off_t, std::ios_base::seekdir , std::ios_base::openmode)
    {
      return -1;
    }

    off_t zckstreambufimpl::tell() const
    {
      return _currfp;
    }
  }

}
