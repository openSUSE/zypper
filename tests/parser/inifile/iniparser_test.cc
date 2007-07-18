#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test_log.hpp>

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

class IniTest : public IniParser
{
  virtual void consume( const std::string &section )
  {
    MIL << section << endl;
  }

  virtual void consume( const std::string &section, const std::string &key, const std::string &value )
  {
    MIL << "'" << section << "'" << " | " << "'" << key << "'" << " | " << "'" << value << "'" << endl;
  }
};


class WithSpacesTest : public IniParser
{
  virtual void consume( const std::string &section )
  {
    MIL << section << endl;
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

void ini_read_test(const string &dir)
{
  InputStream is((Pathname(dir)+"/1.ini"));
  IniTest parser;
  parser.parse(is);
}

void ini_spaces_test(const string &dir)
{
  InputStream is((Pathname(dir)+"/2.ini"));
  WithSpacesTest parser;
  parser.parse(is);
}

test_suite*
init_unit_test_suite( int argc, char *argv[] )
{
  string datadir;
  if (argc < 2)
  {
    datadir = TESTS_SRC_DIR;
    datadir = (Pathname(datadir) + "/parser/inifile/data").asString();
    cout << "inifile_test:"
      " path to directory with test data required as parameter. Using " << datadir  << endl;
    //return (test_suite *)0;
  }
  else
  {
    datadir = argv[1];
  }

  test_suite* test= BOOST_TEST_SUITE("ini_file");

  std::string const params[] = { datadir };
  test->add(BOOST_PARAM_TEST_CASE(&ini_read_test,
                                 (std::string const*)params, params+1));
  test->add(BOOST_PARAM_TEST_CASE(&ini_spaces_test,
                                 (std::string const*)params, params+1));
  return test;
}

// vim: set ts=2 sts=2 sw=2 ai et:
