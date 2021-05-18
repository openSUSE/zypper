#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <boost/test/unit_test.hpp>

#include <zypp/parser/yum/RepomdFileReader.h>
#include <zypp/Url.h>
#include <zypp/PathInfo.h>

using std::cout;
using std::endl;
using namespace zypp;
using namespace boost::unit_test;

using namespace zypp::parser::yum;

#define DATADIR (Pathname(TESTS_SRC_DIR) + "parser/yum/data")

class Collector
{
public:
  Collector()
  {}

  bool operator()( OnMediaLocation &&loc, const std::string &t )
  {
    items.push_back( make_pair( t, std::move(loc) ) );
    return true;
  }
  
  std::vector<std::pair<std::string, OnMediaLocation> > items;
};

BOOST_AUTO_TEST_CASE(repomd_read)
{
  std::list<Pathname> entries;
  if ( filesystem::readdir( entries, DATADIR, false ) != 0 )
    ZYPP_THROW(Exception("failed to read directory"));
    
  for ( std::list<Pathname>::const_iterator it = entries.begin(); it != entries.end(); ++it )
  {
    Pathname file = *it;
    if ( ( file.basename().substr(0, 6) == "repomd" ) && (file.extension() == ".xml" ) )
    {
      cout << *it << endl;

      Collector collect;
      RepomdFileReader r( file, std::ref(collect) );
      if ( file.basename() == "repomd-1.xml" ) {
	BOOST_CHECK_EQUAL( r.keywords().size(), 7 );
	auto keyhints = r.keyhints();
	BOOST_CHECK_EQUAL( keyhints.size(), 5 );
	std::map<std::string,std::string> check = {
	  { "gpg-pubkey-39db7c82-5847eb1f.asc", "FEAB502539D846DB2C0961CA70AF9E8139DB7C82" },
	  { "gpg-pubkey-307e3d54-5aaa90a5.asc", "4E98E67519D98DC7362A" },
	  { "gpg-pubkey-65176565-59787af5.asc", "637B32FF" },
	  { "gpg-pubkey-3dbdc284-is OK",        "3dbdc284" },
	  { "gpg-pubkey-feab502539d846db2c0961ca70af9e8139db7c82-is OK as well", "feab502539d846db2c0961ca70af9e8139db7c82" },
	  { "gpg-pubkey-536X4dd4-X is not a hexdigit", "must not occur" },
	};
	for ( const auto & hint : keyhints ) {
	  BOOST_CHECK_EQUAL( check[hint.first], hint.second );
	}
      }

      std::ifstream ifs( file.extend(".solution").asString().c_str() );

      unsigned int count = 0;
      while ( ifs && !ifs.eof() )
      {
        std::string dtype;
	getline(ifs, dtype);
	if ( dtype.empty() )
	  break;
	BOOST_REQUIRE( count < collect.items.size() );
        BOOST_CHECK_EQUAL( collect.items[count].first, dtype );

        std::string checksum_type;
        std::string checksum;
        getline(ifs, checksum_type);
        getline(ifs, checksum);
        BOOST_CHECK_EQUAL( collect.items[count].second.checksum(), CheckSum(checksum_type, checksum) );

	std::string loc;
        getline(ifs, loc);
        BOOST_CHECK_EQUAL( collect.items[count].second.filename(), Pathname(loc) );

        count++;
      }
      BOOST_CHECK_EQUAL( collect.items.size(), count );
    }
  }
}


// vim: set ts=2 sts=2 sw=2 ai et:
