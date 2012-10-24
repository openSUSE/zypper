
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <string>

#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/TmpPath.h"
#include "zypp/PathInfo.h"
#include "zypp/ExternalProgram.h"

#include "zypp/base/Sysconfig.h"

using boost::unit_test::test_suite;
using boost::unit_test::test_case;
using namespace boost::unit_test;

using namespace std;
using namespace zypp;

#define DATADIR (Pathname(TESTS_SRC_DIR) + "/zypp/base/data/Sysconfig")

BOOST_AUTO_TEST_CASE(Sysconfig)
{
  Pathname file = DATADIR / "proxy";
  map<string,string> values = zypp::base::sysconfig::read(file);
  BOOST_CHECK_EQUAL( values.size(), 6 );
  BOOST_CHECK_EQUAL( values["PROXY_ENABLED"], "no");
  BOOST_CHECK_EQUAL( values["GOPHER_PROXY"], "");
  BOOST_CHECK_EQUAL( values["NO_PROXY"], "localhost, 127.0.0.1");
}

BOOST_AUTO_TEST_CASE(SysconfigWrite)
{
  Pathname file = DATADIR / "proxy";
  filesystem::TmpFile tmpf( filesystem::TmpFile::makeSibling( file ) );
  filesystem::copy( file, tmpf.path() );

  BOOST_REQUIRE_THROW( zypp::base::sysconfig::writeStringVal( "/tmp/wrzlprmpf", "PROXY_ENABLED", "yes", "# fifi\n fofo\n" ),
		       zypp::Exception );
  BOOST_CHECK( zypp::base::sysconfig::writeStringVal( tmpf.path(), "PROXY_ENABLED", "yes", "# fifi\n fofo\n" ) );
  BOOST_CHECK( !zypp::base::sysconfig::writeStringVal( tmpf.path(), "NEW1","12" ) );
  BOOST_CHECK( zypp::base::sysconfig::writeStringVal( tmpf.path(), "NEW2","13", "# fifi\n# fofo" ) );
  BOOST_CHECK( zypp::base::sysconfig::writeStringVal( tmpf.path(), "NEW3","13\"str\"", "fifi\nffofo" ) );

  std::ostringstream s;
  ExternalProgram( "diff -u " + file.asString() + " " + tmpf.path().asString() + " | tail -n +3" ) >> s;
  BOOST_CHECK_EQUAL( s.str(),
		     "@@ -8,7 +8,7 @@\n"
		     " # This setting allows to turn the proxy on and off while\n"
		     " # preserving the particular proxy setup.\n"
		     " #\n"
		     "-PROXY_ENABLED=\"no\"\n"
		     "+PROXY_ENABLED=\"yes\"\n"
		     " \n"
		     " ## Type:\tstring\n"
		     " ## Default:\t\"\"\n"
		     "@@ -49,3 +49,11 @@\n"
		     " # Example: NO_PROXY=\"www.me.de, do.main, localhost\"\n"
		     " #\n"
		     " NO_PROXY=\"localhost, 127.0.0.1\"\n"
		     "+\n"
		     "+# fifi\n"
		     "+# fofo\n"
		     "+NEW2=\"13\"\n"
		     "+\n"
		     "+# fifi\n"
		     "+# ffofo\n"
		     "+NEW3=\"13\\\"str\\\"\"\n"
  );
}

