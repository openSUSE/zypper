#include <cstdlib>
#include <iostream>
#include <boost/test/auto_unit_test.hpp>

#include "zypp/Locale.h"

#define BOOST_TEST_MODULE Locale

using std::cout;
using std::endl;

using namespace zypp;
using namespace boost::unit_test;

BOOST_AUTO_TEST_CASE(static_deps)
{
  setenv( "LANG", "C", 1 );

  // static vars initialization sequence:  Locale depends on LanguageCode
  BOOST_CHECK_EQUAL( LanguageCode::enCode.code(), "en" );
  BOOST_CHECK_EQUAL( IdString(Locale::enCode),  IdString(LanguageCode::enCode) );

  // solver communication: Both must lead to the same ID
  BOOST_CHECK_EQUAL( Locale::enCode.id(),  IdString(LanguageCode::enCode.code()).id() );
}

//
// NOTE: In checks testing for empty codes (IdString::Null/IdString::Empty)
// explicitly use the ID, because both share the same string representation.
//
// This way you get "failed [1 != 0]" rather than "failed [ != ]"
//

BOOST_AUTO_TEST_CASE(no_codes)
{
  // IdString::Null is probably a rare case
  BOOST_CHECK_EQUAL( LanguageCode(nullptr).id(),	IdString::Null.id() );
  BOOST_CHECK_EQUAL( CountryCode(nullptr).id(),		IdString::Null.id() );
  BOOST_CHECK_EQUAL( Locale(nullptr).id(),		IdString::Null.id() );
  BOOST_CHECK_EQUAL( Locale(nullptr).language().id(),	IdString::Null.id() );
  BOOST_CHECK_EQUAL( Locale(nullptr).country().id(),	IdString::Null.id() );

  // IdString::Null is the ususal noCode
  BOOST_CHECK_EQUAL( LanguageCode::noCode.id(),		LanguageCode().id() );
  BOOST_CHECK_EQUAL( LanguageCode::noCode.id(),		LanguageCode("").id() );
  BOOST_CHECK_EQUAL( LanguageCode::noCode.id(),		IdString::Empty.id() );

  BOOST_CHECK_EQUAL( CountryCode::noCode.id(),		CountryCode().id() );
  BOOST_CHECK_EQUAL( CountryCode::noCode.id(),		CountryCode("").id() );
  BOOST_CHECK_EQUAL( CountryCode::noCode.id(),		IdString::Empty.id() );

  BOOST_CHECK_EQUAL( Locale::noCode.id(),		Locale().id() );
  BOOST_CHECK_EQUAL( Locale::noCode.id(),		Locale("").id() );
  BOOST_CHECK_EQUAL( Locale::noCode.id(),		IdString::Empty.id() );
  BOOST_CHECK_EQUAL( Locale::noCode.language().id(),	LanguageCode::noCode.id() );
  BOOST_CHECK_EQUAL( Locale::noCode.country().id(),	CountryCode::noCode.id() );

  //
  const char * nc = "No Code";
  BOOST_CHECK_EQUAL( LanguageCode(nullptr).name(),	nc );
  BOOST_CHECK_EQUAL( CountryCode(nullptr).name(),	nc );
  BOOST_CHECK_EQUAL( Locale(nullptr).name(),		nc );

  BOOST_CHECK_EQUAL( LanguageCode::noCode.name(),	nc );
  BOOST_CHECK_EQUAL( CountryCode::noCode.name(),	nc );
  BOOST_CHECK_EQUAL( Locale::noCode.name(),		nc );

}

BOOST_AUTO_TEST_CASE(language_code)
{
  // language code: ger deu de, N_( "German" )
  std::string name( "German" );
  for ( const char * s : { "ger", "deu", "de" } )
  {
    BOOST_CHECK_EQUAL( LanguageCode(s).code(), s );
    BOOST_CHECK_EQUAL( LanguageCode(s), IdString(s) );
    BOOST_CHECK_EQUAL( LanguageCode(s).id(), IdString(s).id() );

    BOOST_CHECK_EQUAL( LanguageCode(s).name(), name );
  }
  BOOST_CHECK( LanguageCode("de") < LanguageCode("deu") );
  BOOST_CHECK( LanguageCode("deu") < LanguageCode("ger") );

  BOOST_CHECK_EQUAL( LanguageCode("XX"), IdString("XX") );
}

