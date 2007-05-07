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
#include "zypp/parser/IniDict.h"
#include "zypp/Url.h"
#include "zypp/PathInfo.h"

using std::cout;
using std::endl;
using std::string;
using namespace zypp;
using namespace zypp::parser;
using namespace boost::unit_test;


void ini_read_test(const string &dir)
{
  InputStream is((Pathname(dir)+"/1.ini"));
  IniDict dict(is);

  //MIL << dict["homedmacvicar"]["type"] << endl;

  for ( IniDict::section_const_iterator it = dict.sectionsBegin(); it != dict.sectionsEnd(); ++it )
  {
    MIL << (*it) << endl;
    
    for ( IniDict::entry_const_iterator it2 = dict.entriesBegin(*it); it2 != dict.entriesEnd(*it); ++it2 )
    {
      MIL << "  - " << (*it2).first << " | " << (*it2).second << endl;
    }
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
    cout << "inidict_test:"
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
