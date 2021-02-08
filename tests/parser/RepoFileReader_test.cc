#include <sstream>
#include <string>
#include <zypp/parser/RepoFileReader.h>
#include <zypp/base/NonCopyable.h>

#include "TestSetup.h"

using std::stringstream;
using std::string;
using namespace zypp;

static std::string suse_repo = "[factory-oss]\n"
"name=factory-oss $releasever - $basearch\n"
"failovermethod=priority\n"
"enabled=1\n"
"gpgcheck=1\n"
"autorefresh=0\n"
"baseurl=http://serv.er/loc1\n"
"baseurl=http://serv.er/loc2\n"
"        http://serv.er/loc3  ,   http://serv.er/loc4     http://serv.er/loc5\n"
"gpgkey=http://serv.er/loc1\n"
"gpgkey=http://serv.er/loc2\n"
"        http://serv.er/loc3  ,   http://serv.er/loc4     http://serv.er/loc5\n"
"mirrorlist=http://serv.er/loc1\n"
"mirrorlist=http://serv.er/loc2\n"
"        http://serv.er/loc3  ,   http://serv.er/loc4     http://serv.er/loc5\n"
"type=NONE\n"
"keeppackages=0\n";

struct RepoCollector : private base::NonCopyable
{
  bool collect( const RepoInfo &repo )
  {
    repos.push_back(repo);
    return true;
  }

  RepoInfoList repos;
};

// Must be the first test!
BOOST_AUTO_TEST_CASE(read_repo_file)
{
  {
    std::stringstream input(suse_repo);
    RepoCollector collector;
    parser::RepoFileReader parser( input, bind( &RepoCollector::collect, &collector, _1 ) );
    BOOST_CHECK_EQUAL(1, collector.repos.size());

    const RepoInfo & repo( collector.repos.front() );

    BOOST_CHECK_EQUAL( 5, repo.baseUrlsSize() );
    BOOST_CHECK_EQUAL( 5, repo.gpgKeyUrlsSize() );
    BOOST_CHECK_EQUAL( Url("http://serv.er/loc1"), repo.url() );
    BOOST_CHECK_EQUAL( Url("http://serv.er/loc1"), repo.gpgKeyUrl() );
    BOOST_CHECK_EQUAL( Url("http://serv.er/loc1"), repo.mirrorListUrl() );
  }
}
