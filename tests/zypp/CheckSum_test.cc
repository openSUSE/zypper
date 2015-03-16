#include <iostream>
#include <list>
#include <string>

// Boost.Test
#include <boost/test/auto_unit_test.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/ZYppFactory.h"
#include "zypp/Digest.h"
#include "zypp/ZYpp.h"


using boost::unit_test::test_case;
using namespace std;
using namespace zypp;

void chksumtest( const std::string & type_r, const std::string & sum_r )
{
  BOOST_CHECK_EQUAL( type_r, CheckSum( sum_r ).type() );	// autodetect type
  BOOST_CHECK_EQUAL( type_r, CheckSum( type_r, sum_r ).type() );
  BOOST_CHECK_EQUAL( sum_r, Digest::digest( type_r, "" ) );
  for ( const std::string & t : { "md5", "sha1", "sha224", "sha256", "sha384", "sha512", } )
  {
    if ( t != type_r )
    {
      BOOST_CHECK_THROW( CheckSum( t, sum_r ), Exception ); // wrong type/size
    }
  }
}

// most frequently you implement test cases as a free functions
BOOST_AUTO_TEST_CASE(checksum_test)
{
  CheckSum e;
  BOOST_CHECK( e.empty() );
  BOOST_CHECK( e.type().empty() );
  BOOST_CHECK( e.checksum().empty() );
  //   sum for ""
  //   md5  32 d41d8cd98f00b204e9800998ecf8427e
  //     1  40 da39a3ee5e6b4b0d3255bfef95601890afd80709
  //   224  56 d14a028c2a3a2bc9476102bb288234c415a2b01f828ea62ac5b3e42f
  //   256  64 e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
  //   384  96 38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b
  //   512 128 cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e
  chksumtest( CheckSum::md5Type(),	"d41d8cd98f00b204e9800998ecf8427e" );
  chksumtest( CheckSum::sha1Type(),	"da39a3ee5e6b4b0d3255bfef95601890afd80709" );
  chksumtest( CheckSum::sha224Type(),	"d14a028c2a3a2bc9476102bb288234c415a2b01f828ea62ac5b3e42f" );
  chksumtest( CheckSum::sha256Type(),	"e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855" );
  chksumtest( CheckSum::sha384Type(),	"38b060a751ac96384cd9327eb1b1e36a21fdb71114be07434c0cc7bf63f6e1da274edebfe76f65fbd51ad2f14898b95b" );
  chksumtest( CheckSum::sha512Type(),	"cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e" );
}
