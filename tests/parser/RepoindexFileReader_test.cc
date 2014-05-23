#include <sstream>
#include <string>
#include <zypp/Pathname.h>
#include <zypp/parser/RepoindexFileReader.h>
#include <zypp/base/NonCopyable.h>

#include "TestSetup.h"

using std::stringstream;
using std::string;
using namespace zypp;

static string service = "<repoindex arch=\"i386\" distver=\"11\">"
  "<repo alias=\"company-foo\" name=\"Company's Foo\""
  "      path=\"products/foo\" distro_target=\"sle-%{distver}-%{arch}\" priority=\"20\"/>"
  "<repo alias=\"company-bar\" name=\"Company's Bar\""
  "      path=\"products/bar\" distro_target=\"sle-%{distver}-%{arch}\" enabled=\"tRUe\" autorefresh=\"FaLsE\"/>"
  "<repo alias=\"company-foo-upd\" name=\"Company's Foo Updates\""
  "      path=\"products/foo/updates\" distro_target=\"sle-%{distver}-%{arch}\" priority=\"1\"/>"
  "</repoindex>";

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
BOOST_AUTO_TEST_CASE(read_index_file)
{
  {
    stringstream input(service);
    RepoCollector collector;
    parser::RepoindexFileReader parser( input, bind( &RepoCollector::collect, &collector, _1 ) );
    BOOST_REQUIRE_EQUAL(3, collector.repos.size());

    RepoInfo repo;
    repo = collector.repos.front();

    BOOST_CHECK_EQUAL("Company's Foo", repo.name());
    BOOST_CHECK_EQUAL("company-foo", repo.alias());
    BOOST_CHECK_EQUAL("sle-11-i386", repo.targetDistribution());
    BOOST_CHECK_EQUAL(20, repo.priority());
    // "Repository is per default disabled"
    BOOST_CHECK(!repo.enabled());
    // "Repository autorefresh is per default enabled"
    BOOST_CHECK(repo.autorefresh());
    BOOST_CHECK_EQUAL("/repo/products/foo", repo.path());

    collector.repos.pop_front( );
    repo = collector.repos.front();

    BOOST_CHECK_EQUAL("company-bar", repo.alias());
    BOOST_CHECK_EQUAL("sle-11-i386", repo.targetDistribution());
    // "Priority should be 99 when not explictly defined"
    BOOST_CHECK_EQUAL(99, repo.priority());
    // "Repository is explicitly enabled"
    BOOST_CHECK(repo.enabled());
    // "Repository autorefresh is explicitly disabled"
    BOOST_CHECK(!repo.autorefresh());


  }
}
