/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Date.cc
 *
*/
#include <iostream>
//#include "zypp/base/Logger.h"

#include "zypp/base/String.h"

#include "zypp/Date.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  static std::string adjustLocale();
  static void restoreLocale(const std::string & locale);

  namespace
  {
    inline bool isDST( struct tm & tm )
    {
      time_t t = ::mktime( &tm );
      struct tm *tm2 = ::localtime( &t );
      return ( tm2 && tm2->tm_isdst > 0 );
    }
  }

  const Date::ValueType Date::second;
  const Date::ValueType Date::minute;
  const Date::ValueType Date::hour;
  const Date::ValueType Date::day;
  const Date::ValueType Date::month28;
  const Date::ValueType Date::month29;
  const Date::ValueType Date::month30;
  const Date::ValueType Date::month31;
  const Date::ValueType Date::month;
  const Date::ValueType Date::year365;
  const Date::ValueType Date::year366;
  const Date::ValueType Date::year;

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Date::Date
  //	METHOD TYPE : Constructor
  //
  Date::Date( const std::string & seconds_r )
  { str::strtonum( seconds_r, _date ); }

  Date::Date( const std::string & date_str, const std::string & format )
    : _date( Date( date_str, format, TB_LOCALTIME ) )
  {}

  Date::Date( const std::string & date_str, const std::string & format, Date::TimeBase base_r )
    : _date(0)
  {
    struct tm tm = {0,0,0,0,0,0,0,0,0,0,0};
    std::string thisLocale = adjustLocale();

    char * res = ::strptime( date_str.c_str(), format.c_str(), &tm );
    if ( isDST(tm) )
      tm.tm_isdst = 1;
    if ( res != NULL )
      _date = (base_r == TB_UTC ? ::timegm : ::timelocal)( &tm );

    restoreLocale(thisLocale);

    if (res == NULL)
      throw DateFormatException(
          str::form( "Invalid date format: '%s'", date_str.c_str() ) );
  }


  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : Date::form
  //	METHOD TYPE : std::string
  //
  std::string Date::form( const std::string & format_r, Date::TimeBase base_r ) const
  {
    static char buf[1024];
    std::string thisLocale = adjustLocale();

    if ( ! strftime( buf, 1024, format_r.c_str(), (base_r == TB_UTC ? gmtime : localtime)( &_date ) ) )
      *buf = '\0';

    restoreLocale(thisLocale);

    return buf;
  }

  static std::string adjustLocale()
  {
    const char * tmp = ::setlocale( LC_TIME, NULL );
    std::string thisLocale( tmp ? tmp : "" );

    if (    thisLocale.find( "UTF-8" ) == std::string::npos
         && thisLocale.find( "utf-8" ) == std::string::npos
         && thisLocale != "POSIX"
         && thisLocale != "C"
         && thisLocale != "" )
    {
      // language[_territory][.codeset][@modifier]
      // add/exchange codeset with UTF-8
      std::string needLocale = ".UTF-8";
      std::string::size_type loc = thisLocale.find_first_of( ".@" );
      if ( loc != std::string::npos )
      {
        // prepend language[_territory]
        needLocale = thisLocale.substr( 0, loc ) + needLocale;
        loc = thisLocale.find_last_of( "@" );
        if ( loc != std::string::npos )
        {
          // append [@modifier]
          needLocale += thisLocale.substr( loc );
        }
      }
      else
      {
        // append ".UTF-8"
        needLocale = thisLocale + needLocale;
      }
      ::setlocale( LC_TIME, needLocale.c_str() );
    }
    else
    {
      // no need to change the locale
      thisLocale.clear();
    }

    return thisLocale;
  }

  static void restoreLocale(const std::string & locale)
  {
    if ( ! locale.empty() )
      ::setlocale( LC_TIME, locale.c_str() );
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
