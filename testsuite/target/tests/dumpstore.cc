#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>

#include "boost/filesystem/operations.hpp" // includes boost/filesystem/path.hpp
#include "boost/filesystem/fstream.hpp"    // ditto

#include <boost/iostreams/device/file_descriptor.hpp>

#include "zypp/SourceFactory.h"

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
///////////////////////////////////////////////////////////////////

#include "zypp/target/store/PersistentStorage.h"
#include "zypp/base/Logger.h"
#include "testsuite/src/utils/TestUtils.h"

#include "Benchmark.h"

using namespace zypp::detail;
using namespace std;
using namespace zypp;
using namespace zypp::storage;
using namespace zypp::source;

using namespace boost::filesystem;

using namespace zypp::testsuite::utils;

#define PATCH_FILE "../../../devel/devel.jsrain/repodata/patch.xml"

typedef std::list<ResObject::Ptr> ResObjectPtrList;

int main( int argc, char * argv[])
{
  bool descr = false;
  bool deps = false;
  
  int argpos = 1;

  while ( argpos  < argc )
  {
    if ((strcmp( argv[argpos], "-h") == 0) || (strcmp( argv[argpos], "--help") == 0)) {
      cerr << "Usage: dumpstore [--descr] [--deps]" << endl;
      exit (1);
    }
    if (strcmp( argv[argpos], "--descr") == 0) {
      descr = true;
    }
    if (strcmp( argv[argpos], "--deps") == 0) {
      deps = true;     
    }
    ++argpos;
  }
  
  try { 
    //getZYpp()->initTarget("/", false);
    PersistentStorage *_backend = new PersistentStorage();
    _backend->init("/");
    ResObjectPtrList objs = _backend->storedObjects();
    dump(objs, descr, deps);
    return 0;
  }
  catch ( std::exception &e )
  {
    ERR << "store read failed" << std::endl;
    return 1;
  }
  return 1;
}
