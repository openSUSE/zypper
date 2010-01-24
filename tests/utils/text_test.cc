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
  BOOST_CHECK_EQUAL(mbs_substr_by_width(s, 0, 6), string("玄米茶"));
  // the third character cut in half, must not be included in the result
  BOOST_CHECK_EQUAL(mbs_substr_by_width(s, 0, 5), string("玄米"));

  BOOST_CHECK_EQUAL(mbs_substr_by_width(s, 6), string("空想紅茶です"));
  // the third character cut in half again, must not be included
  BOOST_CHECK_EQUAL(mbs_substr_by_width(s, 5), string("空想紅茶です"));

  // n = 0 must give empty string
  BOOST_CHECK_EQUAL(mbs_substr_by_width(s, 5, 0), string());
}

BOOST_AUTO_TEST_CASE(mbs_write_wrapped_test)
{
  // TODO
}

// vim: set ts=2 sts=8 sw=2 ai et:
