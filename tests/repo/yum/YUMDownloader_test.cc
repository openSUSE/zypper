#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/Logger.h"
#include "zypp/Url.h"
#include "zypp/PathInfo.h"
#include "zypp/TmpPath.h"
#include "zypp/repo/yum/Downloader.h"

using std::cout;
using std::endl;
using std::string;
using namespace zypp;
using namespace boost::unit_test;
using namespace zypp::repo;

#include "tests/zypp/KeyRingTestReceiver.h"

#define DATADIR (Pathname(TESTS_SRC_DIR) + "/repo/yum/data")

BOOST_AUTO_TEST_CASE(yum_download)
{
  KeyRingTestReceiver keyring_callbacks;
  keyring_callbacks.answerAcceptKey(KeyRingReport::KEY_TRUST_TEMPORARILY);

  Pathname p = DATADIR + "/10.2-updates-subset";
  Url url(p.asDirUrl());
  MediaSetAccess media(url);
  RepoInfo repoinfo;
  repoinfo.setAlias("testrepo");
  repoinfo.setPath("/");
  yum::Downloader yum(repoinfo);
  filesystem::TmpDir tmp;

  Pathname localdir(tmp.path());

  yum.download(media, localdir);

  const char* files[] =
  {
//    "filelists.xml.gz",
//    "other.xml.gz",
    "patches.xml",
    "patch-fetchmsttfonts.sh-2333.xml",
    "patch-flash-player-2359.xml",
    "patch-glabels-2348.xml",
    "patch-gv-2350.xml",
    "patch-openssl-2349.xml",
    "patch-tar-2351.xml",
    "primary.xml.gz",
    "repomd.xml",
    "repomd.xml.asc",
    "repomd.xml.key",
    NULL
  };

  int i=0;
  while ( files[i] != NULL )
  {
    BOOST_CHECK_MESSAGE( PathInfo(localdir + "/repodata/" + files[i] ).isExist(), (string("/repodata/") + files[i]).c_str() );
    i++;
  }

}

// vim: set ts=2 sts=2 sw=2 ai et:
