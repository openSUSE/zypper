#include <iostream>

#include <zypp/base/Logger.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>

#include "zypp/Product.h"
#include "zypp/Package.h"
#include "zypp/PublicKey.h"
#include "zypp/detail/PackageImplIf.h"
#include "zypp/source/PackageDelta.h"
#include "zypp/detail/ImplConnect.h"

#include "zypp/SourceFactory.h"
#include "testsuite/src/utils/TestUtils.h"

using namespace std;
using namespace zypp;
using namespace zypp::detail;
using namespace zypp::source;
using namespace zypp::packagedelta;
//using namespace DbXml;

int main()
{
  try
  {
    ZYpp::Ptr z = getZYpp();
    
    PublicKey key("repomd.xml.key");
    cout << key << endl;
        
  }
  catch ( const Exception &e )
  {
    std::cout << e.msg() << endl;
  }
}



