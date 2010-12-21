#include <sstream>
#include <string>
#include <zypp/parser/RepoFileReader.h>
#include <zypp/base/NonCopyable.h>

#include "TestSetup.h"

using std::stringstream;
using std::string;
using namespace zypp;

static string suse_repo = "[factory-oss]\n"
"name=factory-oss\n"
"enabled=1\n"
"autorefresh=0\n"
"baseurl=http://download.opensuse.org/factory-tested/repo/oss/\n"
"type=yast2\n"
"keeppackages=0\n";

static string fedora_repo = "[fedora]\n"
"name=Fedora $releasever - $basearch\n"
"failovermethod=priority\n"
"#baseurl=http://download.fedora.redhat.com/pub/fedora/linux/releases/$releasever/Everything/$basearch/os/\n"
"#mirrorlist=http://mirrors.fedoraproject.org/mirrorlist?repo=fedora-$releasever&arch=$basearch\n"
"mirrorlist=file:///etc/yum.repos.d/local.mirror\n"
"enabled=1\n"
"gpgcheck=1\n"
"gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-fedora file:///etc/pki/rpm-gpg/RPM-GPG-KEY\n";

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
    stringstream input(suse_repo);
    RepoCollector collector;
    parser::RepoFileReader parser( input, bind( &RepoCollector::collect, &collector, _1 ) );
    BOOST_CHECK_EQUAL(1, collector.repos.size());
  }
  // fedora
  {
    stringstream input(fedora_repo);
    RepoCollector collector;
    parser::RepoFileReader parser( input, bind( &RepoCollector::collect, &collector, _1 ) );
    BOOST_REQUIRE_EQUAL(1, collector.repos.size());

    RepoInfo repo = *collector.repos.begin();
    // should have taken the first url if more are present
    BOOST_CHECK_EQUAL(Url("file:///etc/pki/rpm-gpg/RPM-GPG-KEY-fedora"), repo.gpgKeyUrl());
  }

}
