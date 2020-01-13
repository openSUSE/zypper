/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/base/SimpleStreambuf.h
 *
*/
#ifndef ZYPP_BASE_SIMPLESTREAMBUF_H_DEFINED
#define ZYPP_BASE_SIMPLESTREAMBUF_H_DEFINED

#include <streambuf>
#include <vector>

namespace zypp {
  namespace detail {

    /*!
     * Implementation of a std::streambuf that is using a std::vector<char> as buffer,
     * relies on a Impl class that must implement the basic i/o functionality:
     *
     * \code
     *  class streambufimpl {
     *    public:
     *      using error_type = my_error_type;
     *
     *      ~streambufimpl();
     *
     *      bool isOpen   () const; // returns true if the file is currently open
     *      bool canRead  () const; // returns true if in read mode
     *      bool canWrite () const; // returns true if in write mode
     *      bool canSeek  ( std::ios_base::seekdir way_r ) const; // returns true if the backend can seek in the given way
     *
     *      std::streamsize readData ( char * buffer_r, std::streamsize maxcount_r  ); // reads data from the file and returns it in buffer_r
     *      bool writeData( const char * buffer_r, std::streamsize count_r ); // writes data ( if in write mode ) to file from buffer
     *      off_t seekTo( off_t off_r, std::ios_base::seekdir way_r, std::ios_base::openmode omode_r ); // seeks in file if supported
     *      off_t tell() const; // returns the current FP
     *
     *      error_type error() const; // returns the last error that happend in backend
     *
     *    protected:
     *      bool openImpl( const char * name_r, std::ios_base::openmode mode_r ); // backend implementation of opening the file
     *      bool closeImpl ();  // closes the file
     *  };
     *  using FullStreamBuf = detail::SimpleStreamBuf<streambufimpl>;
     * \endcode
     *
     * \note Currently only supports reading or writing at the same time, but can be extended to support both
     */
    template<typename Impl>
    class SimpleStreamBuf : public std::streambuf, public Impl
    {

      public:

      SimpleStreamBuf( size_t bufsize_r = 512) : _buffer( bufsize_r ) { }
      virtual ~SimpleStreamBuf() { close(); }

      SimpleStreamBuf * open( const char * name_r, std::ios_base::openmode mode_r = std::ios_base::in ) {

          if ( !this->openImpl( name_r, mode_r ) )
            return nullptr;

          if ( this->canRead() ) {
            setp( NULL, NULL );
            setg( &(_buffer[0]), &(_buffer[0]), &(_buffer[0]) );
          } else {
            setp( &(_buffer[0]), &(_buffer[_buffer.size()-1]) );
            setg( NULL, NULL, NULL );
          }

          return this;
        }

        SimpleStreamBuf * close() {

          if ( !this->isOpen() )
            return nullptr;

          if ( this->canWrite() )
            sync();

          if ( !this->closeImpl() )
            return nullptr;

          return this;
        }

      protected:

        virtual int sync() {
          int ret = 0;
          if ( pbase() < pptr() ) {
            const int_type res = overflow();
            if ( traits_type::eq_int_type( res, traits_type::eof() ) )
              ret = -1;
          }
          return ret;
        }

        virtual int_type overflow( int_type c = traits_type::eof() ) {
          int_type ret = traits_type::eof();
          if ( this->canWrite() ) {
            if ( ! traits_type::eq_int_type( c, traits_type::eof() ) )
            {
              *pptr() = traits_type::to_char_type( c );
              pbump(1);
            }
            if ( pbase() <= pptr() )
            {
              if ( this->writeData( pbase(), pptr() - pbase() ) )
              {
                setp( &(_buffer[0]), &(_buffer[_buffer.size()-1]) );
                ret = traits_type::not_eof( c );
              }
              // else: error writing the file
            }
          }
          return ret;
        }

        virtual int_type underflow() {
          int_type ret = traits_type::eof();
          if ( this->canRead() )
          {
            if ( gptr() < egptr() )
              return traits_type::to_int_type( *gptr() );

            const std::streamsize got = this->readData( &(_buffer[0]), _buffer.size() );
            if ( got > 0 )
            {
              setg( &(_buffer[0]), &(_buffer[0]), &(_buffer.data()[got]) );
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

        virtual pos_type seekpos( pos_type pos_r, std::ios_base::openmode openMode ) {
          return seekoff( off_type(pos_r), std::ios_base::beg, openMode );
        }


        virtual pos_type seekoff( off_type off_r, std::ios_base::seekdir way_r, std::ios_base::openmode openMode ) {
          pos_type ret = pos_type(off_type(-1));
          if ( !this->canSeek( way_r) )
            return ret;

          if ( this->isOpen() ) {
            if ( openMode == std::ios_base::out ) {
              //write the buffer out and invalidate it , no need to keep it around
              if ( !this->canWrite() || sync() != 0 )
                return ret;

              ret = this->seekTo( off_r, way_r, openMode );

            } else if ( openMode == std::ios_base::in ) {
              if ( !this->canRead() )
                return ret;

              //current physical FP, should point to end of buffer
              const off_type buffEndOff = this->tell();

              if ( buffEndOff != off_type(-1) ) {
                if ( way_r == std::ios_base::end ) {
                  setg( &(_buffer[0]), &(_buffer[0]), &(_buffer[0]) );
                  ret = this->seekTo( off_r, way_r, openMode );
                }

                const off_type bufLen    = egptr() - eback();
                const off_type bufStartFileOff  = buffEndOff - bufLen;
                const off_type currPtrFileOffset = buffEndOff - ( egptr() - gptr() );
                off_type newFOff = off_r;

                // Transform into ios_base::beg and seek.
                if ( way_r == std::ios_base::cur ) {
                  newFOff += currPtrFileOffset;
                  way_r = std::ios_base::beg;
                }

                //check if a seek would go out of the buffers boundaries
                if ( way_r == std::ios_base::beg ) {
                  if ( bufStartFileOff <= newFOff && newFOff <= buffEndOff ) {
                    // Still inside buffer, adjust gptr and
                    // calculate new position.
                    setg( eback(),
                      eback() + ( newFOff - bufStartFileOff ),
                      egptr() );
                    ret = pos_type( newFOff );
                  } else {
                    // Invalidate buffer and seek.
                    setg( &(_buffer[0]), &(_buffer[0]), &(_buffer[0]) );
                    ret = this->seekTo( off_r, way_r, openMode );
                  }
                }
              }
            }
          }
          return ret;
        }

      private:
        typedef std::vector<char> buffer_type;
        buffer_type              _buffer;
    };
  }
}
#endif
