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
#include "zypp/ExternalProgram.h"

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

struct Benchmark
{
  clock_t _time_start;
  clock_t _curr_time;
  std::string _name;
  Benchmark( const std::string &name)
  {
    _name = name;
    _time_start = clock();
  }
  
  ~Benchmark()
  {
    _curr_time = clock() - _time_start;           // time in micro seconds
    MIL << _name << " completed in " << (double) _curr_time / CLOCKS_PER_SEC << " seconds" << std::endl;
  }
};

struct StorageTargetTest
{
  Pathname _root;
  Source_Ref _source;
  XMLFilesBackend *_backend;
  ResStore _store;
  
  StorageTargetTest(const Pathname &root)
  {
    _root = root;
  }

  void clean()
  {
    Pathname zyppvar = _root + "/var";
    Pathname zyppcache = _root + "/source-cache";
    
    if (zyppvar == "/var")
      ZYPP_THROW(Exception("I refuse to delete /var"));
    
    filesystem::recursive_rmdir( zyppvar );
    filesystem::recursive_rmdir( zyppcache );
  }
  
  void unpackDatabase(const Pathname &name)
  {
    const char *const argv[] = {
      "/bin/tar",
      "-zxpvf",
      name.asString().c_str(),
      NULL
    };

    ExternalProgram prog( argv, ExternalProgram::Stderr_To_Stdout );
    for ( string output( prog.receiveLine() ); output.length(); output = prog.receiveLine() ) {
      DBG << "  " << output;
    }
    int ret = prog.close();
  }
  
  void initSource(const Url &url)
  {
    Url _url;
    SourceFactory source_factory;
    Pathname p = "/";
    _source = source_factory.createFrom( _url, p, "testsource", _root + "/source-cache" );
  }
  
  void initStorageBackend()
  {
    _backend = new XMLFilesBackend(_root);
  }
  
  void readSourceResolvables()
  {
    _store = _source.resolvables();
    MIL << "done reading source type " << _source.type() << ": " << _store <<  std::endl;
  }
  
  void readStoreResolvables()
  {
    std::list<ResObject::Ptr> objs = _backend->storedObjects();
    MIL << "Read " << objs.size() << " objects." << std::endl;
  }
  
  void writeResolvablesInStore()
  {
    MIL << "Writing objects..." << std::endl;
    for (ResStore::const_iterator it = _store.begin(); it != _store.end(); it++)
    {
      DBG << **it << endl;
      _backend->storeObject(*it);
    }
  }
  
  void storeSourceMetadata()
  {
    _source.storeMetadata(_root + "/source-cache");
  }
  
  void storeKnownSources()
  {
    INT << "===[SOURCES]==========================================" << endl;
    PersistentStorage::SourceData data;
    data.url = _source.url().asString();
    data.type = _source.type();
    data.alias = _source.alias();

    _backend->storeSource(data);
    MIL << "Wrote 1 source" << std::endl;
  }
  
  void readKnownSources()
  {
    std::list<PersistentStorage::SourceData> sources = _backend->storedSources();
    MIL << "Read " << sources.size() << " sources" << std::endl;
  }   
   
  //////////////////////////////////////////////////////////////
  // TESTS
  //////////////////////////////////////////////////////////////
  
  int storage_read_test()
  {
    Benchmark b(__FUNCTION__);
    MIL << "===[START: storage_read_test()]==========================================" << endl;
    clean();
    unpackDatabase("db.tar.gz");
    initStorageBackend();
    readStoreResolvables();
    clean();
  }
  
  int read_test()
  {
    MIL << "===[START: read_test()]==========================================" << endl;
    /*
    initSource();
    readSourceResolvables();
    initStorageBackend();
    writeResolvablesInStore();
    readStoreResolvables();
    */
    return 0;
  }
  
  int store_test()
  {
    MIL << "===[START: store_test()]==========================================" << endl;
    
  }
  
