/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/InputStream.cc
 *
*/
#include <iostream>
#include "zypp/base/LogTools.h"

#include "zypp/base/InputStream.h"
#include "zypp/base/GzStream.h"

#include "zypp/PathInfo.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  namespace
  { /////////////////////////////////////////////////////////////////

    inline std::streamoff _heplerInitSize( const Pathname & file_r )
    {
      PathInfo p( file_r );
      if ( p.isFile() && filesystem::zipType( file_r ) == filesystem::ZT_NONE )
	return p.size();
      return -1;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : InputStream::InputStream
  //	METHOD TYPE : Constructor
  //
  InputStream::InputStream()
  : _stream( &std::cin, NullDeleter() )
  , _name( "STDIN" )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : InputStream::InputStream
  //	METHOD TYPE : Constructor
  //
  InputStream::InputStream( std::istream & stream_r,
                            const std::string & name_r )
  : _stream( &stream_r, NullDeleter() )
  , _name( name_r )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : InputStream::InputStream
  //	METHOD TYPE : Constructor
  //
  InputStream::InputStream( const Pathname & file_r )
  : _path( file_r )
  , _stream( new ifgzstream( _path.asString().c_str() ) )
  , _name( _path.asString() )
  , _size( _heplerInitSize( _path ) )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : InputStream::InputStream
  //	METHOD TYPE : Constructor
  //
  InputStream::InputStream( const Pathname & file_r,
                            const std::string & name_r )
  : _path( file_r )
  , _stream( new ifgzstream( _path.asString().c_str() ) )
  , _name( name_r )
  , _size( _heplerInitSize( _path ) )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : InputStream::InputStream
  //	METHOD TYPE : Constructor
  //
  InputStream::InputStream( const std::string & file_r )
  : _path( file_r )
  , _stream( new ifgzstream( _path.asString().c_str() ) )
  , _name( _path.asString() )
  , _size( _heplerInitSize( _path ) )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : InputStream::InputStream
  //	METHOD TYPE : Constructor
  //
  InputStream::InputStream( const std::string & file_r,
                            const std::string & name_r )
  : _path( file_r )
  , _stream( new ifgzstream( _path.asString().c_str() ) )
  , _name( name_r )
  , _size( _heplerInitSize( _path ) )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : InputStream::InputStream
  //	METHOD TYPE : Constructor
  //
  InputStream::InputStream( const char * file_r )
  : _path( file_r )
  , _stream( new ifgzstream( _path.asString().c_str() ) )
  , _name( _path.asString() )
  , _size( _heplerInitSize( _path ) )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : InputStream::InputStream
  //	METHOD TYPE : Constructor
  //
  InputStream::InputStream( const char * file_r,
                            const std::string & name_r )
  : _path( file_r )
  , _stream( new ifgzstream( _path.asString().c_str() ) )
  , _name( name_r )
  , _size( _heplerInitSize( _path ) )
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : InputStream::~InputStream
  //	METHOD TYPE : Destructor
  //
  InputStream::~InputStream()
  {}

  /******************************************************************
   **
   **	FUNCTION NAME : operator<<
   **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const InputStream & obj )
  {
    return str << obj.name() << obj.stream();
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
