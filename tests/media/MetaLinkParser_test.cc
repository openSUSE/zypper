#include <iostream>
#include <vector>
#include <boost/test/unit_test.hpp>

#include <zypp/media/MetaLinkParser.h>

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
  std::vector<Url> urls3 = mlp3.getUrls();

  mlp4.parse(meta4file);
  MediaBlockList bl4 = mlp4.getBlockList();
  std::vector<Url> urls4 = mlp4.getUrls();


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

BOOST_AUTO_TEST_CASE(parse_metalink_with_zsync_hash)
{
   Pathname meta4file = TESTS_SRC_DIR "/media/data/other.xml.gz.meta4";
   MetaLinkParser mlp4;
   mlp4.parse( meta4file );

   auto bl = mlp4.getBlockList();
   BOOST_REQUIRE( bl.haveBlocks() );
   BOOST_REQUIRE_EQUAL( bl.numBlocks(), 246 );

   const auto &rsums = mlp4.getZsyncBlockHashes();
   const auto &csums = mlp4.getSHA1BlockHashes();
   BOOST_REQUIRE_EQUAL( rsums.size(), csums.size() );
   BOOST_REQUIRE_EQUAL( bl.numBlocks(), csums.size() );
   BOOST_REQUIRE_EQUAL( bl.getFilesize(), 16108851 );

   BOOST_REQUIRE( bl.getFileChecksum() == hexstr2bytes("702d2a63e32b11a60ef853247f7901a71d0ec12731003a433dc17d200021a121") );
   BOOST_REQUIRE( rsums.front() == hexstr2bytes("52c0a614") );
   BOOST_REQUIRE( rsums.back() == hexstr2bytes("349ad053") );
   BOOST_REQUIRE( csums.front() == hexstr2bytes("b9f77f48f4a49f3438fc1bb6de90df39657597aa") );
   BOOST_REQUIRE( csums.back() == hexstr2bytes("23788c5daf28813cdc6a14f1f02add712c020fd9") );

   const auto &mirrors = mlp4.getMirrors();
   BOOST_REQUIRE_EQUAL( mirrors.size(), 103 );

   const auto &firstMirr = mirrors.front();
   BOOST_REQUIRE_EQUAL( firstMirr.url, zypp::Url("http://ftp.uni-erlangen.de/pub/mirrors/opensuse/update/leap/15.0/oss/repodata/702d2a63e32b11a60ef853247f7901a71d0ec12731003a433dc17d200021a121-other.xml.gz"));
   BOOST_REQUIRE_EQUAL( firstMirr.priority, 1 );
   BOOST_REQUIRE_EQUAL( firstMirr.maxConnections, 10 );

   const auto &lastMirr = mirrors.back();
   BOOST_REQUIRE_EQUAL( lastMirr.url, zypp::Url("http://mirror.internode.on.net/pub/opensuse/update/leap/15.0/oss/repodata/702d2a63e32b11a60ef853247f7901a71d0ec12731003a433dc17d200021a121-other.xml.gz"));
   BOOST_REQUIRE_EQUAL( lastMirr.priority, 103 );
   BOOST_REQUIRE_EQUAL( lastMirr.maxConnections, -1 );
}
