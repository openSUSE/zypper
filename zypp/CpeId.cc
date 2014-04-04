/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/CpeId.cc
 */
#include <iostream>

#include "zypp/base/String.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/NonCopyable.h"

#include "zypp/CpeId.h"

using std::endl;

/** Initializer list with all wfn attributes */
#define WFN_ATTRIBUTES {\
  Attribute::part,	\
  Attribute::vendor,	\
  Attribute::product,	\
  Attribute::version,	\
  Attribute::update,	\
  Attribute::edition,	\
  Attribute::language,	\
  Attribute::sw_edition,\
  Attribute::target_sw,	\
  Attribute::target_hw,	\
  Attribute::other,	\
}

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace
  {
    /** Hex-digit to number or -1. */
    inline int heDecodeCh( char ch )
    {
      if ( '0' <= ch && ch <= '9' )
	return( ch - '0' );
      if ( 'A' <= ch && ch <= 'F' )
	return( ch - 'A' + 10 );
      if ( 'a' <= ch && ch <= 'f' )
	return( ch - 'a' + 10 );
      return -1;
    }

    /** Printable non whitespace in [0x00,0x7f] valid in WFN */
    inline bool chIsValidRange( char ch )
    { return( '!' <= ch && ch <= '~' ); }

    /** Alpha */
    inline bool chIsAlpha( char ch )
    { return( ( 'a' <= ch && ch <= 'z' ) || ( 'A' <= ch && ch <= 'Z' ) ); }

    /** Digit */
    inline bool chIsNum( char ch )
    { return( '0' <= ch && ch <= '9' ); }

    /** Alphanum */
    inline bool chIsAlNum( char ch )
    { return( chIsAlpha( ch ) || chIsNum( ch ) ); }

    /** Alphanum or \c underscore are unescaped in WFN */
    inline bool chIsWfnUnescaped( char ch )
    { return( chIsAlNum( ch ) || ch == '_' ); }

  } // namespace
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  /// \class CpeId::Impl
  /// \brief CpeId implementation.
  ///////////////////////////////////////////////////////////////////
  class CpeId::Impl : private base::NonCopyable
  {
    typedef std::array<Value,Attribute::numAttributes> Wfn;

  public:
    Impl() {}

    Impl( const std::string & cpe_r )
      : _wfn( unbind( cpe_r ) )
    {}

  public:
    explicit operator bool() const
    { for ( const auto & val : _wfn ) if ( ! val.isANY() ) return true; return false; }

    std::string asFs() const
    {
      str::Str ret;
      ret << "cpe:2.3";
      for ( auto ai : WFN_ATTRIBUTES )
      {
	ret << ':' << _wfn[ai].asFs();
      }
      return ret;
    }

    std::string asUri() const
    {
      str::Str ret;
      ret << "cpe:/";
      std::string val;
      unsigned colon = 0;	// to remember trailing colons
      for ( auto ai : WFN_ATTRIBUTES )
      {
	val = _wfn[ai].asUri();

	if ( ai == Attribute::edition )
	{
	  if ( ! ( _wfn[Attribute::sw_edition].isANY()
	        && _wfn[Attribute::target_sw].isANY()
	        && _wfn[Attribute::target_hw].isANY()
	        && _wfn[Attribute::other].isANY() ) )
	  {
	    // packing is needed
	    val = str::Str()
		  << '~' << val//Attribute::edition
	          << '~' << _wfn[Attribute::sw_edition].asUri()
	          << '~' << _wfn[Attribute::target_sw].asUri()
	          << '~' << _wfn[Attribute::target_hw].asUri()
		  << '~' << _wfn[Attribute::other].asUri();
	  }
	}

	if ( ! val.empty() )
	{
	  if ( colon )
	    ret << std::string( colon, ':' );
	  ret << val;
	  colon = 1;
	}
	else
	  ++colon;

	if ( ai == Attribute::language )
	  break;	// remaining attrs packaed in edition
      }
      return ret;
    }

    std::string asWfn() const
    {
      str::Str ret;
      ret << "wfn:[";
      for ( auto ai : WFN_ATTRIBUTES )
      {
	const Value & val( _wfn[ai] );
	if ( ! val.isANY() )
	{
	  if ( ai ) ret << ',';
	  ret << Attribute::asString( ai ) << '=';
	  if ( val.isString() )
	    ret << '"' << val << '"';
	  else
	    ret << "NA";	// as ANY is omitted, it must be NA
	}
      }
      return ret << "]";
    }

  public:
    SetCompare setRelationMixinCompare( const Impl & trg ) const
    {
      SetCompare ret = SetCompare::equal;
      for ( auto ai : WFN_ATTRIBUTES )
      {
	switch ( _wfn[ai].compare( trg._wfn[ai] ).asEnum() )
	{
	  case SetCompare::uncomparable:
	    ret = SetCompare::uncomparable;
	    break;

	  case SetCompare::equal:
	    break;

	  case SetCompare::properSubset:
	    if ( ret == SetCompare::equal )
	      ret = SetCompare::properSubset;
	    else if ( ret != SetCompare::properSubset )
	      ret = SetCompare::uncomparable;
	    break;

	  case SetCompare::properSuperset:
	    if ( ret == SetCompare::equal )
	      ret = SetCompare::properSuperset;
	    else if ( ret != SetCompare::properSuperset )
	      ret = SetCompare::uncomparable;
	    break;

	  case SetCompare::disjoint:
	    ret = SetCompare::disjoint;
	    break;
	}
	if ( ret == SetCompare::uncomparable || ret == SetCompare::disjoint )
	  break;
      }
      return ret;
    }

  private:
    /** Assign \a val_r if it meets \a attr_r specific contraints.
     * \throws std::invalid_argument if string is malformed
     */
    static void assignAttr( Wfn & wfn_r, Attribute attr_r, const Value & val_r )
    {
      if ( val_r.isString() )
      {
	switch ( attr_r.asEnum() )
	{
	  case Attribute::part:
	    {
	      const std::string & wfn( val_r.asWfn() );
	      switch ( wfn[0] )
	      {
		case 'h':
		case 'o':
		case 'a':
		  if ( wfn[1] == '\0' )
		    break;
		  // else: fallthrough
		default:
		  throw std::invalid_argument( "CpeId:Wfn:part: illegal value" );
		  break;
	      }
	    }
	    break;

	  case Attribute::language:
	    {
	      const std::string & wfn( val_r.asWfn() );
	      std::string::size_type len = 0;
	      // (2*3ALPHA) ["-" (2ALPHA / 3DIGIT)]
	      if ( chIsAlpha( wfn[0] ) && chIsAlpha( wfn[1] ) )
	      {
		len = chIsAlpha( wfn[2] ) ? 3 : 2;
		if ( wfn[len] == '-' )
		{
		  if ( chIsAlpha( wfn[len+1] ) && chIsAlpha( wfn[len+2] ) )
		    len += 3;
		  else if ( chIsNum( wfn[len+1] ) && chIsNum( wfn[len+2] ) && chIsNum( wfn[len+3] ) )
		    len += 4;
		}
	      }
	      if ( wfn.size() != len )
		throw std::invalid_argument( "CpeId:Wfn:language: illegal value" );
	    }
	    break;

	  default:
	    // no contraints
	    break;
	}
      }
      wfn_r[attr_r.asIntegral()] = val_r;
    }

  private:
    /** Parse magic and unbind accordingly
     * \throws std::invalid_argument if string is malformed
     */
    static Wfn unbind( const std::string & cpe_r );

    /** Parse Uri and unbind
     * \throws std::invalid_argument if string is malformed
     */
    static Wfn unbindUri( const std::string & cpe_r );

    /** Parse Fs and unbind
     * \throws std::invalid_argument if string is malformed
     */
    static Wfn unbindFs( const std::string & cpe_r );

  private:
    Wfn _wfn;
  };

  CpeId::Impl::Wfn CpeId::Impl::unbind( const std::string & cpe_r )
  {
    Wfn ret;
    if ( cpe_r[0] == 'c'
      && cpe_r[1] == 'p'
      && cpe_r[2] == 'e'
      && cpe_r[3] == ':' )
    {
      if ( cpe_r[4] == '/' )
      {
	ret = unbindUri( cpe_r );
      }
      else if ( cpe_r[4] == '2'
	     && cpe_r[5] == '.'
	     && cpe_r[6] == '3'
	     && cpe_r[7] == ':' )
      {
	ret = unbindFs( cpe_r );
      }
      else
	throw std::invalid_argument( "CpeId: bad magic" );
    }
    else if ( cpe_r[0] != '\0' )
      throw std::invalid_argument( "CpeId: bad magic" );
    return ret;
  }

  CpeId::Impl::Wfn CpeId::Impl::unbindUri( const std::string & cpe_r )
  {
    Wfn ret;

    std::vector<std::string> field;
    field.reserve( Attribute::numAttributes );
    if ( str::splitFields( cpe_r.c_str()+5/* skip magic 'cpe:/' */, std::back_inserter(field), ":" ) > Attribute::numAttributes )
      throw std::invalid_argument( "CpeId:Uri: too many fields" );
    field.resize( Attribute::numAttributes );	// fillup with ANY("")

    for ( auto ai : WFN_ATTRIBUTES )
    {
      if ( ai == Attribute::edition && field[ai][0] == '~' )
      {
	// unpacking is needed
	static constexpr unsigned numPacks = 6u;	// dummy_before_~ + edition + 4 extended attributes
	std::vector<std::string> pack;
	pack.reserve( numPacks );
	if ( str::splitFields( field[ai], std::back_inserter(pack), "~" ) > numPacks )
	  throw std::invalid_argument( "CpeId:Uri: too many packs" );
	pack.resize( numPacks );	// fillup with ANY(""), should be noOP

	pack[1].swap( field[Attribute::edition] );
	pack[2].swap( field[Attribute::sw_edition] );
	pack[3].swap( field[Attribute::target_sw] );
	pack[4].swap( field[Attribute::target_hw] );
	pack[5].swap( field[Attribute::other] );
      }
      assignAttr( ret, ai, Value( field[ai], Value::uriFormat ) );
    }
    return ret;
  }

  CpeId::Impl::Wfn CpeId::Impl::unbindFs( const std::string & cpe_r )
  {
    Wfn ret;

    std::vector<std::string> field;
    field.reserve( Attribute::numAttributes );
    if ( str::splitFields( cpe_r.c_str()+8/* skip magic 'cpe:2.3:' */, std::back_inserter(field), ":" ) > Attribute::numAttributes )
      throw std::invalid_argument( "CpeId:Fs: too many fields" );
    field.resize( Attribute::numAttributes, "*" );	// fillup with ANY|"*"

    for ( auto ai : WFN_ATTRIBUTES )
    {
      assignAttr( ret, ai, Value( field[ai], Value::fsFormat ) );
    }
    return ret;
  }


  ///////////////////////////////////////////////////////////////////
  //	class CpeId
  ///////////////////////////////////////////////////////////////////

  std::string CpeId::NoThrowType::lastMalformed;

  CpeId::CpeId()
    : _pimpl( new Impl )
  {}

  CpeId::CpeId( const std::string & cpe_r )
    : _pimpl( new Impl( cpe_r ) )
  {}

  CpeId::CpeId( const std::string & cpe_r, NoThrowType )
  {
    try
    {
      _pimpl.reset( new Impl( cpe_r ) );
      NoThrowType::lastMalformed.clear();
    }
    catch(...)
    {
      _pimpl.reset( new Impl );
      NoThrowType::lastMalformed = cpe_r;
    }
  }

  CpeId::~CpeId()
  {}

  CpeId::operator bool() const
  { return bool(*_pimpl); }

  std::string CpeId::asFs() const
  { return _pimpl->asFs(); }

  std::string CpeId::asUri() const
  { return _pimpl->asUri(); }

  std::string CpeId::asWfn() const
  { return _pimpl->asWfn(); }

  SetCompare CpeId::setRelationMixinCompare( const CpeId & trg ) const
  { return _pimpl->setRelationMixinCompare( *trg._pimpl ); }

  ///////////////////////////////////////////////////////////////////
  //	class CpeId::WfnAttribute
  ///////////////////////////////////////////////////////////////////

  const std::string & CpeId::_AttributeDef::asString( Enum val_r )
  {
    static std::map<Enum,std::string> _table = {
#define OUTS(N) { N, #N }
      OUTS( part ),
      OUTS( vendor ),
      OUTS( product ),
      OUTS( version ),
      OUTS( update ),
      OUTS( edition ),
      OUTS( language ),
      OUTS( sw_edition ),
      OUTS( target_sw ),
      OUTS( target_hw ),
      OUTS( other ),
#undef OUTS
    };
    return _table[val_r];
  }

  ///////////////////////////////////////////////////////////////////
  //	class CpeId::Value
  ///////////////////////////////////////////////////////////////////

  const CpeId::Value CpeId::Value::ANY;
  const CpeId::Value CpeId::Value::NA( "" );

  CpeId::Value::Value( const std::string & value_r )
  {
    if ( value_r.empty() )	// NA
    {
      if ( ! CpeId::Value::NA._value )	// initialized by this ctor!
	_value.reset( new std::string );
      else
	_value = CpeId::Value::NA._value;
    }
    else if ( value_r != "*" )	// ANY is default constructed
    {
      bool starting = true;	// false after the 1st non-?
      for_( chp, value_r.begin(), value_r.end() )
      {
	switch ( *chp )
	{
	  case '\\':	// quoted
	    ++chp;
	    if ( ! chIsValidRange( *chp )  )
	    {
	      if ( *chp )
		throw std::invalid_argument( "CpeId:Wfn: illegal quoted character" );
	      else
		throw std::invalid_argument( "CpeId:Wfn: Backslash escapes nothing" );
	    }
	    else if ( chIsWfnUnescaped( *chp ) )
	      throw std::invalid_argument( "CpeId:Wfn: unnecessarily quoted character" );
	    else if ( starting && *chp == '-' && chp+1 == value_r.end() )
	      throw std::invalid_argument( "CpeId:Wfn: '\\-' is illegal value" );
	    break;

	  case '?':	// sequence at beginning or end of string
	    while ( *(chp+1) == '?' )
	      ++chp;
	    if ( ! ( starting || chp+1 == value_r.end() ) )
	      throw std::invalid_argument( "CpeId:Wfn: embedded ?" );
	    break;

	  case '*':	// single at beginning or end of string
	    if ( ! ( starting || chp+1 == value_r.end() ) )
	      throw std::invalid_argument( "CpeId:Wfn: embedded *" );
	    break;

	  default:	// everything else unquoted
	    if ( ! chIsWfnUnescaped( *chp ) )
	    {
	      if ( chIsValidRange( *chp ) )
		throw std::invalid_argument( "CpeId:Wfn: missing quote" );
	      else
		throw std::invalid_argument( "CpeId:Wfn: illegal character" );
	    }
	    break;
	}
	if ( starting )
	  starting = false;
      }
      _value.reset( new std::string( value_r ) );
    }
  }

  CpeId::Value::Value( const std::string & encoded_r, FsFormatType )
  {
    if ( encoded_r != "*" )	// ANY is default constructed
    {
      if ( encoded_r == "-" )	// NA
      {
	_value = CpeId::Value::NA._value;
      }
      else
      {
	str::Str result;
	bool starting = true;	// false after the 1st non-?
	for_( chp, encoded_r.begin(), encoded_r.end() )
	{
	  switch ( *chp )
	  {
	    case '\\':	// may stay quoted
	      ++chp;
	      if ( chIsWfnUnescaped( *chp ) )
		result << *chp;
	      else if ( chIsValidRange( *chp ) )
		result << '\\' << *chp;
	      else if ( *chp )
		throw std::invalid_argument( "CpeId:Fs: illegal quoted character" );
	      else
		throw std::invalid_argument( "CpeId:Fs: Backslash escapes nothing" );
	      break;

	    case '?':	// sequence at beginning or end of string
	      result << '?';
	      while ( *(chp+1) == '?' )
	      {
		++chp;
		result << '?';
	      }
	      if ( ! ( starting || chp+1 == encoded_r.end() ) )
		throw std::invalid_argument( "CpeId:Fs: embedded ?" );
	      break;

	    case '*':	// single at beginning or end of string
	      if ( starting || chp+1 == encoded_r.end() )
		result << '*';
	      else
		throw std::invalid_argument( "CpeId:Fs: embedded *" );
	      break;

	    default:
	      if ( chIsWfnUnescaped( *chp ) )
		result << *chp;
	      else if ( chIsValidRange( *chp ) )
		result << '\\' << *chp;
	      else
		throw std::invalid_argument( "CpeId:Fs: illegal character" );
	      break;
	  }
	  if ( starting )
	    starting = false;
	}
	if ( starting )
	  throw std::invalid_argument( "CpeId:Fs: '' is illegal" );
	_value.reset( new std::string( result ) );
      }
    }
  }

  CpeId::Value::Value( const std::string & encoded_r, UriFormatType )
  {
    if ( ! encoded_r.empty() )	// ANY is default constructed
    {
      if ( encoded_r == "-" )	// NA
      {
	_value = CpeId::Value::NA._value;
      }
      else
      {
	str::Str result;
	bool starting = true;	// false after the 1st non-? (%01)
	for_( chp, encoded_r.begin(), encoded_r.end() )
	{
	  char ch = *chp;

	  if ( ch == '%' )	// legal '%xx' sequence first
	  {
	    int d1 = heDecodeCh( *(chp+1) );
	    if ( d1 != -1 )
	    {
	      int d2 = heDecodeCh( *(chp+2) );
	      if ( d2 != -1 )
	      {
		chp += 2;	// skip sequence
		if ( d1 == 0 )
		{
		  if ( d2 == 1 )	// %01 - ? valid sequence at begin or end
		  {
		    result << '?';
		    while ( *(chp+1) == '%' && *(chp+2) == '0' && *(chp+3) == '1' )
		    {
		      chp += 3;
		      result << '?';
		    }
		    if ( starting || chp+1 == encoded_r.end() )
		    {
		      starting = false;
		      continue;	// -> continue;
		    }
		    else
		      throw std::invalid_argument( "CpeId:Uri: embedded %01" );
		  }
		  else if ( d2 == 2 )	// %02 - * valid at begin or end
		  {
		    if ( starting || chp+1 == encoded_r.end() )
		    {
		      result << '*';
		      starting = false;
		      continue;	// -> continue;
		    }
		    else
		      throw std::invalid_argument( "CpeId:Uri: embedded %02" );
		  }
		}
		ch = (d1<<4)|d2;
		if ( ! chIsValidRange( ch ) )
		  throw std::invalid_argument( "CpeId:Uri: illegal % encoded character" );
	      }
	    }
	  }
	  else if ( ! chIsValidRange( ch ) )
	    throw std::invalid_argument( "CpeId:Uri: illegal character" );

	  if ( chIsWfnUnescaped( ch ) )
	    result << ch;
	  else
	    result << '\\' << ch;

	  if ( starting )
	    starting = false;
	}
	_value.reset( new std::string( result ) );
      }
    }
  }

  std::string CpeId::Value::asWfn() const
  {
    std::string ret;
    if ( ! _value )
    {
      static const std::string any( "*" );
      ret = any;
    }
    else
      ret = *_value;	// includes "" for NA
    return ret;
  }

  std::string CpeId::Value::asFs() const
  {
    std::string ret;
    if ( isANY() )
    {
      static const std::string asterisk( "*" );
      ret = asterisk;
    }
    else if ( isNA() )
    {
      static const std::string dash( "-" );
      ret = dash;
    }
    else
    {
      str::Str result;
      for_( chp, _value->begin(), _value->end() )
      {
	if ( *chp != '\\' )
	  result << *chp;
	else
	{
	  ++chp;
	  switch ( *chp )
	  {
	    case '-':
	    case '.':
	    case '_':
	      result << *chp;	// without escaping
	      break;

	    case '\0':
	      throw std::invalid_argument( "CpeId:Wfn: Backslash escapes nothing" );
	      break;

	    default:
	      result << '\\' << *chp;
	      break;
	  }
	}
      }
      ret = result;
    }
    return ret;
  }

  std::string CpeId::Value::asUri() const
  {
    std::string ret;	// ANY
    if ( ! isANY() )
    {
      if ( isNA() )
      {
	static const std::string dash( "-" );
	ret = dash;
      }
      else
      {
	str::Str result;
	for_( chp, _value->begin(), _value->end() )
	{
	  if ( chIsWfnUnescaped( *chp ) )
	  {
	    result << *chp;
	  }
	  else
	  {
	    static const char *const hdig = "0123456789abcdef";
	    switch ( *chp )
	    {
	      case '\\':
		++chp;
		switch ( *chp )
		{
		  case '-':
		  case '.':
		    result << *chp;	// without encodeing
		    break;

		  case '\0':
		    throw std::invalid_argument( "CpeId:Wfn: Backslash escapes nothing" );
		    break;

		  default:
		    result << '%' << hdig[(unsigned char)(*chp)/16] << hdig[(unsigned char)(*chp)%16];
		    break;
		}
		break;

	      case '?':
		result << "%01";
		break;

	      case '*':
		result << "%02";
		break;

	      default:
		throw std::invalid_argument( str::Str() << "CpeId:Wfn: illegal char '" << *chp << "' in WFN" );
		break;
	    }
	  }
	}
	ret = result;
      }
    }
    return ret;
  }

  ///////////////////////////////////////////////////////////////////
  namespace
  {
    /** Whether it's a wildcard character (<tt>[*?]</tt>). */
    inline bool isWildchar( char ch_r )
    { return( ch_r == '*' || ch_r == '?' ); }

    /** Whether there is an even number of consecutive backslashes before and including \a rbegin_r
     * An even number of backslashes means the character following is unescaped.
     */
    inline bool evenNumberOfBackslashes( std::string::const_reverse_iterator rbegin_r, std::string::const_reverse_iterator rend_r )
    {
      unsigned backslashes = 0;
      for_( it, rbegin_r, rend_r )
      {
	if ( *it == '\\' )
	  ++backslashes;
	else
	  break;
      }
      return !(backslashes & 1U);
    }

    /** Number of chars (not counting escaping backslashes) in <tt>[begin_r,end_r[</tt> */
    inline unsigned trueCharsIn( const std::string & str_r, std::string::size_type begin_r, std::string::size_type end_r )
    {
      unsigned chars = 0;
      for_( it, begin_r, end_r )
      {
	++chars;
	if ( str_r[it] == '\\' )
	{
	  if ( ++it == end_r )
	    break;
	}
      }
      return chars;
    }

    /** Match helper comparing 2 Wildcardfree string values (case insensitive). */
    inline bool matchWildcardfreeString( const std::string & lhs, const std::string & rhs )
    { return( str::compareCI( lhs, rhs ) == 0 ); }

    /** Match helper matching Wildcarded source against Wildcardfree target.
     *
     * Constraints on usage of the unquoted question mark (zero or one char in \a trg):
     * 1. An unquoted question mark MAY be used at the beginning and/or the end of an
     *    attribute-value string.
     * 2. A contiguous sequence of unquoted question marks MAY appear at the beginning
     *    and/or the end of an attribute-value string.
     * 3. An unquoted question mark SHALL NOT be used in any other place in an
     *    attribute-value string.
     *
     * Constraints on usage of the unquoted asterisk  (zero or more chars in \a trg):
     * 1. A single unquoted asterisk MAY be used as the entire attribute-value string.
     * 2. A single unquoted asterisk MAY be used at the beginning and/or end of an
     *    attribute-value string.
     * 3. An unquoted asterisk SHALL NOT be used in any other place in an attribute-value
     *    string.
     *
     * Unquoted question marks and asterisks MAY appear in the same attribute-value string
     * as long as they meet the constraints above.
     *
     * Example of illegal usage: "foo?bar", "bar??baz", "q??x",
     *                           "foo*bar", "**foo", "bar***",
     *                           "*?foobar", "foobar*?"
     *
     * \note Relies on \a src and \a trg being wellformed.
     */
    inline bool matchWildcardedString( std::string src, std::string trg )
    {
      // std::string::npos remembers an asterisk
      // unescaped wildcard prefix
      std::string::size_type prefx = 0;
      switch ( *src.begin() )	// wellformed implies not empty
      {
	case '*':
	  if ( src.size() == 1 )
	    return true;	// "*" matches always: superset
	  else
	    prefx = std::string::npos;
	    src.erase( 0, 1 );
	  break;
	case '?':
	  ++prefx;
	  for_( it, ++src.begin(), src.end() )
	  { if ( *it == '?' ) ++prefx; else break; }
	  if ( src.size() == prefx )
	    return( trg.size() <= prefx );	// "??..?": superset if at most #prefx chars
	  else
	    src.erase( 0, prefx );
	  break;
	default:
	  break;
      }
      // unescaped wildcard suffix
      std::string::size_type suffx = 0;
      if ( ! src.empty() )
      {
	switch ( *src.rbegin() )
	{
	  case '*':
	    if ( evenNumberOfBackslashes( ++src.rbegin(), src.rend() ) )
	    {
	      suffx = std::string::npos;
	      src.erase( src.size()-1 );
	    }
	    break;
	  case '?':
	    ++suffx;
	    for_( it, ++src.rbegin(), src.rend() )
	    { if ( *it == '?' ) ++suffx; else break; }
	    if ( ! evenNumberOfBackslashes( src.rbegin()+suffx, src.rend() ) )
	      --suffx;	// last '?' was escaped.
	    src.erase( src.size()-suffx );
	    break;
	  default:
	    break;
	}
      }
      // now match; find src in trg an check surrounding wildcards
      src = str::toLower( src );
      trg = str::toLower( trg );
      for ( std::string::size_type match = trg.find( src, 0 );
	    match != std::string::npos;
            match = trg.find( src, match+1 ) )
      {
	if ( prefx != std::string::npos && trueCharsIn( trg, 0, match ) > prefx )
	  break;	// not "*", and already more chars than "?"s before match: disjoint
	std::string::size_type frontSize = match + src.size();
	if ( suffx != std::string::npos && trueCharsIn( trg, frontSize, trg.size() ) > suffx )
	  continue;	// not "*", and still more chars than "?"s after match: check next match
	return true;	// match: superset
      }
      return false;	// disjoint
    }
  } // namespace
  ///////////////////////////////////////////////////////////////////

  bool CpeId::Value::containsWildcard() const
  {
    const std::string & value( *_value );
    return ( isWildchar( *value.begin() )
	|| ( isWildchar( *value.rbegin() ) && evenNumberOfBackslashes( ++value.rbegin(), value.rend() ) ) );
  }

  ///////////////////////////////////////////////////////////////////
  /// Symmetric attribute compare if wildcards are involved!
  /// The specs define any comarison with a wildcarded attribute as
  /// target to return \c uncomparable:
  /// \code
  ///    wildcardfree  <=>  wildcarded    ==>  uncomparable,
  ///    wildcarded    <=>  wildcardfree  ==>  superset or disjoint
  /// \endcode
  /// But a symmetric result is much more intuitive:
  /// \code
  ///    wildcardfree  <=>  wildcarded    ==>  subset or disjoint
  ///    wildcarded    <=>  wildcardfree  ==>  superset or disjoint
  /// \endcode
  ///////////////////////////////////////////////////////////////////
#define WFN_STRICT_SPEC 0
#if WFN_STRICT_SPEC
  //SetCompare CpeId::Value::setRelationMixinCompare( const CpeId::Value & trg ) const
  {
    static const SetCompare _NeedsCloserLook( SetCompare::Enum(-1) );	// artificial Compare value
    static const SetCompare matchTabel[4][4] = {{
      /* ANY,		ANY		*/ SetCompare::equal,
      /* ANY,		NA		*/ SetCompare::properSuperset,
      /* ANY,		wildcardfree	*/ SetCompare::properSuperset,
      /* ANY,		wildcarded	*/ SetCompare::uncomparable,
    },{
      /* NA,		ANY		*/ SetCompare::properSubset,
      /* NA,		NA		*/ SetCompare::equal,
      /* NA,		wildcardfree	*/ SetCompare::disjoint,
      /* NA,		wildcarded	*/ SetCompare::uncomparable,
    },{
      /* wildcardfree,	ANY		*/ SetCompare::properSubset,
      /* wildcardfree,	NA		*/ SetCompare::disjoint,
      /* wildcardfree,	wildcardfree	*/ _NeedsCloserLook,	// equal or disjoint
      /* wildcardfree,	wildcarded	*/ SetCompare::uncomparable,
    },{
      /* wildcarded,	ANY		*/ SetCompare::properSubset,
      /* wildcarded,	NA		*/ SetCompare::disjoint,
      /* wildcarded,	wildcardfree	*/ _NeedsCloserLook,	// superset or disjoint
      /* wildcarded,	wildcarded	*/ SetCompare::uncomparable,
    }};

    Type srcType = type();
    Type trgType = trg.type();
    SetCompare ret = matchTabel[srcType.asIntegral()][trgType.asIntegral()];
    if ( ret == _NeedsCloserLook )
    {
      if ( srcType == Type::wildcardfree )	// trgType == Type::wildcardfree
      {
	// simple string compare
	ret = matchWildcardfreeString( *_value, *trg._value ) ? SetCompare::equal : SetCompare::disjoint;
      }
      else if ( srcType == Type::wildcarded )	// trgType == Type::wildcardfree
      {
	// Needs wildcard compare
	ret = matchWildcardedString( *_value, *trg._value ) ? SetCompare::properSuperset : SetCompare::disjoint;
     }
    }
    return ret;
  }
