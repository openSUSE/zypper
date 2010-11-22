#include <iostream>
#include <vector>
#include <boost/test/auto_unit_test.hpp>

#include "zypp/media/MetaLinkParser.h"

using namespace std;
using namespace zypp;
using namespace zypp::media;

BOOST_AUTO_TEST_CASE(parse_metalink)
{
  Pathname meta3file = TESTS_SRC_DIR "/media/data/openSUSE-11.3-NET-i586.iso.metalink";
  Pathname meta4file = TESTS_SRC_DIR "/media/data/openSUSE-11.3-NET-i586.iso.meta4";

	MetaLinkParser mlp3;
	MetaLinkParser mlp4;

  mlp3.parse(meta3file);
  MediaBlockList bl3 = mlp3.getBlockList();
  vector<Url> urls3 = mlp3.getUrls();

  mlp4.parse(meta4file);
  MediaBlockList bl4 = mlp4.getBlockList();
  vector<Url> urls4 = mlp4.getUrls();


  BOOST_CHECK(bl3.asString() == bl4.asString());

  BOOST_CHECK(urls3.size() == 94);
  BOOST_CHECK(urls4.size() == 94);

  BOOST_CHECK(urls3.begin()->asString() == "http://ftp.uni-kl.de/pub/linux/opensuse/distribution/11.3/iso/openSUSE-11.3-NET-i586.iso");
  BOOST_CHECK(urls4.begin()->asString() == "http://ftp4.gwdg.de/pub/opensuse/distribution/11.3/iso/openSUSE-11.3-NET-i586.iso");

  BOOST_CHECK(bl3.getFilesize() == 120285184);
  BOOST_CHECK(bl4.getFilesize() == 120285184);

  BOOST_CHECK(bl3.numBlocks() == 459);
  BOOST_CHECK(bl4.numBlocks() == 459);
}
