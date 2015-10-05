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
    //	CLASS NAME : fgzstreambuf
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
    class fgzstreambuf : public std::streambuf {

    public:

      fgzstreambuf( unsigned bufferSize_r = 512 )
      : _fd( -1 )
      ,_file( NULL )
      , _mode( std::ios_base::openmode(0) )
      , _buffer( (bufferSize_r?bufferSize_r:1), 0 )
      {}

      virtual
      ~fgzstreambuf()
      { close(); }

      bool
      isOpen() const
      { return _file; }

      bool
      inReadMode() const
      { return( _mode == std::ios_base::in ); }

      bool
      inWriteMode() const
      { return( _mode == std::ios_base::out ); }

      fgzstreambuf *
      open( const char * name_r, std::ios_base::openmode mode_r = std::ios_base::in );

        fgzstreambuf *
        close();

	//! Tell the file position in the compressed file.
	//! Analogous to tell(2), complementary to gztell.
	pos_type compressed_tell() const;

        /**
         * The last error returned fron zlib.
         **/
        ZlibError
        zError() const
        { return _error; }

    protected:

      virtual int
      sync();

      virtual int_type
      overflow( int_type c = traits_type::eof() );

      virtual int_type
      underflow();

      virtual pos_type
      seekoff( off_type off_r, std::ios_base::seekdir way_r, std::ios_base::openmode /* ignored */ )
      { return seekTo( off_r, way_r ); }

      virtual pos_type
      seekpos( pos_type pos_r, std::ios_base::openmode /* ignored */ )
      { return seekTo( off_type(pos_r), std::ios_base::beg ); }

    private:

      typedef std::vector<char> buffer_type;

      //! file descriptor of the compressed file
      int		       _fd;

      gzFile                   _file;

      std::ios_base::openmode  _mode;

      buffer_type              _buffer;

      ZlibError                _error;

    private:

      void
      setZError()
      { gzerror( _file, &_error._zError ); }

      std::streamsize
      zReadTo( char * buffer_r, std::streamsize maxcount_r );

      bool
      zWriteFrom( const char * buffer_r, std::streamsize count_r );

      pos_type
      zSeekTo( off_type off_r, std::ios_base::seekdir way_r );

      pos_type
      zTell();

      pos_type
      seekTo( off_type off_r, std::ios_base::seekdir way_r );
    };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : fXstream<class TBStr,class TSBuf>
    /**
     * @short Common template to define ifgzstream/ofgzstream
     * reading/writing gzip files.
     *
     * Don't use fXstream directly, but @ref ifgzstream or
     * @ref ofgzstream. fXstream is just to avoid almost
     * duplicate code.
     **/
    template<class TBStream,class TStreamBuf>
      class fXstream : public TBStream
      {
      public:

        typedef gzstream_detail::ZlibError ZlibError;
        typedef TBStream                   stream_type;
        typedef TStreamBuf                 streambuf_type;

        fXstream()
        : stream_type( NULL )
        { this->init( &_streambuf ); }

        explicit
        fXstream( const char * file_r )
        : stream_type( NULL )
        { this->init( &_streambuf ); this->open( file_r ); }

        virtual
        ~fXstream()
        {}

        bool
        is_open() const
        { return _streambuf.isOpen(); }

        void
        open( const char * file_r )
        {
          if ( !_streambuf.open( file_r, defMode(*this) ) )
            this->setstate(std::ios_base::failbit);
          else
            this->clear();
        }

        void
        close()
        {
          if ( !_streambuf.close() )
            this->setstate(std::ios_base::failbit);
        }

        /**
         * The last error returned retuned fron zlib.
         **/
        ZlibError
        zError() const
        { return _streambuf.zError(); }

	//! Similar to ios::rdbuf.
	//! But it returns our specific type, not the generic streambuf *.
	const streambuf_type&
        getbuf() const
        { return _streambuf; }

      private:

        streambuf_type _streambuf;

        std::ios_base::openmode
        defMode( const std::istream & str_r )
        { return std::ios_base::in; }

        std::ios_base::openmode
        defMode( const std::ostream & str_r )
        { return std::ios_base::out; }

      };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace gzstream_detail
  ///////////////////////////////////////////////////////////////////

  /**
   * istream reading gzip files as well as plain files.
   **/
  typedef gzstream_detail::fXstream<std::istream,gzstream_detail::fgzstreambuf> ifgzstream;

  /**
   * ostream writing gzip files.
   **/
  typedef gzstream_detail::fXstream<std::ostream,gzstream_detail::fgzstreambuf> ofgzstream;

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

#endif // ZYPP_BASE_GZSTREAM_H
