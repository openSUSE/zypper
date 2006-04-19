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
  Locale a("en");
  ZYpp::Ptr z = getZYpp();
  Locale lang( z->getTextLocale() );
  
  while (lang != Locale())
  {
    MIL << "[" << lang << "]" << std::endl;
    lang = lang.fallback();
  }
  
  TranslatedText t;
  t.setText("hola");
  MIL << t.text() << std::endl;
  
  t.setText("hello", Locale("en"));
  MIL << t.text() << std::endl;
  
  t.setText("hellow", Locale("en_US"));
  MIL << t.text() << std::endl;
  
  
}


