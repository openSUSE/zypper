#include <iostream>
#include <fstream>

#include <zypp/base/Logger.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>

#include "zypp/Product.h"
#include "zypp/Package.h"
#include "zypp/PublicKey.h"
#include "zypp/detail/PackageImplIf.h"
#include "zypp/source/PackageDelta.h"
#include "zypp/parser/yum/YUMPrimaryParser.h"
#include "zypp/detail/ImplConnect.h"

#include "zypp/SourceFactory.h"
#include "testsuite/src/utils/TestUtils.h"
#include "zypp/base/GzStream.h"
#include "zypp/source/OnMediaLocation.h"

using namespace std;
using namespace zypp;
using namespace zypp::detail;
using namespace zypp::source;
using namespace zypp::packagedelta;


int main(int argc, char **argv)
{
  try
  {
    ZYpp::Ptr z = getZYpp();
   
    source::OnMediaLocation oml;
    oml.medianr( str::strtonum<unsigned>( "" ) )
                            .filename( "rpm/blah.rpm" )
                            .checksum( CheckSum( "sha1", "a7235c3ae45b353ceca8358160fe305d26b4d3dd" ) )
                            .downloadsize( str::strtonum<ByteCount::SizeType>( "2506999" ) );
    
    MIL << oml << endl;
  }
  catch ( const Exception &e )
  {
    std::cout << e.msg() << endl;
  }
}



