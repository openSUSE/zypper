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

typedef std::list<ResObject::Ptr> ResObjectPtrList;

static std::string dump( const CapSet &caps )
{
  stringstream out;
  CapSet::iterator it = caps.begin();
  for ( ; it != caps.end(); ++it)
  {
    out << (*it).asString() << std::endl;
  }
  return out.str();
}


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
    _backend = 0L;
    _root = root;
  }

  ~StorageTargetTest()
  {
    delete _backend;
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
  
  Pathname sourceCacheDir()
  {
    return _root + "/source-cache";
  }
  
  void initSourceWithCache(const Url &url)
  {
    Url _url = url;
    SourceFactory source_factory;
    Pathname p = "/";
    _source = source_factory.createFrom( _url, p, "testsource", sourceCacheDir() );
  }
  
  void initSource(const Url &url)
  {
    Url _url = url;
    SourceFactory source_factory;
    Pathname p = "/";
    _source = source_factory.createFrom( _url, p, "testsource");
  }
  
  void initStorageBackend()
  {
    _backend = new XMLFilesBackend(_root);
  }
  
  ResStore readSourceResolvables()
  {
    ResStore store;
    store = _source.resolvables();
    MIL << "done reading source type " << _source.type() << ": " << _store <<  std::endl;
    return store;
  }
  
  std::list<ResObject::Ptr> readStoreResolvables()
  {
    std::list<ResObject::Ptr> objs = _backend->storedObjects();
    MIL << "Read " << objs.size() << " objects." << std::endl;
    return objs;
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
   
  //////////////////////////////////////////////////////////////
  // TESTS
  //////////////////////////////////////////////////////////////
  
  int storage_read_test()
  {
    Benchmark b(__PRETTY_FUNCTION__);
    MIL << "===[START: storage_read_test()]==========================================" << endl;
    clean();
    unpackDatabase("db.tar.gz");
    initStorageBackend();
    ResObjectPtrList objs = readStoreResolvables();
    for ( ResObjectPtrList::const_iterator it = objs.begin(); it != objs.end(); it++)
    {
      ResObject::Ptr res = *it;
      if ( isKind<Selection>( res ) && res->name() == "Multimedia" )
      {
        // requires basis-sound and x11
        if ( res->deps()[Dep::REQUIRES].size() != 2 )     
        {
          ERR << dump( res->deps()[Dep::REQUIRES] ) ;   
          ZYPP_THROW(Exception("Parsed of requires for Multimedia Selection failed.")); 
        }
        // provides multimedia and multimedia == version
        if ( res->deps()[Dep::PROVIDES].size() != 2 )
        {        
          ERR << dump( res->deps()[Dep::PROVIDES] ) ;
          ZYPP_THROW(Exception("Parsed of provides for Multimedia Selection failed.")); 
        }
        // recommends like 100 packages
        if ( res->deps()[Dep::RECOMMENDS].size() < 60 )        
        {
          ERR << dump( res->deps()[Dep::RECOMMENDS] ) ;
          ZYPP_THROW(Exception("Parsed of recommends for Multimedia Selection failed."));
        }
      }
    }
    clean();
    return 0;
  }
  
  void test_read_known_sources()
  {
    clean();
    unpackDatabase("db.tar.gz");
    initStorageBackend();
    std::list<PersistentStorage::SourceData> sources = _backend->storedSources();
    MIL << "Read " << sources.size() << " sources" << std::endl;
    if ( sources.size() != 2 )
      ZYPP_THROW(Exception("Known Sources read FAILED")); 
  }   
  
  int read_source_cache_test()
  {
    clean();
    // suse 10.0 metadata cache
    unpackDatabase("source-metadata-cache-1.tar.gz");
    initSourceWithCache(Url("dir:/fake-not-available-dir"));
    ResStore store = readSourceResolvables();
    clean();
    return 0;
  }
  
  int named_flags_test()
  {
    clean();
    initStorageBackend();
    
    _backend->setFlag("locks", "name=bleh");
    _backend->setFlag("locks", "all except me");
  
    std::set<std::string> flags;
      
    flags = _backend->flags("locks");
    if (flags.size() != 2 )
      ZYPP_THROW(Exception("wrote 2 flags, read != 2"));
  
    _backend->removeFlag("locks", "all except me");
    flags = _backend->flags("locks");
    if (flags.size() != 1 )
      ZYPP_THROW(Exception("wrote 2 flags, deleted 1, read != 1"));
    
    return 0;
  }
  
  int publickey_test()
  {
    clean();
    // suse 10.0 metadata cache
    unpackDatabase("source-metadata-cache-1.tar.gz");
    initSourceWithCache(Url("dir:/fake-not-available-dir"));
    std::list<Pathname> keys = _source.publicKeys();
    if (keys.size() != 4 )
      ZYPP_THROW(Exception("Read wrong number of keys"));
    
    clean();
    return 0;
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
    s.storeMetadata("./source-cache");
    */
    /*
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
    */
    return 0;
  }
  
  int store_test()
  {
    MIL << "===[START: store_test()]==========================================" << endl;
    
  }
  
};


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
  try
  {
    StorageTargetTest test1("./");
    test1.storage_read_test();
  
    StorageTargetTest test2("./");
    test2.read_source_cache_test();
  
    StorageTargetTest test3("./");
    test3.test_read_known_sources();
    
    StorageTargetTest test4("./");
    test4.named_flags_test();
    
    StorageTargetTest test5("./");
    test5.publickey_test();
    
    MIL << "store testsuite passed" << std::endl;
    return 0;
  }
  catch ( std::exception &e )
  {
    ERR << "store testsuite failed" << std::endl;
    return 1;
  }
  return 1;
}
