/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#ifndef ZYPP_BASE_ZCKSTREAM_H
#define ZYPP_BASE_ZCKSTREAM_H

#include <iosfwd>
#include <streambuf>
#include <vector>
#include "zypp/base/SimpleStreambuf.h"
#include "zypp/base/fXstream.h"

typedef struct zckCtx zckCtx;

namespace zypp {

  namespace detail {

    /**
     * @short Streambuffer reading or writing zchunk files.
     *
     * Read and write mode are mutual exclusive. Seek is not supported.
     *
     * This streambuf is used in @ref ifzckstream and  @ref ofzckstream.
     **/
    class zckstreambufimpl {
      public:

        using error_type = std::string;

        ~zckstreambufimpl();

        bool isOpen   () const;
        bool canRead  () const;
        bool canWrite () const;
        bool canSeek  ( std::ios_base::seekdir way_r ) const;

        std::streamsize readData ( char * buffer_r, std::streamsize maxcount_r  );
        bool writeData( const char * buffer_r, std::streamsize count_r );
        off_t seekTo( off_t off_r, std::ios_base::seekdir way_r, std::ios_base::openmode omode_r );
        off_t tell() const;

        error_type error() const { return _lastErr; }

      protected:
        bool openImpl( const char * name_r, std::ios_base::openmode mode_r );
        bool closeImpl ();

      private:
        void setError ();
        int _fd = -1;
        bool _isReading = false;
        zckCtx *_zContext = nullptr;
        off_t _currfp = 0;
        error_type _lastErr;

    };
    using ZChunkStreamBuf = detail::SimpleStreamBuf<detail::zckstreambufimpl>;
  }

  /**
   * istream reading zchunk files.
   **/
  using ifzckstream = detail::fXstream<std::istream,detail::ZChunkStreamBuf>;

  /**
   * ostream writing zchunk files.
   **/
  using ofzckstream = detail::fXstream<std::ostream,detail::ZChunkStreamBuf>;
}

#endif
