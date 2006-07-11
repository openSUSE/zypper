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

#include <zypp/base/Logger.h>


#include <map>
#include <set>

#include "zypp/CapFactory.h"
#include "zypp/KeyRing.h"
#include "zypp/PublicKey.h"

#include "zypp/MediaSetAccess.h"
#include "zypp/source/yum/YUMSourceCacher.h"

using namespace zypp::detail;

using namespace std;
using namespace zypp;
using namespace zypp::source;
//using namespace DbXml;

int main()
{
  //MediaSetAccess ma( Url("cd:///"), Pathname("/"));
  //MIL << "done 1" << std::endl;
  //ChecksumFileChecker checker(CheckSum("sha1", "fa0abb22f703a3a41f7a39f0844b24daf572fd4c"));
  //Pathname local = ma.provideFile("content", 1, checker);
  //MIL << local << std::endl;
  try
  {
    zypp::source::yum::YUMSourceCacher cacher(Pathname("/"));
    cacher.cache( Url("http://ftp-1.gwdg.de/pub/opensuse/distribution/SL-OSS-factory/inst-source/suse"), Pathname("/"));
  }
  catch ( const Exception &e )
  {
    MIL << "Sorry, bye" << endl;
  }
}


