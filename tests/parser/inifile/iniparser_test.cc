#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include "zypp/base/Logger.h"
#include "zypp/parser/inifile/iniparser.h"
#include "zypp/Url.h"
#include "zypp/PathInfo.h"

using std::cout;
using std::endl;
using std::string;
using namespace zypp;
using namespace boost::unit_test;


void ini_read_test(const string &dir)
{
  dictionary *d = iniparser_new((Pathname(dir)+"/1.ini").c_str());
  
  if ( d == NULL )
    ZYPP_THROW(Exception("Failed creating dictionary"));
  
  int n = iniparser_getnsec(d);
  BOOST_CHECK_EQUAL( n, 6 );
  MIL << n << endl;
  
  for ( int i = 0; i < n; i++ )
  {
    MIL << iniparser_getsecname(d, i) << endl;
  }
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
  return test;
}

// vim: set ts=2 sts=2 sw=2 ai et:
