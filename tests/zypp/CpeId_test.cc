#include <boost/test/auto_unit_test.hpp>
#include <iostream>
#include "zypp/CpeId.h"

using std::cout;
using std::endl;

using zypp::SetCompare;
using zypp::SetRelation;
using zypp::CpeId;
typedef CpeId::Value Value;

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

#define defVALUE(N,S)			\
  const std::string N##Str( S );	\
  Value N( N##Str );

defVALUE( wildcardfree,			"STrv\\*al\\?" );	// '\?'		quoted?
const std::string wildcardfreeUri(	"STrv%2aal%3f" );
const std::string wildcardfreeFs(	"STrv\\*al\\?" );

defVALUE( wildcardfree2,		"stRV\\*al\\\\\\?" );	// '\\\?'	backslash, quoted?

defVALUE( wildcarded,			"strv\\*AL?" );		// '?'		?
const std::string wildcardedUri(	"strv%2aAL%01" );
const std::string wildcardedFs(		"strv\\*AL?" );

defVALUE( wildcarded2,			"strv\\*AL\\\\?" );	// '\\?'	backslash, ?



BOOST_AUTO_TEST_CASE(cpeid_value_ANY)
{
  for ( const auto & c : { Value(), Value(nullptr), Value("*"), Value::ANY } )
  {
    BOOST_CHECK( c.isANY() );
    BOOST_CHECK( ! c.isNA() );
    BOOST_CHECK( c.isLogical() );
    BOOST_CHECK( ! c.isString() );
    BOOST_CHECK( c == Value::ANY );
    BOOST_CHECK( c == nullptr );	// ANY
    BOOST_CHECK( c != Value::NA );
    BOOST_CHECK( c != wildcardfree );
    BOOST_CHECK( c != wildcarded );
    BOOST_CHECK( ! c.isWildcardfree() );
    BOOST_CHECK( ! c.isWildcarded() );
    BOOST_CHECK_EQUAL( c.asFs(), "*" );
    BOOST_CHECK_EQUAL( c.asUri(), "" );
    BOOST_CHECK_EQUAL( c.asWfn(), "*" );
    BOOST_CHECK_EQUAL( c.asString(), c.asWfn() );
  }
}

BOOST_AUTO_TEST_CASE(cpeid_value_NA)
{
  for ( const auto & c : { Value(""), Value::NA } )
  {
    BOOST_CHECK( ! c.isANY() );
    BOOST_CHECK( c.isNA() );
    BOOST_CHECK( c.isLogical() );
    BOOST_CHECK( ! c.isString() );
    BOOST_CHECK( c != Value::ANY );
    BOOST_CHECK( c == Value::NA );
    BOOST_CHECK( c == std::string() );	// NA
    BOOST_CHECK( c == "" );		// NA
    BOOST_CHECK( c != wildcardfree );
    BOOST_CHECK( c != wildcarded );
    BOOST_CHECK( ! c.isWildcardfree() );
    BOOST_CHECK( ! c.isWildcarded() );
    BOOST_CHECK_EQUAL( c.asFs(), "-" );
    BOOST_CHECK_EQUAL( c.asUri(), "-" );
    BOOST_CHECK_EQUAL( c.asWfn(), "" );
    BOOST_CHECK_EQUAL( c.asString(), c.asWfn() );
  }
}

BOOST_AUTO_TEST_CASE(cpeid_value_string_wildcardfree)
{
  for ( const auto & c : { wildcardfree } )
  {
    BOOST_CHECK( ! c.isANY() );
    BOOST_CHECK( ! c.isNA() );
    BOOST_CHECK( ! c.isLogical() );
    BOOST_CHECK( c.isString() );
    BOOST_CHECK( c != Value::ANY );
    BOOST_CHECK( c != Value::NA );
    BOOST_CHECK( c == wildcardfree );
    BOOST_CHECK( c == wildcardfreeStr );
    BOOST_CHECK( c == wildcardfreeStr.c_str() );
    BOOST_CHECK( c != wildcarded );
    BOOST_CHECK( c.isWildcardfree() );
    BOOST_CHECK( ! c.isWildcarded() );
    BOOST_CHECK_EQUAL( c.asFs(), wildcardfreeFs );
    BOOST_CHECK_EQUAL( c.asUri(), wildcardfreeUri );
    BOOST_CHECK_EQUAL( c.asWfn(), wildcardfreeStr );
    BOOST_CHECK_EQUAL( c.asString(), c.asWfn() );
  }

  BOOST_CHECK( wildcardfree2 == wildcardfree2 );
  BOOST_CHECK( wildcardfree2 != wildcardfree );
  BOOST_CHECK( wildcardfree2 != wildcarded );
  BOOST_CHECK( wildcardfree2.isWildcardfree() );
  BOOST_CHECK( ! wildcardfree2.isWildcarded() );
}

BOOST_AUTO_TEST_CASE(cpeid_value_string_wildcarded)
{
  for ( const auto & c : { wildcarded } )
  {
    BOOST_CHECK( ! c.isANY() );
    BOOST_CHECK( ! c.isNA() );
    BOOST_CHECK( ! c.isLogical() );
    BOOST_CHECK( c.isString() );
    BOOST_CHECK( c != Value::ANY );
    BOOST_CHECK( c != Value::NA );
    BOOST_CHECK( c != wildcardfree );
#if WFN_STRICT_SPEC
    BOOST_CHECK( c != wildcarded );	// !!! According to the CPE Name Matching Specification Version 2.3
					// unquoted wildcard characters yield an undefined result (not ==).
#else
    BOOST_CHECK( c == wildcarded );
#endif
    BOOST_CHECK( ! c.isWildcardfree() );
    BOOST_CHECK( c.isWildcarded() );
    BOOST_CHECK_EQUAL( c.asFs(), wildcardedFs );
    BOOST_CHECK_EQUAL( c.asUri(), wildcardedUri );
    BOOST_CHECK_EQUAL( c.asWfn(), wildcardedStr );
    BOOST_CHECK_EQUAL( c.asString(), c.asWfn() );
  }

#if WFN_STRICT_SPEC
  BOOST_CHECK( wildcarded2 != wildcarded2 );	// unquoted wildcard characters yield an undefined result (not ==).
#else
  BOOST_CHECK( wildcarded2 == wildcarded2 );
#endif
  BOOST_CHECK( wildcarded2 != wildcardfree );
  BOOST_CHECK( wildcarded2 != wildcarded );
  BOOST_CHECK( ! wildcarded2.isWildcardfree() );
  BOOST_CHECK( wildcarded2.isWildcarded() );


}

BOOST_AUTO_TEST_CASE(cpeid_value_valid)
{
  static const char *const hdig = "0123456789abcdef";

  for ( char ch = 0; ch < CHAR_MAX; ++ch )
  {
    // cout << "==== " << unsigned(ch) << endl;
    char qchstr[] = { '\\', ch, '\0' };
    std::string chstr( qchstr+1 );
    char pchstr[] = { '%', hdig[(unsigned char)(ch)/16], hdig[(unsigned char)(ch)%16], '\0' };

    if ( ch == '\0' )
    {
      BOOST_CHECK( Value( chstr ).isNA() );
      BOOST_CHECK_THROW( (Value( chstr, Value::fsFormat )), std::invalid_argument );
      BOOST_CHECK( Value( chstr, Value::uriFormat ).isANY() );
    }
    else if ( ch <= ' ' || '~' < ch )
    {
      BOOST_CHECK_THROW( (Value( chstr )), std::invalid_argument );
      BOOST_CHECK_THROW( (Value( chstr, Value::fsFormat )), std::invalid_argument );
      BOOST_CHECK_THROW( (Value( chstr, Value::uriFormat )), std::invalid_argument );
    }
    else if ( ( '0' <= ch && ch <= '9' )
           || ( 'A' <= ch && ch <= 'Z' )
           || ( 'a' <= ch && ch <= 'z' )
           || ch == '_' )
    {
      BOOST_CHECK( Value( chstr ).isString() );
      BOOST_CHECK( Value( chstr, Value::fsFormat ).isString() );
      BOOST_CHECK( Value( chstr, Value::uriFormat ).isString() );

      BOOST_CHECK_THROW( (Value( qchstr )), std::invalid_argument );
      BOOST_CHECK( Value( qchstr, Value::fsFormat ).isString() );
      BOOST_CHECK( Value( qchstr, Value::uriFormat ).isString() );

      BOOST_CHECK( Value( pchstr, Value::uriFormat ).isString() );
    }
    else if ( ch == '*' )
    {
      BOOST_CHECK( Value( chstr ).isANY() );
      BOOST_CHECK( Value( chstr, Value::fsFormat ).isANY() );
      BOOST_CHECK( Value( chstr, Value::uriFormat ).isString() );

      BOOST_CHECK( Value( qchstr ).isString() );
      BOOST_CHECK( Value( qchstr, Value::fsFormat ).isString() );
      BOOST_CHECK( Value( qchstr, Value::uriFormat ).isString() );

      BOOST_CHECK( Value( pchstr, Value::uriFormat ).isString() );
    }
    else if ( ch == '?' )
    {
      BOOST_CHECK( Value( chstr ).isString() );
      BOOST_CHECK( Value( chstr, Value::fsFormat ).isString() );
      BOOST_CHECK( Value( chstr, Value::uriFormat ).isString() );

      BOOST_CHECK( Value( qchstr ).isString() );
      BOOST_CHECK( Value( qchstr, Value::fsFormat ).isString() );
      BOOST_CHECK( Value( qchstr, Value::uriFormat ).isString() );

      BOOST_CHECK( Value( pchstr, Value::uriFormat ).isString() );
    }
    else if ( ch == '-' )
    {
      BOOST_CHECK_THROW( (Value( chstr )), std::invalid_argument );
      BOOST_CHECK( Value( chstr, Value::fsFormat ).isNA() );
      BOOST_CHECK( Value( chstr, Value::uriFormat ).isNA() );

      BOOST_CHECK_THROW( (Value( qchstr )), std::invalid_argument );
      BOOST_CHECK( Value( qchstr, Value::fsFormat ).isString() );
      BOOST_CHECK( Value( qchstr, Value::uriFormat ).isString() );

      BOOST_CHECK( Value( pchstr, Value::uriFormat ).isString() );
    }
    else if ( ch == '\\' )
    {
      BOOST_CHECK_THROW( (Value( chstr )), std::invalid_argument );
      BOOST_CHECK_THROW( (Value( chstr, Value::fsFormat )), std::invalid_argument );
      BOOST_CHECK( (Value( chstr, Value::uriFormat )).isString() );

      BOOST_CHECK( Value( qchstr ).isString() );
      BOOST_CHECK( Value( qchstr, Value::fsFormat ).isString() );
      BOOST_CHECK( Value( qchstr, Value::uriFormat ).isString() );

      BOOST_CHECK( Value( pchstr, Value::uriFormat ).isString() );
    }
    else
    {
      BOOST_CHECK_THROW( (Value( chstr )), std::invalid_argument );
      Value f( chstr, Value::fsFormat );
      BOOST_CHECK( f.isString() );
      Value u( chstr, Value::uriFormat );
      BOOST_CHECK( u.isString() );

      BOOST_CHECK_EQUAL( f.asString(), u.asString() );
      if ( ch == '.' )
      {
	BOOST_CHECK_EQUAL( f.asFs(), chstr );
	BOOST_CHECK_EQUAL( f.asUri(), chstr );
      }
      else
      {
	BOOST_CHECK_EQUAL( f.asFs(), qchstr );
	BOOST_CHECK_EQUAL( f.asUri(), pchstr );
      }

      BOOST_CHECK( Value( qchstr ).isString() );
      BOOST_CHECK( Value( qchstr, Value::fsFormat ).isString() );
      BOOST_CHECK( Value( qchstr, Value::uriFormat ).isString() );

      BOOST_CHECK( Value( pchstr, Value::uriFormat ).isString() );
    }
  }

  BOOST_CHECK_THROW( Value( "\\!\\a\\-\\_\\.\\!" ), std::invalid_argument );
  BOOST_CHECK_EQUAL( Value( "\\!\\a\\-\\_\\.\\!", Value::fsFormat ).asFs(), "\\!a-_.\\!" );
}

BOOST_AUTO_TEST_CASE(cpeid_type_checks)
{
  for ( const auto & c : { Value::ANY, Value::NA, wildcardfree, wildcarded } )
  {
    BOOST_CHECK_EQUAL( c.isANY(),		c.type() == Value::Type::ANY );
    BOOST_CHECK_EQUAL( c.isNA(),		c.type() == Value::Type::NA );
    BOOST_CHECK_EQUAL( c.isWildcardfree(),	c.type() == Value::Type::wildcardfree );
    BOOST_CHECK_EQUAL( c.isWildcarded(),	c.type() == Value::Type::wildcarded );
    BOOST_CHECK_EQUAL( c.isLogical(),		c.isLogical( c.type() ) );
    BOOST_CHECK_EQUAL( c.isString(),		c.isString( c.type() ) );
    BOOST_CHECK_EQUAL( c.isLogical(),		! c.isString() );
  }
}

BOOST_AUTO_TEST_CASE(cpeid_compare)
{
  BOOST_CHECK( compare( Value::ANY,	Value::ANY,	SetCompare::equal		) );
  BOOST_CHECK( compare( Value::ANY,	Value::NA,	SetCompare::properSuperset	) );
  BOOST_CHECK( compare( Value::ANY,	wildcardfree,	SetCompare::properSuperset	) );
#if WFN_STRICT_SPEC
  BOOST_CHECK( compare( Value::ANY,	wildcarded,	SetCompare::uncomparable	) );
#else
  BOOST_CHECK( compare( Value::ANY,	wildcarded,	SetCompare::properSuperset	) );
#endif

  BOOST_CHECK( compare( Value::NA,	Value::ANY,	SetCompare::properSubset	) );
  BOOST_CHECK( compare( Value::NA,	Value::NA,	SetCompare::equal		) );
  BOOST_CHECK( compare( Value::NA,	wildcardfree,	SetCompare::disjoint		) );
#if WFN_STRICT_SPEC
  BOOST_CHECK( compare( Value::NA,	wildcarded,	SetCompare::uncomparable	) );
#else
  BOOST_CHECK( compare( Value::NA,	wildcarded,	SetCompare::disjoint		) );
#endif

  BOOST_CHECK( compare( wildcardfree,	Value::ANY,	SetCompare::properSubset	) );
  BOOST_CHECK( compare( wildcardfree,	Value::NA,	SetCompare::disjoint		) );
  //BOOST_CHECK( compare( wildcardfree,	wildcardfree,	_NeedsCloserLook,	// equal or disjoint
  BOOST_CHECK( compare( wildcardfree,	wildcardfree,	SetCompare::equal		) );
  BOOST_CHECK( compare( wildcardfree,	wildcardfree2,	SetCompare::disjoint		) );
#if WFN_STRICT_SPEC
  BOOST_CHECK( compare( wildcardfree,	wildcarded,	SetCompare::uncomparable	) );
#else
  //BOOST_CHECK( compare( wildcardfree,	wildcarded,	_NeedsCloserLook,	// subset or disjoint
  BOOST_CHECK( compare( wildcardfree,	wildcarded,	SetCompare::properSubset	) );
  BOOST_CHECK( compare( wildcardfree,	wildcarded2,	SetCompare::disjoint		) );
#endif

  BOOST_CHECK( compare( wildcarded,	Value::ANY,	SetCompare::properSubset	) );
  BOOST_CHECK( compare( wildcarded,	Value::NA,	SetCompare::disjoint		) );
  //BOOST_CHECK( compare( wildcarded,	wildcardfree,	_NeedsCloserLook,	// superset or disjoint
  BOOST_CHECK( compare( wildcarded,	wildcardfree,	SetCompare::properSuperset	) );
  BOOST_CHECK( compare( wildcarded,	wildcardfree2,	SetCompare::disjoint		) );
#if WFN_STRICT_SPEC
  BOOST_CHECK( compare( wildcarded,	wildcarded,	SetCompare::uncomparable	) );
#else
  //BOOST_CHECK( compare( wildcarded,	wildcarded,	_NeedsCloserLook,	// equal or uncomparable
  BOOST_CHECK( compare( wildcarded,	wildcarded,	SetCompare::equal		) );
  BOOST_CHECK( compare( wildcarded,	wildcarded2,	SetCompare::uncomparable	) );
#endif
}


BOOST_AUTO_TEST_CASE(cpeid_value_string_wildcard)
{
  for ( const auto & c : { Value( "a" ), Value( "\\*" ), Value( "\\?" ) } )
  {
    BOOST_CHECK( c.isWildcardfree() );
    BOOST_CHECK( !c.isWildcarded() );
  }

  for ( const auto & c : { Value( "*\\*" ), Value( "\\**" ), Value( "?" ), Value( "??\\?" ), Value( "\\???" ) } )
  {
    BOOST_CHECK( !c.isWildcardfree() );
    BOOST_CHECK( c.isWildcarded() );
  }
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE(cpeid_basics)
{
  BOOST_CHECK_THROW( CpeId( "malformed" ), std::invalid_argument );
  CpeId none( "malformed", CpeId::noThrow );
  BOOST_CHECK_EQUAL( CpeId::NoThrowType::lastMalformed, "malformed" );
  CpeId( "", CpeId::noThrow );
  BOOST_CHECK_EQUAL( CpeId::NoThrowType::lastMalformed, "" );

  for ( const auto & c : { CpeId(), CpeId( nullptr ), CpeId( "" ), CpeId( std::string() ), CpeId( "cpe:2.3:" ), CpeId( "cpe:/" ) } )
  {
    BOOST_CHECK( ! c );			// evaluate false in boolean context
    BOOST_CHECK_EQUAL( c.asString(), c.asFs() );
    BOOST_CHECK_EQUAL( c.asFs(),  "cpe:2.3:*:*:*:*:*:*:*:*:*:*:*" );
    BOOST_CHECK_EQUAL( c.asUri(), "cpe:/" );
    BOOST_CHECK_EQUAL( c.asWfn(), "wfn:[]" );
    BOOST_CHECK_EQUAL( c, none );	// matching!!
  }

  for ( const auto & c : { CpeId( "cpe:/o:sle" ), CpeId( "cpe:/o:*" ) } )
  {
    BOOST_CHECK( c );			// evaluate true in boolean context
    BOOST_CHECK( ! c.asString().empty() );// empty string rep
    BOOST_CHECK_EQUAL( c, c );		// matching!!
  }
}

void testStrconv( const std::string & fs, const std::string & uri, const std::string & wfn )
{
  CpeId fromFS( fs ) ;
  CpeId fromURI( uri );

  BOOST_CHECK_EQUAL( fromFS, fromURI );

  for ( const auto & c : { fromFS, fromURI } )
  {
    BOOST_CHECK_EQUAL( c.asFs(), fs );
    BOOST_CHECK_EQUAL( c.asUri(), uri );
    BOOST_CHECK_EQUAL( c.asWfn(), wfn );
  }
}

BOOST_AUTO_TEST_CASE(cpeid_strconv)
{
  // colon embedded in product value
  testStrconv ( "cpe:2.3:a:opensuse:lib\\:zypp:14.16.0:beta:*:*:*:*:*:-",
		"cpe:/a:opensuse:lib%3azypp:14.16.0:beta:~~~~~-",
		"wfn:[part=\"a\",vendor=\"opensuse\",product=\"lib\\:zypp\",version=\"14\\.16\\.0\",update=\"beta\",other=NA]" );

  testStrconv ( "cpe:2.3:a:hp:insight_diagnostics:7.4.0.1570:-:*:*:online:win2003:x64:*",
		"cpe:/a:hp:insight_diagnostics:7.4.0.1570:-:~~online~win2003~x64~",
		"wfn:[part=\"a\",vendor=\"hp\",product=\"insight_diagnostics\",version=\"7\\.4\\.0\\.1570\",update=NA,sw_edition=\"online\",target_sw=\"win2003\",target_hw=\"x64\"]" );

  testStrconv ( "cpe:2.3:a:hp:openview_network_manager:7.51:*:*:*:*:linux:*:*",
		"cpe:/a:hp:openview_network_manager:7.51::~~~linux~~",
		"wfn:[part=\"a\",vendor=\"hp\",product=\"openview_network_manager\",version=\"7\\.51\",target_sw=\"linux\"]" );

  testStrconv ( "cpe:2.3:a:foo\\\\bar:big\\$money_manager_2010:*:*:*:*:special:ipod_touch:80gb:*",
		"cpe:/a:foo%5cbar:big%24money_manager_2010:::~~special~ipod_touch~80gb~",
		"wfn:[part=\"a\",vendor=\"foo\\\\bar\",product=\"big\\$money_manager_2010\",sw_edition=\"special\",target_sw=\"ipod_touch\",target_hw=\"80gb\"]" );

  BOOST_CHECK_THROW( (CpeId( "cpe:/x:" )), std::invalid_argument );	// illegal part 'x'
  BOOST_CHECK_THROW( CpeId( "cpe:/a:foo%5cbar:big%24money_2010%07:::~~special~ipod_touch~80gb~" ), std::invalid_argument );	// illegal %07
  BOOST_CHECK_EQUAL( CpeId( "cpe:/a:foo~bar:big%7emoney_2010" ).asUri(), "cpe:/a:foo%7ebar:big%7emoney_2010" );	// unescaped ~ is ok but not preferred
}

BOOST_AUTO_TEST_CASE(cpeid_matches)
{
  CpeId sle( "cpe:/o:sles" );
  CpeId win( "cpe:/o:windows" );
  CpeId any;
  CpeId ons( "cpe:2.3:o:??????s" );
  CpeId oops( "cpe:2.3:o:?????s" );

  BOOST_CHECK_EQUAL( compare( sle, win ), SetRelation::disjoint );

  BOOST_CHECK_EQUAL( compare( sle, any ), SetRelation::subset );
  BOOST_CHECK_EQUAL( compare( win, any ), SetRelation::subset );

  BOOST_CHECK_EQUAL( compare( any, sle ), SetRelation::superset );
  BOOST_CHECK_EQUAL( compare( any, win ), SetRelation::superset );

#if WFN_STRICT_SPEC
  BOOST_CHECK_EQUAL( compare( sle, ons ), SetRelation::uncomparable );
  BOOST_CHECK_EQUAL( compare( win, ons ), SetRelation::uncomparable );
#else
  BOOST_CHECK_EQUAL( compare( sle, ons ), SetRelation::subset );
  BOOST_CHECK_EQUAL( compare( win, ons ), SetRelation::subset );
#endif

  BOOST_CHECK_EQUAL( compare( ons, sle ), SetRelation::superset );
  BOOST_CHECK_EQUAL( compare( ons, win ), SetRelation::superset );

  BOOST_CHECK_EQUAL( compare( oops, sle ), SetRelation::superset );
  BOOST_CHECK_EQUAL( compare( oops, win ), SetRelation::disjoint );

}
