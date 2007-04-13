#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include "zypp/parser/yum/PatchesFileReader.h"
#include "zypp/Url.h"
#include "zypp/PathInfo.h"

using std::cout;
using std::endl;
using std::string;
using namespace zypp;
using namespace boost::unit_test;

using namespace zypp::source::yum;

class Collector
{
public:
  Collector()
  {}
  
  bool callback( const OnMediaLocation &loc, const string &id )
  {
    items.push_back( make_pair( id, loc ) );
    //items.push_back(loc);
    //cout << items.size() << endl;
    return true;
  }
  
  vector<pair<string, OnMediaLocation> > items;
  //vector<OnMediaLocation> items;
};

void patches_read_test(const string &dir)
{
  list<Pathname> entries;
  if ( filesystem::readdir( entries, Pathname(dir), false ) != 0 )
    ZYPP_THROW(Exception("failed to read directory"));
  
  for ( list<Pathname>::const_iterator it = entries.begin(); it != entries.end(); ++it )
  {
    Pathname file = *it;
    //cout << file.basename().substr(0, 7) << " " << file.extension() << endl;
    if ( ( file.basename().substr(0, 7) == "patches" ) && (file.extension() == ".xml" ) )
    {
      //cout << *it << endl;
      
      Collector collect;
      PatchesFileReader( file, bind( &Collector::callback, &collect, _1, _2 ));
      
      std::ifstream ifs( file.extend(".solution").asString().c_str() );
      cout << "Comparing to " << file.extend(".solution") << endl;
      int count = 0;
      while ( ifs && ! ifs.eof() && count < collect.items.size() )
      {
        string id;
        string checksum_type;
        string checksum;
        string loc;
        
        getline(ifs, id);
        BOOST_CHECK_EQUAL( collect.items[count].first, id);
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
  string datadir;
  if (argc < 2)
  {
    datadir = TESTS_SRC_DIR;
    datadir = (Pathname(datadir) + "/parser/yum/data").asString();
    cout << "PatchesFileReader_test:"
      " path to directory with test data required as parameter. Using " << datadir  << endl;
    //return (test_suite *)0;
    
  }
  else
  {
    datadir = argv[1];
  }
  
  test_suite* test= BOOST_TEST_SUITE("PatchesFileReader");
  
  std::string const params[] = { datadir };
  test->add(BOOST_PARAM_TEST_CASE(&patches_read_test,
                                 (std::string const*)params, params+1));
  return test;
}

// vim: set ts=2 sts=2 sw=2 ai et:
