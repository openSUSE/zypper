#ifndef MA_PRINTING_H
#define MA_PRINTING_H

#include <iostream>

#include "zypp/base/LogControl.h"
#include "zypp/base/LogTools.h"
#include <zypp/base/Logger.h>

#include <zypp/base/String.h>
#include <zypp/base/Iterator.h>
#include <zypp/base/Algorithm.h>
#include <zypp/base/Functional.h>

///////////////////////////////////////////////////////////////////
#if 0
///////////////////////////////////////////////////////////////////
struct FormatStream
{
  explicit
  FormatStream( std::ostream & stream_r )
  : _stream( stream_r )
  {}

  FormatStream & operator<<( std::ostream & (*fnc)( std::ostream & ) )
  { _stream << fnc; return *this; }

  template<class _Tp>
    FormatStream & operator<<( const _Tp & obj )
    { _stream << "@(" << obj << ")@"; return *this; }

  std::ostream & _stream;
};
///////////////////////////////////////////////////////////////////
#undef XXX
#undef DBG
#undef MIL
#undef WAR
#undef ERR
#undef SEC
#undef INT
#undef USR

#define XXX FormatStream(_XXX( ZYPP_BASE_LOGGER_LOGGROUP ))
#define DBG FormatStream(_DBG( ZYPP_BASE_LOGGER_LOGGROUP ))
#define MIL FormatStream(_MIL( ZYPP_BASE_LOGGER_LOGGROUP ))
#define WAR FormatStream(_WAR( ZYPP_BASE_LOGGER_LOGGROUP ))
#define ERR FormatStream(_ERR( ZYPP_BASE_LOGGER_LOGGROUP ))
#define SEC FormatStream(_SEC( ZYPP_BASE_LOGGER_LOGGROUP ))
#define INT FormatStream(_INT( ZYPP_BASE_LOGGER_LOGGROUP ))
#define USR FormatStream(_USR( ZYPP_BASE_LOGGER_LOGGROUP ))
///////////////////////////////////////////////////////////////////
#endif
///////////////////////////////////////////////////////////////////


template<class _Tp>
  struct PrintOn : public std::unary_function<_Tp, bool>
  {
    bool operator()( const _Tp & obj ) const
    {
      if ( _leadNL )
        _str << std::endl << _prfx << obj;
      else
        _str << _prfx << obj << std::endl;
      return true;
    }

    PrintOn( std::ostream & str, const std::string & prfx = std::string(), bool leadNL = false )
    : _str( str )
    , _prfx( prfx )
    , _leadNL( leadNL )
    {}

    std::ostream & _str;
    std::string _prfx;
    bool _leadNL;
  };

///////////////////////////////////////////////////////////////////
#endif // MA_PRINTING_H
