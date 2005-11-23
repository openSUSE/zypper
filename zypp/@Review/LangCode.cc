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

   File:       LangCode.cc

   Author:     Michael Andres <ma@suse.de>
   Maintainer: Michael Andres <ma@suse.de>

/-*/

#include <iostream>

#include <y2util/LangCode.h>

using namespace std;

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : LangCode::LangCode
//	METHOD TYPE : Constructor
//
LangCode::LangCode( const std::string & code_r )
{
  string t;
  string::size_type sep = code_r.find_first_of( "@." );
  if ( sep == string::npos ) {
    t = code_r;
  } else {
    t = code_r.substr( 0, sep );
  }

  sep = t.find( '_' );
  if ( sep == string::npos ) {
    _language = ISOLanguage( t );
  } else {
    _language = ISOLanguage( t.substr( 0, sep ) );
    _country = ISOCountry( t.substr( sep+1 ) );
  }

}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : LangCode::code
//	METHOD TYPE : std::string
//
std::string LangCode::code() const
{
  string ret( languageCode() );
  if ( hasCountry() )
    ret += "_" + countryCode();
  return ret;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : LangCode::name
//	METHOD TYPE : std::string
//
std::string LangCode::name() const
{
  string ret( languageName() );
  if ( hasCountry() )
    ret += " (" + countryName() + ")";
  return ret;
}

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : std::ostream &
*/
std::ostream & operator<<( std::ostream & str, const LangCode & obj )
{
  return str << obj.code();
}
