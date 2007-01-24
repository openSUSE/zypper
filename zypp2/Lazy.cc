/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Lazy.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"
#include <fstream>
#include "zypp/PathInfo.h"
#include "zypp/base/String.h"
#include "zypp/base/Exception.h"
#include "zypp2/Lazy.h"

using std::endl;
using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

    LazyLoadFromFileFunc::LazyLoadFromFileFunc( const Pathname &path, long int start, long int offset)
      : _path(path), _start(start), _offset(offset)
    {}
    
    string LazyLoadFromFileFunc::operator()()
    {
      char buffer[_offset + 1];
      std::ifstream is( _path.c_str() );
        
      if ( ! is )
      {
        ZYPP_THROW( Exception("Can't open " + _path.asString()) );
      }
        
      is.seekg(_start);
      is.read( buffer, _offset );
      buffer[_offset] = '\0';
      return std::string(buffer);
    }
     
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
