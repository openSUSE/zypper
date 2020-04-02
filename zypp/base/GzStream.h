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

  File:       GzStream.h

  Author:     Michael Andres <ma@suse.de>
  Maintainer: Michael Andres <ma@suse.de>

  Purpose: Streams reading and writing gzip files.

/-*/
#ifndef ZYPP_BASE_GZSTREAM_H
#define ZYPP_BASE_GZSTREAM_H

#include <iosfwd>
#include <streambuf>
#include <vector>
#include <zlib.h>

#include <zypp/base/SimpleStreambuf.h>
#include <zypp/base/fXstream.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace gzstream_detail
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ZlibError
    /**
     * @short Helper class to ship zlib errors.
     **/
    struct ZlibError
    {
      /**
       * The zlib error code
       **/
      int _zError;

      /**
       * errno, valid if zError is Z_ERRNO
       **/
      int _errno;

      ZlibError()
      : _zError( 0 ), _errno( 0 )
      {}

      /**
       * Return string describing the zlib error code
       **/
      std::string
      strerror() const;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates ZlibError Stream output. */
    inline std::ostream & operator<<( std::ostream & str, const ZlibError & obj )
    { return str << obj.strerror(); }


    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : gzstreambufimpl
    /**
     * @short Streambuffer reading or writing gzip files.
     *
     * Read and write mode are mutual exclusive. Seek is supported,
     * but zlib restrictions appy (only forward seek in write mode;
     * backward seek in read mode might be expensive).Putback is not
     * supported.
     *
     * Reading plain (no gziped) files is possible as well.
     *
     * This streambuf is used in @ref ifgzstream and  @ref ofgzstream.
     **/
    class gzstreambufimpl {
    public:

      using error_type = ZlibError;

      ~gzstreambufimpl()
      { closeImpl(); }

      bool
      isOpen   () const
      { return _file; }

      bool
      canRead  () const
      { return( _mode == std::ios_base::in ); }

      bool
      canWrite () const
      { return( _mode == std::ios_base::out ); }

      bool
      canSeek  ( std::ios_base::seekdir way_r ) const
      { return ( way_r == std::ios_base::beg || way_r == std::ios_base::cur ); }

    protected:
      bool openImpl( const char * name_r, std::ios_base::openmode mode_r );
      bool closeImpl ();

      //! Tell the file position in the compressed file.
      //! Analogous to tell(2), complementary to gztell.
      off_t compressed_tell() const;

    public:
      /**
         * The last error returned fron zlib.
         **/
      error_type
      error() const
      { return _error; }

      std::streamsize readData ( char * buffer_r, std::streamsize maxcount_r  );
      bool writeData( const char * buffer_r, std::streamsize count_r );
      off_t seekTo( off_t off_r, std::ios_base::seekdir way_r, std::ios_base::openmode omode_r );
      off_t tell() const;

    private:

      void
      setZError() const
      { gzerror( _file, &_error._zError ); }

      //! file descriptor of the compressed file
      int		         _fd = -1;

      gzFile                   _file = nullptr;

      std::ios_base::openmode  _mode = std::ios_base::openmode(0);

      mutable ZlibError        _error;

    };
    using fgzstreambuf = detail::SimpleStreamBuf<gzstreambufimpl>;
  } // namespace gzstream_detail

  /**
   * istream reading gzip files as well as plain files.
   **/
  typedef detail::fXstream<std::istream,gzstream_detail::fgzstreambuf> ifgzstream;

  /**
   * ostream writing gzip files.
   **/
  typedef detail::fXstream<std::ostream,gzstream_detail::fgzstreambuf> ofgzstream;

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#endif // ZYPP_BASE_GZSTREAM_H