#else
  SetCompare CpeId::Value::setRelationMixinCompare( const CpeId::Value & trg ) const
  {
    ///////////////////////////////////////////////////////////////////
    // ANY,		ANY		=> equal
    // ANY,		NA		=> properSuperset
    // ANY,		wildcardfree	=> properSuperset
    // ANY,		wildcarded	=> properSuperset
    //
    // NA,		ANY		=> properSubset
    // NA,		NA		=> equal
    // NA,		wildcardfree	=> disjoint
    // NA,		wildcarded	=> disjoint
    //
    // wildcardfree,	ANY		=> properSubset
    // wildcardfree,	NA		=> disjoint
    // wildcardfree,	wildcardfree	=> NeedsCloserLook:	equal or disjoint
    // wildcardfree,	wildcarded	=> NeedsCloserLook:	subset or disjoint
    //
    // wildcarded,	ANY		=> properSubset
    // wildcarded,	NA		=> disjoint
    // wildcarded,	wildcardfree	=> NeedsCloserLook:	superset or disjoint
    // wildcarded,	wildcarded	=> NeedsCloserLook"	equal or uncomparable
    ///////////////////////////////////////////////////////////////////

    SetCompare ret = SetCompare::disjoint;

    if ( isANY() )
    {
      ret = trg.isANY() ? SetCompare::equal : SetCompare::properSuperset;
    }
    else if ( trg.isANY() )
    {
      ret = SetCompare::properSubset;
    }
    else if ( isNA() )
    {
      if ( trg.isNA() ) ret = SetCompare::equal; // else: SetCompare::disjoint;
    }
    else if ( ! trg.isNA() ) // else: SetCompare::disjoint;
    {
      // NeedsCloserLook:
      if ( isWildcarded() )
      {
	if ( trg.isWildcarded() )
	{
	  // simple string compare just to detect 'equal'
	  ret = matchWildcardfreeString( *_value, *trg._value ) ? SetCompare::equal : SetCompare::uncomparable;
	}
	else
	{
	  // Needs wildcard compare (src,trg)
	  if ( matchWildcardedString( *_value, *trg._value ) ) ret = SetCompare::properSuperset; // else: SetCompare::disjoint;
	}
      }
      else
      {
	if ( trg.isWildcarded() )
	{
	  // Needs wildcard compare (trg,src)
	  if ( matchWildcardedString( *trg._value, *_value ) ) ret = SetCompare::properSubset; // else: SetCompare::disjoint;
	}
	else
	{
	  // simple string compare
	  if ( matchWildcardfreeString( *_value, *trg._value ) ) ret = SetCompare::equal; // else: SetCompare::disjoint;
	}
      }
    }
    return ret;
  }
#endif // WFN_STRICT_SPEC

  std::ostream & operator<<( std::ostream & str, const CpeId::Value & obj )
  { return str << obj.asString(); }

} // namespace zypp
///////////////////////////////////////////////////////////////////
