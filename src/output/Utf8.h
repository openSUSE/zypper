#ifndef UTF8_H_
#define UTF8_H_

#include <cstdlib>
#include <cstring>
#include <wchar.h>

#include <iostream>
#include <string>

///////////////////////////////////////////////////////////////////
namespace utf8
{
  /** Simple utf8 string */
  class string
  {
  public:
    typedef std::string::size_type size_type;
    static const size_type npos = std::string::npos;

  public:
    string() {}
    string( const char * rhs )		: _str( rhs ) {}
    string( const std::string & rhs )	: _str( rhs ) {}

  public:
    const char * c_str() const			{ return _str.c_str(); }
    const std::string & str() const		{ return _str; }
    std::string & str()				{ return _str; }

  public:
    /** utf8 size */
    size_type size() const
    {
      // test for locales using dual width fonts:
      static bool isCJK = []()->bool {
	const char * lang = ::getenv( "LANG" );
	return ( lang && ( !strncmp( lang, "zh", 2 )
	                || !strncmp( lang, "ko", 2 )
			|| !strncmp( lang, "ja", 2 ) ) );
      }();

      if ( isCJK )
      {
	// this should actually be correct for ALL locales:
	size_type len = 0;
	const char *s = _str.c_str();
	for ( size_type slen = _str.size(); slen > 0; )
	{
	  wchar_t wc;
	  size_t bytes = mbrtowc( &wc, s, slen, NULL );
	  if ( bytes <= 0 )
	    break;
	  len += wcwidth( wc );
	  slen -= bytes;
	  s += bytes;
	}
	return len;
      }

      // NON CJK: faster and hopefully accurate enough:
      // simply do not count continuation bytes '10xxxxxx'
      size_type ret = _str.size();
      for ( auto ch : _str )
      {
	if ( isContinuationByte( ch ) )
	  --ret;
      }
      return ret;
    }

    /** \overload std::string has both too */
    size_type length() const
    { return size(); }

    /** utf8 substring */
    string substr( size_type pos_r = 0, size_type len_r = npos ) const
    {
      size_type p = upos( pos_r );
      size_type l = upos( len_r, p );
      return string( _str.substr( p, ( l == npos ? npos : l-p ) ) );
    }

  private:
    /** Test for continuation byte \c '10xxxxxx' */
    bool isContinuationByte( char ch ) const
    { return( (ch & 0xC0) == 0x80 ); }

    /** Return start of codepoint \a pos_r starting at position \c start_r. */
    size_type upos( size_type pos_r, size_type start_r = 0 ) const
    {
      if ( pos_r == npos || start_r > _str.size() )
	return npos;

      size_type upos = start_r;
      for ( const char * chp = _str.c_str() + upos; *chp; ++chp )
      {
	if ( ! isContinuationByte( *chp ) )
	{
	   if ( pos_r )
	     --pos_r;
	   else
	     return upos;
	}
	++upos;
      }
      return( pos_r ? npos : upos );
    }

  private:
    std::string _str;
  };

  /** \relates string concatenation */
  inline string operator+( const string & lhs, const string & rhs )
  { return string( lhs.str() + rhs.str() ); }
  /** \overload */
  inline string operator+( const string & lhs, const std::string & rhs )
  { return string( lhs.str() + rhs ); }
  /** \overload */
  inline string operator+( const std::string & lhs, const string & rhs )
  { return string( lhs + rhs.str() ); }
  /** \overload */
  inline string operator+( const string & lhs, const char * rhs )
  { return string( lhs.str() + rhs ); }
  /** \overload */
  inline string operator+( const char * lhs, const string & rhs )
  { return string( lhs + rhs.str() ); }

  /** \relates string Stream output */
  inline std::ostream & operator<<( std::ostream & str, const string & obj )
  { return str << obj.str(); }

} // namespace utf8
///////////////////////////////////////////////////////////////////

#endif // UTF8_H_
