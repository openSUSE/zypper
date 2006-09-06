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

using namespace std;
using namespace zypp;
using namespace zypp::detail;
using namespace zypp::source;
using namespace zypp::packagedelta;
//using namespace DbXml;

struct YUMSourceEventHandler
{
  YUMSourceEventHandler()
  {}
        
  void operator()( int p )
  {
    std::cout << "\rProgress " << p << "               " << std::endl;
  }
};

int main(int argc, char **argv)
{
  try
  {
    ZYpp::Ptr z = getZYpp();
    
    //PublicKey key("repomd.xml.key");
    //cout << key << endl;
    //z->initializeTarget("/");
    
    Pathname filename = "primary.xml";
    //ifgzstream st ( argv[1] );
    std::ifstream st ( argv[1] );
    
    if ( st.bad() )
      ZYPP_THROW(Exception("archivo malo"));
    
    parser::ParserProgress::Ptr progress;
    YUMSourceEventHandler npp;
    progress.reset( new parser::ParserProgress( npp, PathInfo(filename).size()  ) );
    
    parser::yum::YUMPrimaryParser prim(st, "", progress);
    for (; !prim.atEnd(); ++prim)
    {
      std::cout << "iteracion" << std::endl;
      if (*prim == NULL) continue;      // incompatible arch detected during parsi
    }    
  }
  catch ( const Exception &e )
  {
    std::cout << e.msg() << endl;
  }
}



