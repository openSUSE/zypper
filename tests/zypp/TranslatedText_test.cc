#include <iostream>
#include <list>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/TranslatedText.h"
#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"

#include <boost/test/unit_test.hpp>

using boost::unit_test::test_suite;
using boost::unit_test::test_case;

using namespace std;
using namespace zypp;

BOOST_AUTO_TEST_CASE(translatedtext_test)
{
  ZYpp::Ptr god;

  god = getZYpp();

  TranslatedText testTT;
  MIL << "Locale: en" << std::endl;
  god->setTextLocale(Locale("en"));
  testTT.setText("default");
  MIL << "value: '" << testTT.text() << "'" << std::endl;
  BOOST_CHECK_EQUAL( testTT.text(), "default" );

  testTT.setText("default english", Locale("en"));
  BOOST_CHECK_EQUAL( testTT.text(), "default english" );

  MIL << "Locale: es_ES" << std::endl;
  god->setTextLocale(Locale("es_ES"));

  BOOST_CHECK_EQUAL( testTT.text(), "default english" );

  testTT.setText("hola esto es neutro", Locale("es"));
  testTT.setText("this is neutral", Locale("en"));

  BOOST_CHECK_EQUAL( testTT.text(), "hola esto es neutro" );

  testTT.setText("hola Spain", Locale("es_ES"));
  BOOST_CHECK_EQUAL( testTT.text(), "hola Spain" );

  MIL << "Locale: null" << std::endl;
  god->setTextLocale(Locale());
  BOOST_CHECK_EQUAL( testTT.text(), "default" );
}

