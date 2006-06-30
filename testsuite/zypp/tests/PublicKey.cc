
#include <iostream>
#include <list>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/PublicKey.h"
#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"
#include "testsuite/src/utils/TestUtils.h"

using namespace std;
using namespace zypp;
using namespace zypp::testsuite::utils;

int main( int argc, char * argv[] )
{
  try
  {
    zypp::devel::PublicKey k1("publickey-1.asc");
  
    assert_equal( k1.id(), "CD1EB6A9667E42D1");
    assert_equal( k1.name(), "Duncan Mac-Vicar Prett <duncan@puc.cl>" );
    assert_equal( k1.fingerprint(), "75DA7B971FD6ADB9A880BA9FCD1EB6A9667E42D1" );
  
    zypp::devel::PublicKey k2("publickey-suse.asc");
  
    assert_equal( k2.id(), "A84EDAE89C800ACA" );
    assert_equal( k2.name(), "SuSE Package Signing Key <build@suse.de>" );
    assert_equal( k2.fingerprint(), "79C179B2E1C820C1890F9994A84EDAE89C800ACA" );
    
    return 0;
  }
  catch(const Exception &e)
  {
    return 40;
  }  
  
  return 0;
}
