#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include "zypp/base/Random.h"
#include "zypp/base/Logger.h"
#include "zypp/base/GzStream.h"
#include "zypp/base/Measure.h"
#include "zypp/capability/CapabilityImpl.h"
#include "zypp/data/ResolvableData.h"
#include "zypp2/cache/CacheStore.h"
#include "zypp/Url.h"
#include "zypp/NVRA.h"
#include "zypp/PathInfo.h"
#include "zypp/TmpPath.h"

using namespace std;
using namespace zypp;
using namespace boost::unit_test;

void read_dash( zypp::ifgzstream &ifs, const std::string &s, int &line )
{
  string buffer;
  // get the "-"
  getline(ifs, buffer);
  line++;
  if ( buffer != s )
  {
    ERR << "line : " << line << endl;
    ZYPP_THROW(Exception("missing " + s ));
  }
}

void read_deps( zypp::ifgzstream &ifs, data::DependencyList &list, int &line, const string &endchar )
{
  string buffer;
  while ( ifs && !ifs.eof())
  {
    getline(ifs, buffer);
    line++;
    if ( buffer == endchar )
      break;
    try
    {
      capability::CapabilityImpl::Ptr cap = zypp::capability::parse( ResTraits<Package>::kind, buffer);
      if (cap)
        list.push_back(cap);
    }
    catch( const Exception  &e )
    {
      ERR << "line : " << line << endl;
      ZYPP_THROW(Exception("bad capability line")); 
    }    
  }
}

struct MiniResolvable
{
  NVRA nvra;
  data::Dependencies deps;
};

void cache_write_test(const string &dir)
{
  list<MiniResolvable> res_list;
  std::string buffer;
  Pathname nvra_list = Pathname(dir) + "package-set.txt.gz";
  
  int line = 0;
  zypp::ifgzstream nvra_stream(nvra_list.c_str());
  MIL << "reading " << nvra_list << endl;
  
  if ( ! nvra_stream )
    ZYPP_THROW(Exception("cant open data file " + nvra_list.asString()));
  
  while ( nvra_stream && !nvra_stream.eof())
  {
    MiniResolvable res;
    getline(nvra_stream, buffer);
    line++;
    
    if ( buffer.empty() )
      break;
    
    std::vector<std::string> words;
    if ( str::split( buffer, std::back_inserter(words) ) != 4 )
    {
      ERR << nvra_list << " : line : " << line << endl;
      ZYPP_THROW(Exception("bad NVRA line"));
    }
    
    res.nvra = NVRA(words[0], Edition(words[1], words[2]), Arch(words[3]));
    // requires
    read_dash( nvra_stream, "+r", line);
    read_deps( nvra_stream, res.deps[Dep::REQUIRES], line, "-r");
    read_dash( nvra_stream, "+p", line);
    read_deps( nvra_stream, res.deps[Dep::PROVIDES], line, "-p");
    
    res_list.push_back(res);
  }
  //MIL << deps.size() << " capabilities read." << endl;
  
  filesystem::TmpDir tmpdir;
  cache::CacheStore store(tmpdir.path());
  
  data::RecordId catalog_id = store.lookupOrAppendCatalog( Url("http://novell.com"), "/");
  
  zypp::debug::Measure cap_parse_timer("store resolvables");
  for ( list<MiniResolvable>::iterator it = res_list.begin(); it != res_list.end(); it++)
  {
    data::RecordId id = store.appendResolvable( catalog_id,
                                       ResTraits<Package>::kind,
                                       (*it).nvra,
                                       (*it).deps );
  }
}

test_suite*
init_unit_test_suite( int argc, char *argv[] )
{
  string datadir;
  if (argc < 2)
  {
    datadir = TESTS_SRC_DIR;
    datadir = (Pathname(datadir) + "/cache/data").asString();
    cout << "CacheStore_test:"
      " path to directory with test data required as parameter. Using " << datadir  << endl;
    //return (test_suite *)0;
    
  }
  else
  {
    datadir = argv[1];
  }
  
  test_suite* test= BOOST_TEST_SUITE("CacheStore");
  
  std::string const params[] = { datadir };
  test->add(BOOST_PARAM_TEST_CASE(&cache_write_test,
                                 (std::string const*)params, params+1));
  return test;
}


// vim: set ts=2 sts=2 sw=2 ai et:
