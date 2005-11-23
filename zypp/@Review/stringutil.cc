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

  File:       stringutil.cc

  Author:     Michael Andres <ma@suse.de>
  Maintainer: Michael Andres <ma@suse.de>

  Purpose:

/-*/

#include <iostream>
#include <fstream>

#include <y2util/stringutil.h>

using namespace std;
///////////////////////////////////////////////////////////////////
namespace stringutil {
///////////////////////////////////////////////////////////////////

const unsigned tmpBuffLen = 1024;
char           tmpBuff[tmpBuffLen];

/******************************************************************
**
**
**	FUNCTION NAME : getline
**	FUNCTION TYPE : std::string
**
**	DESCRIPTION :
*/
static inline std::string _getline( std::istream & str, const Trim trim_r )
{
  string ret;
  do {
    str.clear();
    str.getline( tmpBuff, tmpBuffLen ); // always writes '\0' terminated
    ret += tmpBuff;
  } while( str.rdstate() == ios::failbit );

  return trim( ret, trim_r );
}

std::string getline( std::istream & str, const Trim trim_r )
{
  return _getline(str, trim_r);
}

std::string getline( std::istream & str, bool trim )
{
  return _getline(str, trim?TRIM:NO_TRIM);
}

/******************************************************************
**
**
**	FUNCTION NAME : split
**	FUNCTION TYPE : unsigned
**
**	DESCRIPTION :
*/
unsigned split( const std::string          line_tv,
		std::vector<std::string> & words_Vtr,
		const std::string &        sep_tv,
		const bool                 singlesep_bv )
{
  words_Vtr.clear();
  if ( line_tv.empty() )
    return words_Vtr.size();

  struct sepctrl {
    const string & sep_t;
    sepctrl( const string & sep_tv ) : sep_t( sep_tv ) {}
    // Note that '\0' ist neither Sep nor NonSep
    inline bool isSep     ( const char c )    const { return( sep_t.find( c ) != string::npos ); }
    inline bool isNonSep  ( const char c )    const { return( c && !isSep(c) ); }
    inline void skipSep   ( const char *& p ) const { while ( isSep( *p ) ) ++p; }
    inline void skipNonSep( const char *& p ) const { while ( isNonSep( *p ) ) ++p; }
  };

  sepctrl      sep_Ci( sep_tv );
  const char * s_pci = line_tv.c_str();
  const char * c_pci = s_pci;

  // Start with c_pci at the beginning of the 1st field to add.
  // In singlesep the beginning might be equal to the next sep,
  // which makes an empty field before the sep.
  if ( !singlesep_bv && sep_Ci.isSep( *c_pci ) ) {
    sep_Ci.skipSep( c_pci );
  }

  for ( s_pci = c_pci; *s_pci; s_pci = c_pci ) {
    sep_Ci.skipNonSep( c_pci );
    words_Vtr.push_back( string( s_pci, c_pci - s_pci ) );
    if ( *c_pci ) {
      if ( singlesep_bv ) {
        if ( !*(++c_pci) ) {
          // line ends with a sep -> add the empty field behind
          words_Vtr.push_back( "" );
        }
      } else
        sep_Ci.skipSep( c_pci );
    }
  }

  return words_Vtr.size();
}

/******************************************************************
**
**
**	FUNCTION NAME : join
**	FUNCTION TYPE : std::string
**
**	DESCRIPTION :
*/
std::string join( const std::vector<std::string> & words_r,
		  const std::string & sep_r )
{
  if ( words_r.empty() )
    return "";

  string ret( words_r[0] );

  for ( unsigned i = 1; i < words_r.size(); ++i ) {
    ret += sep_r + words_r[i];
  }

  return ret;
}

/******************************************************************
**
**
**	FUNCTION NAME : stripFirstWord
**	FUNCTION TYPE : std::string
**
**	DESCRIPTION :
*/
string stripFirstWord( string & line, const bool ltrim_first )
{
  if ( ltrim_first )
    line = ltrim( line );

  if ( line.empty() )
    return line;

  string ret;
  string::size_type p = line.find_first_of( " \t" );

  if ( p == string::npos ) {
    // no ws on line
    ret = line;
    line.erase();
  } else if ( p == 0 ) {
    // starts with ws
    // ret remains empty
    line = ltrim( line );
  }
  else {
    // strip word and ltim line
    ret = line.substr( 0, p );
    line = ltrim( line.erase( 0, p ) );
  }
  return ret;
}

/******************************************************************
**
**
**	FUNCTION NAME : ltrim
**	FUNCTION TYPE : std::string
**
**	DESCRIPTION :
*/
std::string ltrim( const std::string & s )
{
  if ( s.empty() )
    return s;

  string::size_type p = s.find_first_not_of( " \t\n" );
  if ( p == string::npos )
    return "";

  return s.substr( p );
}

/******************************************************************
**
**
**	FUNCTION NAME : rtrim
**	FUNCTION TYPE : std::string
**
**	DESCRIPTION :
*/
std::string rtrim( const std::string & s )
{
  if ( s.empty() )
    return s;

  string::size_type p = s.find_last_not_of( " \t\n" );
  if ( p == string::npos )
    return "";

  return s.substr( 0, p+1 );
}

/******************************************************************
**
**
**	FUNCTION NAME : toLower
**	FUNCTION TYPE : std::string
**
**	DESCRIPTION :
*/
std::string toLower( const std::string & s )
{
  if ( s.empty() )
    return s;

  string ret( s );
  for ( string::size_type i = 0; i < ret.length(); ++i ) {
    if ( isupper( ret[i] ) )
      ret[i] = static_cast<char>(tolower( ret[i] ));
  }
  return ret;
}

/******************************************************************
**
**
**	FUNCTION NAME : toUpper
**	FUNCTION TYPE : std::string
**
**	DESCRIPTION :
*/
std::string toUpper( const std::string & s )
{
  if ( s.empty() )
    return s;

  string ret( s );
  for ( string::size_type i = 0; i < ret.length(); ++i ) {
    if ( islower( ret[i] ) )
      ret[i] = static_cast<char>(toupper( ret[i] ));
  }
  return ret;
}

/******************************************************************
**
**
**	FUNCTION NAME : dumpOn
**	FUNCTION TYPE : std::ostream &
**
**	DESCRIPTION :
*/
std::ostream & dumpOn( std::ostream & str, const std::list<std::string> & l, const bool numbered )
{
  unsigned i = 0;
  for ( std::list<std::string>::const_iterator it = l.begin(); it != l.end(); ++it, ++i ) {
    if ( numbered )
      str << '[' << i << ']';
    str << *it << endl;
  }
  return str;
}

/******************************************************************
**
**
**	FUNCTION NAME : dumpOn
**	FUNCTION TYPE : std::ostream &
**
**	DESCRIPTION :
*/
std::ostream & dumpOn( std::ostream & str, const std::vector<std::string> & l, const bool numbered )
{
  for ( unsigned i = 0; i < l.size(); ++i ) {
    if ( numbered )
      str << '[' << i << ']';
    str << l[i] << endl;
  }
  return str;
}

///////////////////////////////////////////////////////////////////
}  // namespace stringutil
///////////////////////////////////////////////////////////////////

