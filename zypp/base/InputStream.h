/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/InputStream.h
 *
*/
#ifndef ZYPP_BASE_INPUTSTREAM_H
#define ZYPP_BASE_INPUTSTREAM_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/DefaultIntegral.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : InputStream
  //
  /** Helper to create and pass std::istream.
   * The provided std::istream may either be std::cin,
   * sone (gziped) file or an aleady existig \c std::istream.
   *
   * An optional \c name arument may be passed to the ctor,
   * to identify the stream in log messages, even if it is
   * not a file.
   *
   * Per default the name is "STDIN", the path to an input file
   * or empty.
   *
   * \code
   * void parse( const InputStream & input = InputStream() )
   * {
   *   // process input.stream() and refer to input.name()
   *   // in log messages.
   * }
   *
   * parse();                  // std::cin
   * parse( "/some/file" );    // file
   * parse( "/some/file.gz" ); // gziped file
   * std::istream & mystream;
   * parse( mystream );        // some existing stream
   * parse( InputStream( mystream,
   *                     "my stream's name" ) );
   * \endcode
  */
  class InputStream
  {
  public:
    /** Default ctor providing \c std::cin. */
    InputStream();

    /** Ctor providing an aleady existig \c std::istream. */
    InputStream( std::istream & stream_r,
                 const std::string & name_r = std::string() );

    /** Ctor for reading a (gziped) file. */
    InputStream( const Pathname & file_r );

    /** Ctor for reading a (gziped) file. */
    InputStream( const Pathname & file_r,
                 const std::string & name_r );

    /** Ctor for reading a (gziped) file. */
    InputStream( const std::string & file_r );

    /** Ctor for reading a (gziped) file. */
    InputStream( const std::string & file_r,
                 const std::string & name_r );

    /** Ctor for reading a (gziped) file. */
    InputStream( const char * file_r );

    /** Ctor for reading a (gziped) file. */
    InputStream( const char * file_r,
                 const std::string & name_r );

    /** Dtor. */
    ~InputStream();

    /** The std::istream.
     * \note The provided std::istream is never \c const.
    */
    std::istream & stream() const
    { return *_stream; }

    /** Allow implicit conversion to std::istream.*/
    operator std::istream &() const
    { return *_stream; }

    /** Name of the std::istream.
     * Per default this is "STDIN", the path to an input file or
     * empty. A custom string may be provided to the ctor.
     *
     * This may be used in log messages to identify the stream even
     * even if it is not a file.
    */
    const std::string & name() const
    { return _name; }

    /** Path to the input file or empty if no file. */
    const Pathname & path() const
    { return _path; }

    /** Size of the input stream (informal).
     * If constructed from an uncompressed file, the file size.
     * Otherwise \c -1. See \ref setSize;
    */
    std::streamoff size() const
    { return _size; }

    /** Set the size of the input stream.
     * You may set it to whatever vaule is appropriate. E.g.
     * <tt>*=10</tt> to compensate gzip comression. or the
     * number of items, lines, ... The value is not used here,
     * just provided.
    */
    void setSize( std::streamoff val_r )
    { _size = val_r; }

  private:
    Pathname                 _path;
    shared_ptr<std::istream> _stream;
    std::string              _name;
    DefaultIntegral<std::streamoff,-1> _size;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates InputStream Stream output */
  std::ostream & operator<<( std::ostream & str, const InputStream & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASE_INPUTSTREAM_H
