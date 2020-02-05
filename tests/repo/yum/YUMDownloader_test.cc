#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <boost/test/unit_test.hpp>
#include <solv/solvversion.h>

#include "zypp/base/Logger.h"
#include "zypp/Url.h"
#include "zypp/PathInfo.h"
#include "zypp/TmpPath.h"
#include "zypp/repo/yum/Downloader.h"
#include "tests/zypp/KeyRingTestReceiver.h"

using std::cout;
using std::endl;
using namespace zypp;
using namespace boost::unit_test;

#define DATADIR (Pathname(TESTS_SRC_DIR) + "/repo/yum/data")

BOOST_AUTO_TEST_CASE(yum_download)
{
  KeyRingTestReceiver keyring_callbacks;
  keyring_callbacks.answerAcceptKey(KeyRingReport::KEY_TRUST_TEMPORARILY);

  Pathname p = DATADIR + "/ZCHUNK";
  Url url(p.asDirUrl());
  MediaSetAccess media(url);
  RepoInfo repoinfo;
  repoinfo.setAlias("testrepo");
  repoinfo.setPath("/");
  repo::yum::Downloader yum(repoinfo);
  filesystem::TmpDir tmp;

  Pathname localdir(tmp.path());

  yum.download(media, localdir);

#if ENABLE_ZCHUNK_COMPRESSION && defined(LIBSOLVEXT_FEATURE_ZCHUNK_COMPRESSION)
    const bool zchunk = true;
#else
    const bool zchunk = false;
#endif
  std::map<std::string,bool> files {
    { "filelists.xml.gz",	false&&!zchunk },
    { "filelists.xml.zck",	false&&zchunk },
    { "other.xml.gz",		false&&!zchunk },
    { "other.xml.zck",		false&&zchunk },
    { "patterns.xml.gz",	true },
    { "primary.xml.gz",		!zchunk },
    { "primary.xml.zck",	zchunk },
    { "repomd.xml",		true },
    { "repomd.xml.asc",		true },
    { "repomd.xml.key",		true },
  };

  for ( const auto & el : files )
  {
    Pathname stem { "/repodata/"+el.first };
    bool downloaded { PathInfo(localdir/stem).isExist() };
    BOOST_CHECK_MESSAGE( downloaded == el.second, std::string(el.second?"missing ":"unexpected ")+stem );
  }

}

// vim: set ts=2 sts=2 sw=2 ai et:
