#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <boost/test/auto_unit_test.hpp>

#include "zypp/parser/yum/RepomdFileReader.h"
#include "zypp/Url.h"
#include "zypp/PathInfo.h"

using namespace std;
using namespace zypp;
using namespace boost::unit_test;

using namespace zypp::parser::yum;
using repo::yum::ResourceType;

#define DATADIR (Pathname(TESTS_SRC_DIR) + "parser/yum/data")

class Collector
{
public:
  Collector()
  {}

  bool operator()( OnMediaLocation &&loc, const ResourceType &t, const std::string & )
  {
    items.push_back( make_pair( t, std::move(loc) ) );
    return true;
  }
  
  vector<pair<ResourceType, OnMediaLocation> > items;
};

BOOST_AUTO_TEST_CASE(repomd_read)
{
  list<Pathname> entries;
  if ( filesystem::readdir( entries, DATADIR, false ) != 0 )
    ZYPP_THROW(Exception("failed to read directory"));
    
  for ( list<Pathname>::const_iterator it = entries.begin(); it != entries.end(); ++it )
  {
    Pathname file = *it;
    if ( ( file.basename().substr(0, 6) == "repomd" ) && (file.extension() == ".xml" ) )
    {
      cout << *it << endl;

      Collector collect;
      RepomdFileReader( file, std::ref(collect) );

      std::ifstream ifs( file.extend(".solution").asString().c_str() );

      unsigned int count = 0;
      while ( ifs && !ifs.eof() )
      {
        string dtype;
	getline(ifs, dtype);
	if ( dtype.empty() )
	  break;
	BOOST_REQUIRE( count < collect.items.size() );
        BOOST_CHECK_EQUAL( collect.items[count].first, ResourceType(dtype));

        string checksum_type;
        string checksum;
        getline(ifs, checksum_type);
        getline(ifs, checksum);
        BOOST_CHECK_EQUAL( collect.items[count].second.checksum(), CheckSum(checksum_type, checksum) );

	string loc;
        getline(ifs, loc);
        BOOST_CHECK_EQUAL( collect.items[count].second.filename(), Pathname(loc) );

        count++;
      }
      BOOST_CHECK_EQUAL( collect.items.size(), count );
    }
  }
}


// vim: set ts=2 sts=2 sw=2 ai et:
