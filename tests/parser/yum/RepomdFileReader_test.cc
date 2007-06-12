#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include "zypp/parser/yum/RepomdFileReader.h"
#include "zypp/Url.h"
#include "zypp/PathInfo.h"

using namespace std;
using namespace zypp;
using namespace boost::unit_test;

using namespace zypp::parser::yum;
using source::yum::YUMResourceType;

class Collector
{
public:
  Collector()
  {}
  
  bool callback( const OnMediaLocation &loc, const YUMResourceType &t )
  {
    items.push_back( make_pair( t, loc ) );
    //items.push_back(loc);
    //cout << items.size() << endl;
    return true;
  }
  
  vector<pair<YUMResourceType, OnMediaLocation> > items;
  //vector<OnMediaLocation> items;
};

void repomd_read_test(const string &dir)
{
  list<Pathname> entries;
  if ( filesystem::readdir( entries, Pathname(dir), false ) != 0 )
    ZYPP_THROW(Exception("failed to read directory"));
    
  for ( list<Pathname>::const_iterator it = entries.begin(); it != entries.end(); ++it )
  {
    Pathname file = *it;
    if ( ( file.basename().substr(0, 6) == "repomd" ) && (file.extension() == ".xml" ) )
    {
      cout << *it << endl;
      
      Collector collect;
      RepomdFileReader( file, bind( &Collector::callback, &collect, _1, _2 ));
      
      std::ifstream ifs( file.extend(".solution").asString().c_str() );
      
      unsigned int count = 0;
      while ( ifs && ! ifs.eof() && count < collect.items.size() )
      {
        string dtype;
        string checksum_type;
        string checksum;
        string loc;
        
        getline(ifs, dtype);
        BOOST_CHECK_EQUAL( collect.items[count].first, YUMResourceType(dtype));
        getline(ifs, checksum_type);
        getline(ifs, checksum);
        BOOST_CHECK_EQUAL( collect.items[count].second.checksum(), CheckSum(checksum_type, checksum) );
        getline(ifs, loc);
        BOOST_CHECK_EQUAL( collect.items[count].second.filename(), Pathname(loc) );
        
        count++;
      }
      BOOST_CHECK_EQUAL( collect.items.size(), count );
    }
  }
}

test_suite*
init_unit_test_suite( int argc, char *argv[] )
{
  if (argc < 2)
  {
    cout << "RepomdFileReader_test:"
      " path to directory with test data required as parameter" << endl;
    return (test_suite *)0;
  }
  
  test_suite* test= BOOST_TEST_SUITE("RepomdFileReader");
  string datadir = argv[1];
  std::string const params[] = { datadir };
  test->add(BOOST_PARAM_TEST_CASE(&repomd_read_test,
                                 (std::string const*)params, params+1));
  return test;
}

// vim: set ts=2 sts=2 sw=2 ai et:
