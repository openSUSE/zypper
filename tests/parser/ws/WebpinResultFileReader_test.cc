#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <boost/test/auto_unit_test.hpp>

#include "zypp/parser/ws/WebpinResultFileReader.h"
#include "zypp/ws/WebpinResult.h"

#include "zypp/Url.h"
#include "zypp/PathInfo.h"

using namespace std;
using namespace zypp;
using namespace boost::unit_test;

using namespace zypp::ws;
using namespace zypp::parser::ws;

#define DATADIR (Pathname(TESTS_SRC_DIR) + "parser/ws/data")

class Collector
{
public:
  Collector()
  {}
  
  bool callback( const WebpinResult &result )
  {
    items.push_back(result);
    //items.push_back(loc);
    //cout << items.size() << endl;
    return true;
  }
  
  vector<WebpinResult> items;
};

BOOST_AUTO_TEST_CASE(result_read)
{  
    Collector collect;
    Pathname file;

    // this testcase represents this search:
    // http://api.opensuse-community.org/searchservice/Search/Simple/openSUSE_103/kopete

    file = DATADIR + "/search-kopete.xml";
    
    WebpinResultFileReader reader( file, bind( &Collector::callback, &collect, _1));
    BOOST_CHECK_EQUAL( collect.items.size(), 17);
    
    WebpinResult first = collect.items[0];
    BOOST_CHECK_EQUAL( first.name(), "kopete-otr");
    BOOST_CHECK_EQUAL( first.edition(), "0.6");
    BOOST_CHECK_EQUAL( first.repositoryUrl(), Url("http://download.opensuse.org/repositories/home:/burnickl_andreas/openSUSE_10.3"));
    BOOST_CHECK_EQUAL( first.distribution(), "openSUSE_103");
    BOOST_CHECK_EQUAL( first.checksum(), CheckSum::sha1("2a4d9e95f87abe16c28e4aefa0b3a0ae52220429"));
    BOOST_CHECK_EQUAL( first.priority(), 0);
    BOOST_CHECK_EQUAL( first.summary(), "OTR Plugin for Kopete");
}


// vim: set ts=2 sts=2 sw=2 ai et:
