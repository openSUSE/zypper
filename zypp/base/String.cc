/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/base/String.cc
 *
*/
#include <cstdio>
#include <cstdarg>

#include <iostream>

#include "zypp/base/String.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace base
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace string
    { /////////////////////////////////////////////////////////////////

      /******************************************************************
       **
       **      FUNCTION NAME : form
       **      FUNCTION TYPE : std::string
      */
      std::string form( const char * format, ... )
      {
        struct SafeBuf
        {
          char * _buf;
          SafeBuf() : _buf( 0 ) {}
          ~SafeBuf() { if ( _buf ) free( _buf ); }
          std::string asString() const
          { return _buf ? std::string(_buf) : std::string(); }
        };
        SafeBuf safe;

        va_list ap;
        va_start( ap, format );
        vasprintf( &safe._buf, format, ap );
        va_end( ap );

        return safe.asString();
      }

      /******************************************************************
       **
       **      FUNCTION NAME : toLower
       **      FUNCTION TYPE : std::string
      */
      std::string toLower( const std::string & s )
      {
        if ( s.empty() )
          return s;

        std::string ret( s );
        for ( std::string::size_type i = 0; i < ret.length(); ++i ) {
          if ( isupper( ret[i] ) )
            ret[i] = static_cast<char>(tolower( ret[i] ));
        }
        return ret;
      }

      /******************************************************************
       **
       **      FUNCTION NAME : toUpper
       **      FUNCTION TYPE : std::string
      */
      std::string toUpper( const std::string & s )
      {
        if ( s.empty() )
          return s;

        std::string ret( s );
        for ( std::string::size_type i = 0; i < ret.length(); ++i ) {
          if ( islower( ret[i] ) )
            ret[i] = static_cast<char>(toupper( ret[i] ));
        }
        return ret;
      }


    /////////////////////////////////////////////////////////////////
    } // namespace string
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace base
  ///////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////