BOOST_AUTO_TEST_CASE(country_code)
{
  // country code: "DE", N_("Germany)
  std::string name( "Germany" );
  for ( const char * s : { "DE" } )
  {
    BOOST_CHECK_EQUAL( CountryCode(s).code(), s );
    BOOST_CHECK_EQUAL( CountryCode(s), IdString(s) );
    BOOST_CHECK_EQUAL( CountryCode(s).id(), IdString(s).id() );

    BOOST_CHECK_EQUAL( CountryCode(s).name(), name );
  }

  BOOST_CHECK( CountryCode("AA") < CountryCode("DE") );

  BOOST_CHECK_EQUAL( CountryCode("XX"), IdString("XX") );
}

BOOST_AUTO_TEST_CASE(locale)
{
  // IdString::Null (rare)
  {
    for ( const Locale & l : { Locale( nullptr ), Locale( LanguageCode(nullptr), CountryCode(nullptr) ) } )
    {
      BOOST_CHECK_EQUAL( l.id(),		IdString::Null.id() );
      BOOST_CHECK_EQUAL( l.language().id(),	IdString::Null.id() );
      BOOST_CHECK_EQUAL( l.country().id(),	IdString::Null.id() );
      BOOST_CHECK_EQUAL( bool(l),		false );
      BOOST_CHECK_EQUAL( bool(l.language()),	false );
      BOOST_CHECK_EQUAL( bool(l.country()),	false );
    }
  }
  // Trailing garbage ([.@].*) is ignored
  {
    for ( const Locale & l : { Locale(), Locale( "" ), Locale( "@UTF-8" ), Locale( ".UTF-8" )
                             , Locale( LanguageCode(), CountryCode(nullptr) )
			     , Locale( LanguageCode(nullptr), CountryCode() )
			     , Locale( LanguageCode(), CountryCode() ) } )
    {
      BOOST_CHECK_EQUAL( l.id(),		IdString::Empty.id() );
      BOOST_CHECK_EQUAL( l.language().id(),	IdString::Empty.id() );
      BOOST_CHECK_EQUAL( l.country().id(),	IdString::Empty.id() );
      BOOST_CHECK_EQUAL( bool(l),		false );
      BOOST_CHECK_EQUAL( bool(l.language()),	false );
      BOOST_CHECK_EQUAL( bool(l.country()),	false );
    }
  }
  {
    for ( const Locale & l : { Locale("de_DE"), Locale( "de_DE@UTF-8" )
                             , Locale( LanguageCode("de"), CountryCode("DE") ) } )
    {
      BOOST_CHECK_EQUAL( l,			IdString("de_DE") );
      BOOST_CHECK_EQUAL( l.language(),		IdString("de") );
      BOOST_CHECK_EQUAL( l.country(),		IdString("DE") );
      BOOST_CHECK_EQUAL( bool(l),		true );
      BOOST_CHECK_EQUAL( bool(l.language()),	true );
      BOOST_CHECK_EQUAL( bool(l.country()),	true );
    }
  }
  {
    for ( const Locale & l : { Locale("de"), Locale( "de@UTF-8" )
                             , Locale( LanguageCode("de") ) } )
    {
      BOOST_CHECK_EQUAL( l.id(),		l.language().id() );
      BOOST_CHECK_EQUAL( l.country().id(),	IdString::Empty.id() );
      BOOST_CHECK_EQUAL( bool(l),		true );
      BOOST_CHECK_EQUAL( bool(l.language()),	true );
      BOOST_CHECK_EQUAL( bool(l.country()),	false );
    }
  }
}

BOOST_AUTO_TEST_CASE(fallback)
{
  { // default fallback...
    Locale l( "de_DE" );
    BOOST_CHECK_EQUAL( (l = l.fallback()), "de" );
    BOOST_CHECK_EQUAL( (l = l.fallback()), "en" );
    BOOST_CHECK_EQUAL( (l = l.fallback()), "" );
  }
  { // special rules...
    Locale l( "pt_BR" );
    BOOST_CHECK_EQUAL( (l = l.fallback()), "en" );
    BOOST_CHECK_EQUAL( (l = l.fallback()), "" );
  }
}
