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

#include "zypp/source/SourceInfo.h"
#include "zypp/target/store/PersistentStorage.h"
#include "zypp/target/store/XMLFilesBackend.h"

#include "zypp/parser/xmlstore/XMLPatternParser.h"

#include "zypp/base/Logger.h"

#include "zypp/SourceFactory.h"
#include "zypp/Source.h"
#include "zypp/source/SourceImpl.h"
#include "zypp/PathInfo.h"
#include "zypp/ExternalProgram.h"

#include <map>
#include <set>

#include "zypp/CapFactory.h"

#include "zypp/ZYppFactory.h"

#include "zypp/target/store/serialize.h"

#include "Benchmark.h"

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

static std::string dump( const ResStore &store )
{
  stringstream out;
  for (ResStore::const_iterator it = store.begin(); it != store.end(); it++)
  {
    MIL << **it << endl;
    if ( isKind<Patch>(*it) )
    {
      MIL << asKind<Patch>(*it)->atoms().size() << std::endl;
    }
    MIL << "------------------------------------------------" << endl;
    MIL << (**it).deps() << endl;
  }
  return out.str();
}

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
    
//    filesystem::recursive_rmdir( zyppvar );
//    filesystem::recursive_rmdir( zyppcache );
    _store.clear();
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
    
    for (ResStore::const_iterator it = store.begin(); it != store.end(); it++)
      _store.insert(*it);
    
    MIL << "done reading " << store.size() << " from source type " << _source.type() << ": Total resolvables now: " << _store.size() <<  std::endl;
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
    _source.storeMetadata(sourceCacheDir());
  }
  
  void storeKnownSources()
  {
    INT << "===[SOURCES]==========================================" << endl;
    source::SourceInfo data;
    data.setUrl(_source.url());
    data.setType(_source.type());
    data.setAlias(_source.alias());

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
  
  void read_known_sources_test()
  {
    clean();
    unpackDatabase("db.tar.gz");
    initStorageBackend();
    source::SourceInfoList sources = _backend->storedSources();
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
  
  int sles10_machcd_source_read_test()
  {
    clean();
    //initSourceWithCache(Url("ftp://machcd2.suse.de/SLES/SLES-10-i386-Beta8/DVD1"));
    //initSourceWithCache(Url("ftp://10.10.0.5/CDs/SUSE-Linux-10.1-Build_803-Addon-BiArch/CD1"));
    initSourceWithCache(Url("ftp://machcd2.suse.de/CDs/SLES-10-CD-i386-Build_1001/CD1"));
    //initSource(Url("dir:/mounts/dist/10.0-i386"));
    
    ResStore store = readSourceResolvables();
    storeSourceMetadata();
    return 0;
  }
  
  int quick_refresh_test()
  {
    //clean();
    
    //initSourceWithCache(Url("ftp://you.suse.de/pub/suse/update/10.1"));
    initSourceWithCache(Url("dir:/mounts/dist/install/stable-x86"));
    
    storeSourceMetadata();
    return 0;
  }
  
  int yumbug_read_test()
  {
    unpackDatabase("yum-createrepo-fixed.tar.gz");
    initSource(Url("dir:/space/sources/zypp-trunk/trunk/libzypp/testsuite/target/tests/repo"));
    ResStore store = readSourceResolvables();
    dump(store);
    return 0;
  }
   
  int huha_yum_patch_bug_read_test()
  {
    clean();
    initStorageBackend();
    //initSource(Url("http://armstrong.suse.de/download/Code/10/update/i386/")); 
    initSource(Url("ftp://machcd2.suse.de/CDs/SLES-10-CD-i386-Build_1001/CD1"));
    ResStore store = readSourceResolvables();
    
    for (ResStore::const_iterator it = _store.begin(); it != _store.end(); it++)
    {
      //DBG << **it << endl
      ResObject::Ptr res = *it;
      if ( isKind<Patch>(res) )
      {
        Patch::Ptr p = asKind<Patch>(res);
        MIL << "From yum source, patch " << p->name() << "-" << p->edition() << " has " << p->atoms().size() << " atoms." << std::endl;
      }
    }
    dump(store);
    writeResolvablesInStore();
    std::list<ResObject::Ptr> objs = readStoreResolvables();
    for ( std::list<ResObject::Ptr>::const_iterator it = objs.begin(); it != objs.end(); it++ )
    {
      _backend->deleteObject(*it);
    }
    return 0;
  }
      
  int download_rpm_checksum_test()
  {
    clean();
    //initStorageBackend();
    initSource(Url("file:///mounts/machcd2/CDs/SLED-10-CD-i386-Beta9/CD1")); 
    //initSource(Url("http://ftp.chg.ru/pub/Linux/SuSE/suse/update/10.1"));
    ResStore store = readSourceResolvables();
    
    for (ResStore::const_iterator it = _store.begin(); it != _store.end(); it++)
    {
      //DBG << **it << endl
      ResObject::Ptr res = *it;
      if ( isKind<Package>(res) )
      {
        Package::Ptr p = asKind<Package>(res);
        MIL << "From yum source, package " << p->name() << "-" << p->edition() << std::endl;
        if ( p->name().substr( 0, 2) == "pa" )
          _source.providePackage(p);
      }
    }
    //dump(store);
    //writeResolvablesInStore();
    //readStoreResolvables();
    return 0;
  }
      
  int agruenbacher_cap_test()
  {
    clean();
    initSource(Url("dir:/space/tmp")); 
    ResStore store = readSourceResolvables();
    
    //for (ResStore::const_iterator it = _store.begin(); it != _store.end(); it++)
    //{
      //ResObject::Ptr res = *it;
    //}
    dump(store);
    return 0;
  }
  
  //SUSE-Linux-10.1-beta6-x86_64-CD1
  int sl_101_beta6_x86_64_source_read_test()
  {
    clean();
    //initSource(Url("ftp://cml.suse.cz/netboot/find/SUSE-Linux-10.1-beta6-x86_64-CD1"));
    unpackDatabase("SUSE-Linux-10.1-beta6-x86_64-source-metadata-cache.tar.gz");
    initSourceWithCache(Url("dir:/fake-not-available-dir"));
    
    ResStore store = readSourceResolvables();
    clean();
    return 0;
  }
  
  int armstrong_yum_source_source_read_test()
  {
    clean();
    initSourceWithCache(Url("http://armstrong.suse.de/download/Code/10/update/i386"));
    ResStore store = readSourceResolvables();
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
    return 0;    
  }
  
  int parse_store_with_yum_test()
  {
    clean();
    unpackDatabase("db.tar.gz");
    std::ifstream res_file("var/lib/zypp/db/selections/d9b9b61057cdbcf9cf5c8f21408e3db5");
    XMLPatternParser iter(res_file,"");
    for (; !iter.atEnd(); ++iter)
    {
      //YUMPatternData data = **iter;
      //break;
    }
    clean();
    return 0;    
  }
  
};

#define TEST_FUNC_NAME(a) a##_test()

#define RUN_TEST(name) \
  do { \
    StorageTargetTest test("./"); \
    test.TEST_FUNC_NAME(name); \
  } while (false)

int main()
{ 
  getZYpp()->initializeTarget("/");
  try
  {  
    //RUN_TEST(armstrong_yum_source_source_read);
    //RUN_TEST(huha_yum_patch_bug_read);
    //RUN_TEST(yumbug_read);#
    //RUN_TEST(sles10_machcd_source_read);
    RUN_TEST(quick_refresh);
    //RUN_TEST(huha_yum_patch_bug_read);
    /*
    RUN_TEST(storage_read);
    RUN_TEST(read_source_cache);
    RUN_TEST(read_known_sources);
    RUN_TEST(named_flags);
    RUN_TEST(publickey);
    
    //RUN_TEST(parse_store_with_yum);
    RUN_TEST(sles10_machcd_source_read);
    RUN_TEST(sl_101_beta6_x86_64_source_read);
    
    //RUN_TEST(armstrong_yum_source_source_read);
    
    MIL << "store testsuite passed" << std::endl;
    */
    return 0;
  }
  catch ( std::exception &e )
  {
    ERR << "store testsuite failed" << std::endl;
    return 1;
  }
  return 1;
}
