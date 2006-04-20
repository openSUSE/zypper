#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>

#include "boost/filesystem/operations.hpp" // includes boost/filesystem/path.hpp
#include "boost/filesystem/fstream.hpp"    // ditto

#include <boost/iostreams/device/file_descriptor.hpp>

#include <zypp/base/Logger.h>
#include <zypp/Locale.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>
#include <zypp/TranslatedText.h>
///////////////////////////////////////////////////////////////////

#include <zypp/parser/yum/YUMParser.h>
#include <zypp/base/Logger.h>
#include <zypp/source/yum/YUMScriptImpl.h>
#include <zypp/source/yum/YUMMessageImpl.h>
#include <zypp/source/yum/YUMPackageImpl.h>
#include <zypp/source/yum/YUMSourceImpl.h>

#include <map>
#include <set>

#include <zypp/CapFactory.h>

using namespace zypp::detail;

using namespace std;
using namespace zypp;
using namespace zypp::parser::yum;
using namespace zypp::source::yum;



//using namespace DbXml;

int main()
{
  try
  { 
    ZYpp::Ptr z = getZYpp();
    //while(1);
  }
  catch (...)
  {
    MIL << "sorry" << std::endl;
  } 
}


