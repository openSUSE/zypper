#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/InputStream.h"
#include "zypp/parser/IniParser.h"
#include "zypp/Url.h"
#include "zypp/PathInfo.h"

using std::cout;
using std::endl;
using std::string;
using namespace zypp;
using namespace zypp::parser;
using namespace boost::unit_test;

#define DATADIR (Pathname(TESTS_SRC_DIR) +  "/parser/inifile/data")

class IniTest : public IniParser
{
  virtual void consume( const std::string &section )
  {
    MIL << section << endl;
  }

  virtual void consume( const std::string &section, const std::string &key, const std::string &value )
  {
    MIL << "'" << section << "'" << " | " << "'" << key << "'" << " | " << "'" << value << "'" << endl;
    if (section == "base" && key == "gpgcheck")
      BOOST_CHECK_EQUAL(value, "1");
  }
};


class WithSpacesTest : public IniParser
{
  virtual void consume( const std::string &section )
  {
    MIL << section << endl;
    BOOST_CHECK(section == "base" || section == "equal" || section == "te]st");
  }

  virtual void consume( const std::string &section, const std::string &key, const std::string &value )
  {
    MIL << "'" << section << "'" << " | " << "'" << key << "'" << " | " << "'" << value << "'" << endl;
    if ( section == "base")
    {
      if ( key == "name" )
        BOOST_CHECK_EQUAL( value, "foo" );
    }
    else if ( section == "equal" )
    {
      if ( key == "name1" )
        BOOST_CHECK_EQUAL( value, "=foo" );
      else if ( key == "name2" )
        BOOST_CHECK_EQUAL( value, "f=oo" );
      else if ( key == "name3" )
        BOOST_CHECK_EQUAL( value, "foo=" );
      else
      {
        cout << "'" << section << "'" << " | " << "'" << key << "'" << " | " << "'" << value << "'" << endl;
        BOOST_CHECK_MESSAGE( false, "Unhandled key" );
      }
    }
  }
};

BOOST_AUTO_TEST_CASE(ini_read)
{
  InputStream is((DATADIR+"/1.ini"));
  IniTest parser;
  parser.parse(is);
}

BOOST_AUTO_TEST_CASE(ini_spaces_test)
{
  InputStream is((DATADIR+"/2.ini"));
  WithSpacesTest parser;
  parser.parse(is);
}
