#include "TestSetup.h"
#include "utils/text.h"

using namespace std;

BOOST_AUTO_TEST_CASE(mbs_width_test)
{
  cout << "locale set to: " << setlocale (LC_CTYPE, "en_US.UTF-8") << endl;
  unsigned width;

  width = mbs_width("Koľko stĺpcov zaberajú znaky '和平'?");
  BOOST_CHECK_EQUAL(width, 36);
}

BOOST_AUTO_TEST_CASE(mbs_substr_by_width_test)
{
  string s = "玄米茶空想紅茶です";
  BOOST_CHECK_EQUAL(mbs_substr_by_width(s, 0, 6),	"玄米茶");
  BOOST_CHECK_EQUAL(mbs_substr_by_width(s, 6),		"空想紅茶です");
  // the third character cut in halfL
  BOOST_CHECK_EQUAL(mbs_substr_by_width(s, 0, 5),	"玄米 ");
  BOOST_CHECK_EQUAL(mbs_substr_by_width(s, 5),		" 空想紅茶です");

  // n = 0 must give empty string
  BOOST_CHECK_EQUAL(mbs_substr_by_width(s, 6, 0),	"");
  BOOST_CHECK_EQUAL(mbs_substr_by_width(s, 5, 0),	"");

  // n = 1 must give (clipped) size 1 string
  BOOST_CHECK_EQUAL(mbs_substr_by_width(s, 6, 1),	" ");
  BOOST_CHECK_EQUAL(mbs_substr_by_width(s, 5, 1),	" ");

  // n = 2 must give (clipped?) size 2 string...
  BOOST_CHECK_EQUAL(mbs_substr_by_width(s, 6, 2),	"空");
  BOOST_CHECK_EQUAL(mbs_substr_by_width(s, 5, 2),	"  ");

  BOOST_CHECK_EQUAL(mbs_substr_by_width(s, 6, 3),	"空 ");
  BOOST_CHECK_EQUAL(mbs_substr_by_width(s, 5, 3),	" 空");
}

BOOST_AUTO_TEST_CASE(mbs_iterator)
{
  Zypper::instance()->config().do_colors = true;
  BOOST_CHECK_EQUAL( Zypper::instance()->config().do_colors, true );

  ColorString cs( "'和\t平'", ColorContext::NEGATIVE );
  const std::string & s( cs.str() );
  // str:	\033[22;27;31;49m ' \345\222\214 \t \345\271\263 ' \033[0m	(\t converted to L' ' in *it)
  // size:	14                1 3            1  3            1 4		= 27
  // startcol:	0                 0 1            3  4            6 7
  // columns:	0                 1 2            1  2            1 0		= 7
  // endcol:	0                 1 3            4  6            7 7
  // substrs:	\_________________/ \__________/ V  \__________/ V \_____/

  BOOST_CHECK_EQUAL( s.size(),		27 );
  BOOST_CHECK_EQUAL( mbs_width(s),	7 );

  mbs::MbsIterator it( s );
  BOOST_CHECK_EQUAL( *it,		L'\033' );
  BOOST_CHECK_EQUAL( it.size(),		14 );
  BOOST_CHECK_EQUAL( it.columns(),	0 );
  BOOST_CHECK_EQUAL( it.atEnd(),	false );
  BOOST_CHECK_EQUAL( it.isNL(),		false );
  BOOST_CHECK_EQUAL( it.isWS(),		false );
  BOOST_CHECK_EQUAL( it.isCH(),		true );	// isCH includes SGR!

  ++it;					// '
  BOOST_CHECK_EQUAL( *it,		L'\'' );
  BOOST_CHECK_EQUAL( it.size(),		1 );
  BOOST_CHECK_EQUAL( it.columns(),	1 );
  BOOST_CHECK_EQUAL( it.atEnd(),	false );
  BOOST_CHECK_EQUAL( it.isNL(),		false );
  BOOST_CHECK_EQUAL( it.isWS(),		false );
  BOOST_CHECK_EQUAL( it.isCH(),		true );

  ++it;					// 和
  BOOST_CHECK_EQUAL( *it,		L'和' );
  BOOST_CHECK_EQUAL( it.size(),		3 );
  BOOST_CHECK_EQUAL( it.columns(),	2 );
  BOOST_CHECK_EQUAL( it.atEnd(),	false );
  BOOST_CHECK_EQUAL( it.isNL(),		false );
  BOOST_CHECK_EQUAL( it.isWS(),		false );
  BOOST_CHECK_EQUAL( it.isCH(),		true );

  ++it;					// ' ' (\t converted to L' ' in *it)
  BOOST_CHECK_EQUAL( *it,		L' ' );
  BOOST_CHECK_EQUAL( it.size(),		1 );
  BOOST_CHECK_EQUAL( it.columns(),	1 );
  BOOST_CHECK_EQUAL( it.atEnd(),	false );
  BOOST_CHECK_EQUAL( it.isNL(),		false );
  BOOST_CHECK_EQUAL( it.isWS(),		true );
  BOOST_CHECK_EQUAL( it.isCH(),		false );

  ++it;
  BOOST_CHECK_EQUAL( *it,		L'平' );
  BOOST_CHECK_EQUAL( it.atEnd(),	false );
  ++it;
  BOOST_CHECK_EQUAL( *it,		L'\'' );
  BOOST_CHECK_EQUAL( it.atEnd(),	false );
  ++it;
  BOOST_CHECK_EQUAL( *it,		L'\033' );
  BOOST_CHECK_EQUAL( it.atEnd(),	false );
  ++it;
  BOOST_CHECK_EQUAL( *it,		L'\0' );
  BOOST_CHECK_EQUAL( it.atEnd(),	true );
  ++it;
  BOOST_CHECK_EQUAL( *it,		L'\0' );	// stays at end
  BOOST_CHECK_EQUAL( it.atEnd(),	true );
}
