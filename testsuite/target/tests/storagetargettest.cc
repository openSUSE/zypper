#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>
 #include <ctime>

#include "boost/filesystem/operations.hpp" // includes boost/filesystem/path.hpp
#include "boost/filesystem/fstream.hpp"    // ditto

#include <boost/iostreams/device/file_descriptor.hpp>

#include "zypp/SourceFactory.h"

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
///////////////////////////////////////////////////////////////////

#include "zypp/target/store/PersistentStorage.h"
#include "zypp/target/store/XMLFilesBackend.h"

#include "zypp/base/Logger.h"

#include "zypp/SourceFactory.h"
#include "zypp/Source.h"
#include "zypp/source/SourceImpl.h"
#include "zypp/PathInfo.h"

#include <map>
#include <set>

#include "zypp/CapFactory.h"

#include "zypp/target/store/serialize.h"

#include "boost/filesystem/operations.hpp" // includes boost/filesystem/path.hpp
#include "boost/filesystem/fstream.hpp"    // ditto

using namespace zypp::detail;
using namespace std;
using namespace zypp;
using namespace zypp::storage;
using namespace zypp::source;

using namespace boost::filesystem;

#define PATCH_FILE "../../../devel/devel.jsrain/repodata/patch.xml"

//using namespace DbXml;

static int assert_dir_is_broken()
{
  Pathname p("./1/2/3/4/5");
  if (0 != assert_dir(p, 0700))
    ZYPP_THROW(Exception("Cannot create directory" + p.asString()));
  else
    MIL << "created " << p << std::endl;
}

static int readAndStore()
{
  
  /* Read YUM resolvables from a source */
  INT << "===[START]==========================================" << endl;
  SourceFactory _f;
  Pathname p = "/";
  //  Url url = Url("ftp://cml.suse.cz/netboot/find/SUSE-10.1-CD-OSS-i386-Beta1-CD1");
  Url url = Url("dir:/space/sources/zypp-trunk/trunk/libzypp/devel/devel.jsrain");
  //Url url = Url("dir:/mounts/dist/next-i386");
  //Url url = Url("dir:/space/zypp-test/sles-beta5");
  //Source_Ref s = _f.createFrom( url, p, "testsource", "./source-cache" );
  Source_Ref s = _f.createFrom( url, p, "testsource");
  ResStore store = s.resolvables();
  MIL << "done reading source type " << s.type() << ": " << store <<  std::endl;
  s.storeMetadata("./source-cache");
  Pathname root("."); 
  //exit(1);
  XMLFilesBackend backend(root);

  //backend.setRandomFileNameEnabled(true);
  clock_t time_start, curr_time;
  time_start = clock();

  // now write the files
  DBG << "Writing objects..." << std::endl;
  for (ResStore::const_iterator it = store.begin(); it != store.end(); it++)
  {
    DBG << **it << endl;
    backend.storeObject(*it);
  }
  
  curr_time = clock() - time_start;           // time in micro seconds
  MIL << "Wrote " << store.size() << " objects in " << (double) curr_time / CLOCKS_PER_SEC << " seconds" << std::endl;
  
  time_start = clock();
  std::list<ResObject::Ptr> objs = backend.storedObjects();
  curr_time = clock() - time_start;           // time in micro seconds
  MIL << "Read " << objs.size() << " patches in " << (double) curr_time / CLOCKS_PER_SEC << " seconds" << std::endl;

  INT << "===[SOURCES]==========================================" << endl;
  PersistentStorage::SourceData data;
  data.url = "http://localhost/rpms";
  data.type = "yum";
  data.alias = "duncan bugfree rpms";

  backend.storeSource(data);

  data.url = "http://localhost/debd";
  data.type = "yum";
  data.alias = "duncan bugfree 2";

  backend.storeSource(data);

  MIL << "Wrote 2 sources" << std::endl;
  std::list<PersistentStorage::SourceData> sources = backend.storedSources();
  MIL << "Read " << sources.size() << " sources" << std::endl;
  return 0;
}

int nld10TestCase()
{
  SourceFactory _f;
  Pathname p = "/";
  Url url = Url("dir:/mounts/dist/install/SLP/NLD-10-Beta4/i386/CD1");
  Source_Ref s = _f.createFrom( url, p, "testsource");
  ResStore store = s.resolvables();
  MIL << "done reading source type " << s.type() << ": " << store <<  std::endl;
  Pathname root("."); 
  XMLFilesBackend backend(root);

  DBG << "Writing objects..." << std::endl;
  for (ResStore::const_iterator it = store.begin(); it != store.end(); it++)
  {
    DBG << **it << endl;
    backend.storeObject(*it);
  }
  MIL << "Wrote " << store.size() << " objects" << std::endl;
  
  std::list<ResObject::Ptr> objs = backend.storedObjects(ResTraits<zypp::Selection>::kind);
  MIL << "Read " << objs.size() << " patches" << std::endl;
  return 0;
}

int main()
{
  int error = 0;
  if ((error = readAndStore()) != 0) return error;
  //if ((error = assert_dir_is_broken()) != 0) return error;
  //if ((error = nld10TestCase()) != 0) return error;  
  return 0;
}
