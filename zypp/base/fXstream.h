/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#ifndef ZYPP_BASE_FXSTREAM_H
#define ZYPP_BASE_FXSTREAM_H

#include <iosfwd>
#include <iostream>

namespace zypp {
  namespace detail {
    /**
     * @short Common template to define ifgzstream/ofgzstream
     * reading/writing compressed files.
     *
     * Don't use fXstream directly, but @ref ifgzstream or
     * @ref ofgzstream. fXstream is just to avoid almost
     * duplicate code.
     **/
    template<class TBStream,class TStreamBuf>
    class fXstream : public TBStream
    {
    public:

      using ZlibError = typename TStreamBuf::error_type;
      using stream_type = TBStream;
      using streambuf_type = TStreamBuf;

      fXstream()
        : stream_type( nullptr )
      { this->init( &_streambuf ); }

      explicit
        fXstream( const char * file_r )
        : stream_type( nullptr )
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
             * The last error returned retuned from zlib.
             **/
      ZlibError
      zError() const
      { return _streambuf.error(); }

      //! Similar to ios::rdbuf.
      //! But it returns our specific type, not the generic streambuf *.
      const streambuf_type&
      getbuf() const
      { return _streambuf; }

    private:

      streambuf_type _streambuf;

      std::ios_base::openmode
      defMode( const std::istream & )
      { return std::ios_base::in; }

      std::ios_base::openmode
      defMode( const std::ostream & )
      { return std::ios_base::out; }

    };
  }
}

#endif
