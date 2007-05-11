/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <iostream>
#include <map>

#include "zypp/base/Logger.h"
#include "zypp/base/Measure.h"

#include "zypp2/cache/QueryFactory.h"
#include "zypp2/cache/CapabilityQuery.h"
#include "zypp2/cache/sqlite_detail/CacheSqlite.h"
#include "zypp2/cache/DatabaseTypes.h"
#include "zypp2/cache/CacheCommon.h"
#include "zypp2/cache/sqlite_detail/QueryFactoryImpl.h"
#include "zypp/detail/ResImplTraits.h"
#include "zypp/CapFactory.h"

#include "zypp2/repository/cached/CachedRepositoryImpl.h"
#include "zypp2/repository/cached/CachedRepositoryPackageImpl.h"


using namespace zypp::detail;
using namespace zypp::cache;
using namespace std;
using namespace sqlite3x;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace repository
{ /////////////////////////////////////////////////////////////////
 ///////////////////////////////////////////////////////////////////
namespace cached
{ /////////////////////////////////////////////////////////////////

CachedRepositoryImpl::CachedRepositoryImpl( const Pathname &dbdir)
  : _dbdir(dbdir)
{

}

CachedRepositoryImpl::~CachedRepositoryImpl()
{

}


void CachedRepositoryImpl::factoryInit()
{
  MIL << "Plaindir repository initialized." << std::endl;
}

void read_capabilities( sqlite3_connection &con, map<data::RecordId, NVRAD> &nvras );

Rel CachedRepositoryImpl::relationFor( const data::RecordId &id )
{
  Rel rel;
  std::map<data::RecordId, Rel>::const_iterator it;
  if ( (it = _rel_cache.find(id) ) != _rel_cache.end() )
    rel = it->second;
  else
    ZYPP_THROW(Exception("Inconsistent Rel"));
  
  return rel;

}

Resolvable::Kind CachedRepositoryImpl::kindFor( const data::RecordId &id )
{
  Resolvable::Kind kind;
  std::map<data::RecordId, Resolvable::Kind>::const_iterator it;
  if ( (it = _kind_cache.find(id) ) != _kind_cache.end() )
    kind = it->second;
  else
    ZYPP_THROW(Exception("Inconsistent Kind"));
  
  return kind;
}

Dep CachedRepositoryImpl::deptypeFor( const data::RecordId &id )
{
  std::map<data::RecordId, string>::const_iterator it;
  if ( (it = _deptype_cache.find(id) ) != _deptype_cache.end() )
    return Dep(it->second);
  else
  {
    ERR << "deptype: " << id << endl;
    ZYPP_THROW(Exception("Inconsistent deptype"));
  }
}

Arch CachedRepositoryImpl::archFor( const data::RecordId &id )
{
   
  Arch arch;
  std::map<data::RecordId, Arch>::const_iterator it;
  if ( (it = _arch_cache.find(id) ) != _arch_cache.end() )
    arch = it->second;
  else
    ZYPP_THROW(Exception("Inconsistent Arch"));
  
  return arch;
}

void CachedRepositoryImpl::createResolvables()
{
  debug::Measure m("create resolvables");
  CapFactory capfactory;
  try {
    sqlite3_connection con((_dbdir + "zypp.db").asString().c_str());
    con.executenonquery("PRAGMA cache_size=8000;");
    con.executenonquery("BEGIN;");
    
    // get all types
    sqlite3_command select_types_cmd( con, "select id,class,name from types;");
    sqlite3_reader reader = select_types_cmd.executereader();
    
    while(reader.read())
    {
      data::RecordId id = reader.getint64(0);
      string klass = reader.getstring(1);
      string name = reader.getstring(2);
      if ( klass == "arch" )
        _arch_cache[id] = Arch(name);
      if ( klass == "rel" )
        _rel_cache[id] = Rel(name);
      if ( klass == "kind" )
        _kind_cache[id] = Resolvable::Kind(name);
      if ( klass == "deptype" )
        _deptype_cache[id] = name;
    }
    
    MIL << "archs: " << _arch_cache.size() << endl;
    MIL << "rel  : " << _rel_cache.size() << endl;
    MIL << "kind : " << _kind_cache.size() << endl;
    MIL << "deptype : " << _deptype_cache.size() << endl;
    
    sqlite3_command cmd( con, "select id,name,version,release,epoch,arch,kind from resolvables;");
    map<data::RecordId, NVRAD> nvras;
    
    reader = cmd.executereader();
    while(reader.read())
    {
      long long id = reader.getint64(0);
      Dependencies deps;
      
      // Collect basic Resolvable data
      nvras[id] = NVRAD( reader.getstring(1),
                       Edition( reader.getstring(2), reader.getstring(3), reader.getint(4) ),
                       archFor(reader.getint(5)),
                       deps
                     );
    }
    
    MIL << "Done reading resolvables nvra" << endl;
    
    read_capabilities( con, nvras);
    
    for ( map<data::RecordId, NVRAD>::const_iterator it = nvras.begin(); it != nvras.end(); ++it )
    {
      ResImplTraits<CachedRepositoryPackageImpl>::Ptr impl = new CachedRepositoryPackageImpl(selfRepository());
      Package::Ptr package = detail::makeResolvableFromImpl( it->second, impl );
      _store.insert (package);
    }
    con.executenonquery("COMMIT;");
  }
  catch(exception &ex) {
      cerr << "Exception Occured: " << ex.what() << endl;
   }
  //extract_packages_from_directory( _store, thePath, selfRepositoryRef(), true );
   
}

void CachedRepositoryImpl::read_capabilities( sqlite3_connection &con, map<data::RecordId, NVRAD> &nvras )
{
  CapFactory capfactory;
  
  
  // precompile statements
  
  
  
//   map<data::RecordId, capability::CapabilityImpl::Ptr> named_caps;
//   sqlite3_command select_named_cmd( con, "select v.id, c.refers_kind, n.name, v.version, v.release, v.epoch, v.relation named_capabilities v, capabilities c, names n where v.name_id=n.id and c.id=ncc.capability_id and ncc.named_capability_id=v.id;");
//   {
//     debug::Measure mnc("read named capabilities");
//     sqlite3_reader reader = select_named_cmd.executereader();
//     while  ( reader.read() )
//     {
//       
//     }
//   }
  sqlite3_command select_named_cmd( con, "select v.refers_kind, n.name, v.version, v.release, v.epoch, v.relation, v.dependency_type, v.resolvable_id from named_capabilities v, names n where v.name_id=n.id;");
  sqlite3_command select_file_cmd( con, "select fc.refers_kind, dn.name, fn.name, fc.dependency_type, fc.resolvable_id from file_capabilities fc, files f, dir_names dn, file_names fn where f.id=fc.file_id and f.dir_name_id=dn.id and f.file_name_id=fn.id;");
  
  {
    debug::Measure mnc("read named capabilities");
    sqlite3_reader reader = select_named_cmd.executereader();
    while  ( reader.read() )
    {
      
      Resolvable::Kind refer = kindFor(reader.getint(0));
      Rel rel = relationFor(reader.getint(5));
      
      data::RecordId rid = reader.getint64(7);
  
      if ( rel == zypp::Rel::NONE )
      {
        capability::NamedCap *ncap = new capability::NamedCap( refer, reader.getstring(1) );
        zypp::Dep deptype = deptypeFor(reader.getint(6));  
        nvras[rid][deptype].insert( capfactory.fromImpl( capability::CapabilityImpl::Ptr(ncap) ) ); 
      }
      else
      {
        capability::VersionedCap *vcap = new capability::VersionedCap( refer, reader.getstring(1), /* rel */ rel, Edition( reader.getstring(2), reader.getstring(3), reader.getint(4) ) );
        zypp::Dep deptype = deptypeFor(reader.getint(6));
        nvras[rid][deptype].insert( capfactory.fromImpl( capability::CapabilityImpl::Ptr(vcap) ) ); 
      }
    }
  }
  
  {
    debug::Measure mnf("read file capabilities");
    sqlite3_reader reader = select_file_cmd.executereader();
    while  ( reader.read() )
    {
      Resolvable::Kind refer = kindFor(reader.getint(0));
      capability::FileCap *fcap = new capability::FileCap( refer, reader.getstring(1) + "/" + reader.getstring(2) );
      zypp::Dep deptype = deptypeFor(reader.getint(3));
      data::RecordId rid = reader.getint64(4);
      nvras[rid][deptype].insert( capfactory.fromImpl( capability::CapabilityImpl::Ptr(fcap) ) ); 
    }
  }
  
  MIL << nvras.size() << " capabilities" << endl;
}








/////////////////////////////////////////////////////////////////
} // namespace plaindir
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace repository
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////

