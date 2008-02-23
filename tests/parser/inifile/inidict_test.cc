#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/InputStream.h"
#include "zypp/parser/IniDict.h"
#include "zypp/Url.h"
#include "zypp/PathInfo.h"

using std::cout;
using std::endl;
using std::string;
using std::map;
using namespace zypp;
using namespace zypp::parser;
using namespace boost::unit_test;

#define DATADIR (Pathname(TESTS_SRC_DIR) +  "/parser/inifile/data")

BOOST_AUTO_TEST_CASE(ini_read)
{
  InputStream is((DATADIR+"/1.ini"));
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

  BOOST_CHECK( dict.hasSection("addons") );
  BOOST_CHECK( !dict.hasSection("uhlala") );
  BOOST_CHECK( dict.hasEntry("contrib", "name") );
  BOOST_CHECK( !dict.hasEntry("foo", "bar") );
}

BOOST_AUTO_TEST_CASE(ini_read2)
{
  InputStream is((DATADIR+"/2.ini"));
  IniDict dict(is);

  BOOST_CHECK( find( dict.sectionsBegin(), dict.sectionsEnd(), "base" ) != dict.sectionsEnd() );
  //IniDict::entry_const_iterator i = find( dict.entriesBegin("base"), dict.entriesEnd("base"), "name");
  //BOOST_CHECK( i != dict.entriesEnd("base") );
}

// vim: set ts=2 sts=2 sw=2 ai et:
