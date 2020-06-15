#include <boost/test/unit_test.hpp>
#include <zypp/zyppng/io/private/iobuffer_p.h>
#include <algorithm>

BOOST_AUTO_TEST_CASE(iobuf)
{
  zyppng::IOBuffer buf ( 10 );

  zyppng::ByteArray t("Hello World");

  buf.append( t );
  buf.append( zyppng::ByteArray(" it's a great day!") );


  zyppng::ByteArray rbuf( 30, '\0' );

  BOOST_REQUIRE_EQUAL( buf.size(), 29);
  BOOST_REQUIRE_EQUAL( buf.chunks(), 2);

  buf.read( rbuf.data(), 5 );
  BOOST_REQUIRE_EQUAL( std::string_view(rbuf.data()), "Hello" );
  BOOST_REQUIRE_EQUAL( buf.size(), 24);
  BOOST_REQUIRE_EQUAL( buf.chunks(), 2);

  buf.read( rbuf.data()+5, 6);
  BOOST_REQUIRE_EQUAL( std::string_view(rbuf.data()), "Hello World" );
  BOOST_REQUIRE_EQUAL( buf.size(), 18);
  BOOST_REQUIRE_EQUAL( buf.chunks(), 1);

  buf.read( rbuf.data()+11, 18 );
  BOOST_REQUIRE_EQUAL( std::string_view(rbuf.data()), "Hello World it's a great day!" );
  BOOST_REQUIRE_EQUAL( buf.size(), 0);
  BOOST_REQUIRE_EQUAL( buf.chunks(), 0);
}


BOOST_AUTO_TEST_CASE(iobuf_reserve_and_discard)
{
  zyppng::IOBuffer buf;
  buf.reserve( 1234 );
  BOOST_REQUIRE_EQUAL( buf.size(), 1234);
  BOOST_REQUIRE_EQUAL( buf.chunks(), 1);
  buf.discard( 1234 );
  BOOST_REQUIRE_EQUAL( buf.size(), 0);
  BOOST_REQUIRE_EQUAL( buf.chunks(), 0);
}

BOOST_AUTO_TEST_CASE(iobuf_readtoomuch)
{
  zyppng::IOBuffer buf;
  buf.append( zyppng::ByteArray("Short Text") );
  BOOST_REQUIRE_EQUAL( buf.size(), 10);

  zyppng::ByteArray rbuf( 15, '\0' );
  const auto read = buf.read( rbuf.data(), rbuf.size() );
  BOOST_REQUIRE_EQUAL( read, 10 );
  BOOST_REQUIRE_EQUAL( std::string_view(rbuf.data()), "Short Text" );
  BOOST_REQUIRE_EQUAL( buf.size(), 0);
}

BOOST_AUTO_TEST_CASE(iobuf_reserve_small)
{
  zyppng::IOBuffer buf( 20 );
  buf.append( zyppng::ByteArray("1234567890") );
  buf.append( zyppng::ByteArray("1234567890") );
  BOOST_REQUIRE_EQUAL( buf.chunks(), 1);
  buf.append( zyppng::ByteArray("1234567890") );
  BOOST_REQUIRE_EQUAL( buf.chunks(), 2);
}


BOOST_AUTO_TEST_CASE(iobuf_indexof)
{
  zyppng::IOBuffer buf(10);
                               /*|-Chunk1-|*/
  buf.append( zyppng::ByteArray("1234567890") );
                                /*|--Chunk2-|*/
  buf.append( zyppng::ByteArray("\n23456\n89a") );
                               /*|-Chunk3-|*/
  buf.append( zyppng::ByteArray("9876543210") );

  BOOST_REQUIRE_EQUAL( buf.chunks(), 3);
  BOOST_REQUIRE_EQUAL( buf.indexOf('\n'), 10 );
  BOOST_REQUIRE_EQUAL( buf.indexOf('\n', buf.size(), 10), 10 );
  BOOST_REQUIRE_EQUAL( buf.indexOf('\n', buf.size(), 11), 16 );
  BOOST_REQUIRE_EQUAL( buf.indexOf('\n', buf.size(), 17), -1 );
  BOOST_REQUIRE_EQUAL( buf.indexOf('a', buf.size(), 12), 19 );
}

//shamelessly adapted from Qt
BOOST_AUTO_TEST_CASE(iobuf_indexof_progression)
{
  zyppng::IOBuffer buf(16);
  for ( int i = 1; i <= 256; i++ ) {
    buf.append( zyppng::ByteArray({char(i)}));
  }

  for (int i = 16; i < 256; ++i) {
    auto index = buf.indexOf(char(i));
    BOOST_REQUIRE_EQUAL(index, (i - 1));
    BOOST_REQUIRE_EQUAL(buf.indexOf(char(i), i, i >> 1), index);
    BOOST_REQUIRE_EQUAL(buf.indexOf(char(i), 256, i), -1);
    BOOST_REQUIRE_EQUAL(buf.indexOf(char(i), i - 1), -1); // test for absent char
  }
}

BOOST_AUTO_TEST_CASE(reserveAndRead)
{
  zyppng::IOBuffer buf;
  // fill buffer with an arithmetic progression
  for (int i = 1; i < 256; ++i) {
    zyppng::ByteArray ba(i, char(i));
    char *ringPos = buf.reserve(i);
    BOOST_REQUIRE(ringPos);
    memcpy(ringPos, ba.data(), i);
  }
  // readback and check stored data
  for (int i = 1; i < 256; ++i) {
    zyppng::ByteArray ba;
    ba.resize(i);
    int64_t thisRead = buf.read(ba.data(), i);
    BOOST_REQUIRE_EQUAL(thisRead, int64_t(i));
    BOOST_REQUIRE_EQUAL(std::count( ba.begin(), ba.end(), char(i)), i);
  }
  BOOST_REQUIRE_EQUAL(buf.size(), 0);
}

BOOST_AUTO_TEST_CASE(readline)
{
  zyppng::IOBuffer buf(10);

  zyppng::ByteArray b1("Hello World\n");
  buf.append( b1 );
  zyppng::ByteArray b2("Another\n");
  buf.append( b2 );
  zyppng::ByteArray b3("1234\n");
  buf.append( b3 );
  zyppng::ByteArray b4("6789\n");
  buf.append( b4 );

  BOOST_REQUIRE( buf.canReadLine() );

  auto r1 = buf.readLine( 5 );
  BOOST_REQUIRE_EQUAL( r1.size(), 0 );
  r1 = buf.readLine();
  BOOST_REQUIRE_EQUAL( std::string_view( r1.data(), r1.size() ), std::string_view(b1.data(), b1.size()) );
  r1 = buf.readLine();
  BOOST_REQUIRE_EQUAL( std::string_view( r1.data(), r1.size() ), std::string_view(b2.data(), b2.size()) );
  r1 = buf.readLine();
  BOOST_REQUIRE_EQUAL( std::string_view( r1.data(), r1.size() ), std::string_view(b3.data(), b3.size()) );
  r1 = buf.readLine( 5 );
  BOOST_REQUIRE_EQUAL( std::string_view( r1.data(), r1.size() ), std::string_view(b4.data(), b4.size()) );

}

BOOST_AUTO_TEST_CASE(move)
{
  zyppng::IOBuffer buf(10);

  zyppng::ByteArray b1("Hello World\n");
  buf.append( b1 );
  zyppng::ByteArray b2("Another\n");
  buf.append( b2 );
  zyppng::ByteArray b3("1234\n");
  buf.append( b3 );
  zyppng::ByteArray b4("6789\n");
  buf.append( b4 );

  const auto size = buf.size();
  zyppng::IOBuffer buf2( std::move( buf ) );

  BOOST_REQUIRE_EQUAL( size, buf2.size() );

}
