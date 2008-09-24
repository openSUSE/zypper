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

using std::string;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace str
  { /////////////////////////////////////////////////////////////////

    /******************************************************************
     **
     **      FUNCTION NAME : form
     **      FUNCTION TYPE : std::string
    */
    std::string form( const char * format, ... )
    {
      SafeBuf safe;

      va_list ap;
      va_start( ap, format );
      vasprintf( &safe._buf, format, ap );
      va_end( ap );

      return safe.asString();
    }

    /******************************************************************
     **
     **      FUNCTION NAME : strerror
     **      FUNCTION TYPE : std::string
    */
    std::string strerror( int errno_r )
    {
      return form( "(%d)%s", errno_r, ::strerror( errno_r ) );
    }

    /******************************************************************
     **
     **      FUNCTION NAME : strToTrue
     **      FUNCTION TYPE : bool
    */
    bool strToTrue( const C_Str & str )
    {
      std::string t( toLower( str ) );
      return(    t == "1"
              || t == "yes"
              || t == "true"
              || t == "on"
            );
    }

    /******************************************************************
     **
     **      FUNCTION NAME : strToFalse
     **      FUNCTION TYPE : bool
    */
    bool strToFalse( const C_Str & str )
    {
      std::string t( toLower( str ) );
      return ! (    t == "0"
                 || t == "no"
                 || t == "false"
                 || t == "off"
               );
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
      for ( std::string::size_type i = 0; i < ret.length(); ++i )
        {
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
      for ( std::string::size_type i = 0; i < ret.length(); ++i )
        {
          if ( islower( ret[i] ) )
            ret[i] = static_cast<char>(toupper( ret[i] ));
        }
      return ret;
    }

    /******************************************************************
     **
     **      FUNCTION NAME : trim
     **      FUNCTION TYPE : std::string
    */
    std::string trim( const std::string & s, const Trim trim_r )
    {
      if ( s.empty() || trim_r == NO_TRIM )
        return s;

      std::string ret( s );

      if ( trim_r && L_TRIM )
        {
          std::string::size_type p = ret.find_first_not_of( " \t\n" );
          if ( p == std::string::npos )
            return std::string();

          ret = ret.substr( p );
        }

      if ( trim_r && R_TRIM )
        {
          std::string::size_type p = ret.find_last_not_of( " \t\n" );
          if ( p == std::string::npos )
            return std::string();

          ret = ret.substr( 0, p+1 );
        }

      return ret;
    }

    /******************************************************************
    **
    **	FUNCTION NAME : stripFirstWord
    **	FUNCTION TYPE : std::string
    */
    std::string stripFirstWord( std::string & line, const bool ltrim_first )
    {
      if ( ltrim_first )
        line = ltrim( line );

      if ( line.empty() )
        return line;

      std::string ret;
      std::string::size_type p = line.find_first_of( " \t" );

      if ( p == std::string::npos ) {
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
    **	FUNCTION NAME : stripLastWord
    **	FUNCTION TYPE : std::string
    */
    std::string stripLastWord( std::string & line, const bool rtrim_first )
    {
      if ( rtrim_first )
        line = rtrim( line );

      if ( line.empty() )
        return line;

      std::string ret;
      std::string::size_type p = line.find_last_of( " \t" );

      if ( p == std::string::npos ) {
        // no ws on line
        ret = line;
        line.erase();
      } else if ( p == line.size()-1 ) {
        // ends with ws
        // ret remains empty
        line = rtrim( line );
      }
      else {
        // strip word and rtim line
        ret = line.substr( p+1 );
        line = rtrim( line.erase( p ) );
      }
      return ret;
    }

    string gsub(const string& sData, const string& sFrom, const string& sTo)
    {
      string sNew;
      sNew.reserve(sData.size());

      if (! sData.empty())
      {
        string::size_type frLen = sFrom.length();
        string::size_type loc = 0;
        string::size_type oldLoc = 0;

        while (string::npos != (loc = sData.find(sFrom, loc)))
        {
          sNew.append(sData,oldLoc,loc-oldLoc);
          sNew.append(sTo);
          loc += frLen;
          oldLoc = loc;
          if (loc >= sData.length())
            break;
        }
        if (oldLoc!=sData.size())
            sNew.append(sData,oldLoc,sData.size()-oldLoc);
      }

      return sNew;
    }

    string & replaceAll(string & str, const string & from, const string & to)
    {
      string::size_type pos = 0;
      while((pos = str.find(from, pos)) != string::npos)
      {
        str.replace(pos, from.size(), to);
        pos += to.size();

        if (pos >= str.length())
          break;
      }
      return str;
    }


    std::string escape( const std::string & str_r, const char sep_r )
    {
      std::vector<char> buf;
      for_( s, str_r.begin(), str_r.end() )
      {
        switch ( *s )
        {
        case '"':
        case '\'':
        case '\\':
          buf.push_back( '\\' );
          buf.push_back( *s );
          break;
        default:
          if ( *s == sep_r )
            buf.push_back( '\\' );
          buf.push_back( *s );
        }
      }
      return std::string( buf.begin(), buf.end() );
    }


    /******************************************************************
    **
    **
    **      FUNCTION NAME : getline
    **      FUNCTION TYPE : std::string
    **
    **      DESCRIPTION :
    */
    static inline std::string _getline( std::istream & str, const Trim trim_r )
    {
      const unsigned tmpBuffLen = 1024;
      char           tmpBuff[tmpBuffLen];

      std::string ret;
      do {
        str.clear();
        str.getline( tmpBuff, tmpBuffLen ); // always writes '\0' terminated
        ret += tmpBuff;
      } while( str.rdstate() == std::ios::failbit );

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

    /////////////////////////////////////////////////////////////////
  } // namespace str
  ///////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////
} // namespace zypp
//////////////////////////////////////////////////////////////////