  int publickeys_test()
  {
    MIL << "===[START: publickeys_test()]==========================================" << endl;
    
  }
  
  
};


static int readAndStore()
{
  
  /* Read YUM resolvables from a source */
  INT << "===[START]==========================================" << endl;
  SourceFactory _f;
  Pathname p = "/";
  Pathname root("."); 
  //Url url("cd:///");
  //static Url url = Url("ftp://cml.suse.cz/netboot/find/SUSE-10.1-CD-OSS-i386-Beta1-CD1");
//static Url url = Url("dir:/space/sources/zypp-trunk/trunk/libzypp/devel/devel.jsrain");
//static Url url("ftp://machcd2.suse.de/SLES/SUSE-Linux-10.1-beta5-i386/CD1");
  Url url = Url("dir:/mounts/dist/10.0-i386");
//Url url = Url("dir:/space/zypp-test/sles-beta5");
  Source_Ref s = _f.createFrom( url, p, "testsource", "./source-cache" );
  //Source_Ref s = _f.createFrom( url, p, "testsource");
  ResStore store = s.resolvables();
  MIL << "done reading source type " << s.type() << ": " << store <<  std::endl;
  s.storeMetadata("./source-cache");
  
  std::list<Pathname> keys = s.publicKeys();
  //Pathname root("."); 
  //exit(1);
  XMLFilesBackend backend(root);
  backend.setFlag("locks", "name=bleh");
  backend.setFlag("locks", "all except me");
  
  std::set<std::string> flags;
      
  flags = backend.flags("locks");
  if (flags.size() != 2 )
    ZYPP_THROW(Exception("wrote 2 flags, read != 2"));
  
  backend.removeFlag("locks", "all except me");
  flags = backend.flags("locks");
  if (flags.size() != 1 )
    ZYPP_THROW(Exception("wrote 2 flags, deleted 1, read != 1"));
  
  
  //backend.setRandomFileNameEnabled(true);
  clock_t time_start, curr_time;
  time_start = clock();

  // now write the files
  DBG << "Writing objects..." << std::endl;
  for (ResStore::const_iterator it = store.begin(); it != store.end(); it++)
  {
    DBG << **it << endl;
    backend.storeObject(*it);
    //backend.setObjectFlag(*it, "blah1");
    //backend.setObjectFlag(*it, "locked");
    //backend.setObjectFlag(*it, "license-acepted");
  }
  
  curr_time = clock() - time_start;           // time in micro seconds
  MIL << "Wrote " << store.size() << " objects in " << (double) curr_time / CLOCKS_PER_SEC << " seconds" << std::endl;
  
  for ( ResStore::const_iterator it = store.begin(); it != store.end(); it++)
  {
    std::set<std::string> flags = backend.objectFlags(*it);
    if ( flags.size() != 3 )
    {
      for ( std::set<std::string>::const_iterator itf = flags.begin(); itf != flags.end(); ++it)
      {
        //ERR << "Tag " << *itf << std::endl;
      }
      
      //ERR << "Saved 3 tags, read " << flags.size() << " for " << **it << std::endl;
      //ZYPP_THROW(Exception("Saved 3 tags, read other")); 
    }
  }
  //exit(0);
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
  Pathname root("."); 
  Url url = Url("dir:/mounts/dist/install/SLP/NLD-10-Beta4/i386/CD1");
  Source_Ref s = _f.createFrom( url, p, "testsource");
  ResStore store = s.resolvables();
  MIL << "done reading source type " << s.type() << ": " << store <<  std::endl;
  //Pathname root("."); 
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
  StorageTargetTest test1("./");
  test1.storage_read_test();
  
  int error = 0;
  //if ((error = only_read()) != 0) return error;
  //if ((error = readAndStore()) != 0) return error;
  //if ((error = assert_dir_is_broken()) != 0) return error;
  //if ((error = nld10TestCase()) != 0) return error;  
  return 0;
}
