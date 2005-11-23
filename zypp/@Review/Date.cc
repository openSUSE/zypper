/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

  File:       Date.cc

  Author:     Michael Andres <ma@suse.de>
  Maintainer: Michael Andres <ma@suse.de>

  Purpose: Store and operate on date (time_t).

/-*/

// for strptime
// #define _XOPEN_SOURCE

#include <iostream>

#include <y2util/stringutil.h>
#include <y2util/Date.h>

using namespace std;

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : Date::form
//	METHOD TYPE : std::string
//
//	DESCRIPTION :
//
std::string Date::form( const std::string & format, time_t tval_r )
{
  static char buf[1024];
  if ( !strftime( buf, 1024, format.c_str(), localtime( &tval_r ) ) )
    return string();
  return buf;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : Date::fromSECONDS
//	METHOD TYPE : time_t
//
//	DESCRIPTION :
//
time_t Date::fromSECONDS( const std::string & str_r )
{
  return atol( str_r.c_str() );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : Date::toSECONDS
//	METHOD TYPE : std::string
//
//	DESCRIPTION :
//
std::string Date::toSECONDS( time_t tval_r )
{
  return form( "%s", tval_r );
}

#if 0
///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : Date::scan
//	METHOD TYPE : time_t
//
//	DESCRIPTION :
//
time_t Date::scan( const std::string & format, const std::string & str_r )
{
  struct tm tm;
  if ( strptime( str_r.c_str(), format.c_str(), &tm ) == NULL )
    return 0;
  DBG << "---------->" << asctime( &tm ) << endl;
  return mktime( &tm );
}
#endif

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : Date::asString
//	METHOD TYPE : std::string
//
//	DESCRIPTION :
//
std::string Date::asString() const
{
  return form( "%c", _date );
}

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : std::ostream &
**
**	DESCRIPTION :
*/
std::ostream & operator<<( std::ostream & str, const Date & obj )
{
  return str << obj.asString();
}


