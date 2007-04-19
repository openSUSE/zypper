
#include <sqlite3.h>
#include <map>
#include "zypp2/cache/sqlite3x/sqlite3x.hpp"

#include "zypp/base/Measure.h"
#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"

#include "zypp2/cache/CacheInitializer.h"
#include "zypp2/cache/CacheStore.h"
#include "zypp2/cache/DatabaseTypes.h"

using namespace std;
using namespace zypp;
using namespace zypp::capability;
using namespace zypp::cache;
using namespace sqlite3x;

using zypp::debug::Measure;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace cache
{ /////////////////////////////////////////////////////////////////
    
typedef shared_ptr<sqlite3_command> sqlite3_command_ptr;
  
struct CacheStore::Impl
{
  Impl()
  : name_cache_hits(0)
  {}

 /**
  * SQL statements
  * (we precompile them
  */
  sqlite3_connection con;
  
  sqlite3_command_ptr update_catalog_cmd;
  sqlite3_command_ptr insert_resolvable_in_catalog_cmd;
  
  sqlite3_command_ptr select_name_cmd;
  sqlite3_command_ptr insert_name_cmd;
  
  sqlite3_command_ptr select_dirname_cmd;
  sqlite3_command_ptr insert_dirname_cmd;
  
  sqlite3_command_ptr select_filename_cmd;
  sqlite3_command_ptr insert_filename_cmd;
  
  sqlite3_command_ptr select_catalog_cmd;
  sqlite3_command_ptr insert_catalog_cmd;
  
  sqlite3_command_ptr select_file_cmd;
  sqlite3_command_ptr insert_file_cmd;
  
  //sqlite3_command_ptr insert_dependency_entry_cmd;
  
  sqlite3_command_ptr append_file_dependency_cmd;
  sqlite3_command_ptr append_named_dependency_cmd;
  
  sqlite3_command_ptr append_resolvable_cmd;
  
  map<string, data::RecordId> name_cache;
  int name_cache_hits;
};


CacheStore::CacheStore( const Pathname &dbdir )
  : _pimpl( new Impl() )
{
  cache::CacheInitializer initializer(dbdir, "zypp.db");
  if ( initializer.justInitialized() )
  {
    MIL << "database " << (dbdir + "zypp.db") << " was just created" << endl;
  }
  
  try
  {
    _pimpl->con.open( (dbdir + "zypp.db").asString().c_str());
    //_insert_resolvable_cmd = new sqlite3_command( *_con, INSERT_RESOLVABLE_QUERY );                                    
    //_insert_package_cmd = new sqlite3_command( *_con, INSERT_PACKAGE_QUERY );
  }
  catch(exception &ex)
  {
    //ZYPP_CAUGHT(ex);
    ZYPP_THROW(Exception(ex.what()));
  }
  
  _pimpl->insert_resolvable_in_catalog_cmd.reset( new sqlite3_command( _pimpl->con, "insert into resolvables_catalogs (resolvable_id, catalog_id) values (:resolvable_id, :catalog_id);" ));
  
  _pimpl->update_catalog_cmd.reset( new sqlite3_command( _pimpl->con, "update catalogs set checksum=:checksum, timestamp=:timestamp where id=:catalog_id;" ));
  
  _pimpl->select_catalog_cmd.reset( new sqlite3_command( _pimpl->con, "select id from catalogs where url=:url and path=:path;" ));
  _pimpl->insert_catalog_cmd.reset( new sqlite3_command( _pimpl->con, "insert into catalogs (url,path,timestamp) values (:url,:path,:timestamp);" ));
  
  _pimpl->select_name_cmd.reset( new sqlite3_command( _pimpl->con, "select id from names where name=:name;" ));
  _pimpl->insert_name_cmd.reset( new sqlite3_command( _pimpl->con, "insert into names (name) values (:name);" ));
  
  _pimpl->select_dirname_cmd.reset( new sqlite3_command( _pimpl->con, "select id from dir_names where name=:name;" ));
  _pimpl->insert_dirname_cmd.reset( new sqlite3_command( _pimpl->con, "insert into dir_names (name) values (:name);" ));
  
  _pimpl->select_filename_cmd.reset( new sqlite3_command( _pimpl->con, "select id from file_names where name=:name;" ));
  _pimpl->insert_filename_cmd.reset( new sqlite3_command( _pimpl->con, "insert into file_names (name) values (:name);" ));

  _pimpl->select_file_cmd.reset( new sqlite3_command( _pimpl->con, "select id from files where dir_name_id=:dir_name_id and file_name_id=:file_name_id;" ));
  _pimpl->insert_file_cmd.reset( new sqlite3_command( _pimpl->con, "insert into files (dir_name_id,file_name_id) values (:dir_name_id,:file_name_id);" ));

  //_pimpl->insert_dependency_entry_cmd.reset( new sqlite3_command( _pimpl->con, "insert into capabilities ( resolvable_id, dependency_type, refers_kind ) values ( :resolvable_id, :dependency_type, :refers_kind );" ));
  _pimpl->append_file_dependency_cmd.reset( new sqlite3_command( _pimpl->con, "insert into file_capabilities ( resolvable_id, dependency_type, refers_kind, file_id ) values ( :resolvable_id, :dependency_type, :refers_kind, :file_id );" ));
  _pimpl->append_named_dependency_cmd.reset( new sqlite3_command( _pimpl->con, "insert into named_capabilities ( resolvable_id, dependency_type, refers_kind, name_id, version, release, epoch, relation ) values ( :resolvable_id, :dependency_type, :refers_kind, :name_id, :version, :release, :epoch, :relation );" ));
  
  _pimpl->append_resolvable_cmd.reset( new sqlite3_command( _pimpl->con, "insert into resolvables ( name, version, release, epoch, arch, kind, catalog_id ) values ( :name, :version, :release, :epoch, :arch, :kind, :catalog_id );" ));
  
  // disable autocommit
  _pimpl->con.executenonquery("BEGIN;");
}

CacheStore::CacheStore()
{
  CacheStore( getZYpp()->homePath() );
}

CacheStore::~CacheStore()
{
  _pimpl->con.executenonquery("COMMIT;");
  MIL << "name cache hits: " << _pimpl->name_cache_hits << " | cache size: " << _pimpl->name_cache.size() << endl;
}

void CacheStore::consumePackage( const data::Package &package )
{
  //data::RecordId pkgid = appendResolvable( ResTraits<Package>::kind, NVRA( package.name, package.edition, package.arch ), package.deps );
}

data::RecordId CacheStore::appendResolvable( const data::RecordId &catalog_id,
                                             const Resolvable::Kind &kind, 
                                             const NVRA &nvra, 
                                             const data::Dependencies &deps )
{
  _pimpl->append_resolvable_cmd->bind( ":name", nvra.name );
  _pimpl->append_resolvable_cmd->bind( ":version", nvra.edition.version() );
  _pimpl->append_resolvable_cmd->bind( ":release", nvra.edition.release() );
  _pimpl->append_resolvable_cmd->bind( ":epoch", static_cast<int>( nvra.edition.epoch() ) );
  _pimpl->append_resolvable_cmd->bind( ":arch", zypp_arch2db_arch(nvra.arch) );
  _pimpl->append_resolvable_cmd->bind( ":kind", zypp_kind2db_kind(kind) );
  _pimpl->append_resolvable_cmd->bind( ":catalog_id", catalog_id );
  
  _pimpl->append_resolvable_cmd->executenonquery();

  long long id = _pimpl->con.insertid();
  
  appendDependencies( id, deps );
  /*
  _pimpl->insert_resolvable_in_catalog_cmd->bind(":catalog_id", catalog_id);
  _pimpl->insert_resolvable_in_catalog_cmd->bind(":resolvable_id", id);
  _pimpl->insert_resolvable_in_catalog_cmd->executenonquery();*/
  
  return static_cast<data::RecordId>(id);
  return 1;
}

void CacheStore::appendDependencies( const data::RecordId &resolvable_id, const data::Dependencies &deps )
{
  for ( data::Dependencies::const_iterator it = deps.begin(); it != deps.end(); ++it )
  {
    appendDependencyList( resolvable_id, it->first, it->second );
  }
}

void CacheStore::appendDependencyList( const data::RecordId &resolvable_id, zypp::Dep deptype, const data::DependencyList &caps )
{
  for ( data::DependencyList::const_iterator it = caps.begin(); it != caps.end(); ++it )
  {
    appendDependency( resolvable_id, deptype, *it );
  }
}

void CacheStore::appendDependency( const data::RecordId &resolvable_id, zypp::Dep deptype, capability::CapabilityImpl::Ptr cap )
{
  //DBG << cap << endl;
  if ( cap->refers() != ResTraits<Package>::kind )
  {
    DBG << "invalid capability" << endl;
    return;
  }
  
  if ( cap == 0 )
  {
    DBG << "invalid capability" << endl;
    return;
  }
  
  if ( capability::isKind<FileCap>(cap) )
  {
    appendFileDependency( resolvable_id, deptype, capability::asKind<FileCap>(cap) );
    return;
  }
  else if ( capability::isKind<NamedCap>(cap) )
  {
      appendNamedDependency( resolvable_id, deptype, capability::asKind<NamedCap>(cap) );
  }
}

// data::RecordId CacheStore::lookupOrAppendNamedDependencyEntry( const data::RecordId name_id, const Edition &edition, const zypp::Rel &rel )
// {
//   _pimpl->select_named_dependency_cmd->bind( ":name_id", name_id);
//   _pimpl->select_named_dependency_cmd->bind( ":version", edition.version() );
//   _pimpl->select_named_dependency_cmd->bind( ":release", edition.release() );
//   _pimpl->select_named_dependency_cmd->bind( ":epoch", static_cast<int>( edition.epoch() ) );
//   _pimpl->select_named_dependency_cmd->bind( ":relation", zypp_rel2db_rel( rel ) );
//   long long id = 0;
//   try {
//     id = _pimpl->select_named_dependency_cmd->executeint64();
//   }
//   catch ( const sqlite3x::database_error &e )
//   {
//     // does not exist
//     _pimpl->append_named_dependency_entry_cmd->bind( ":name_id", name_id);
//     _pimpl->append_named_dependency_entry_cmd->bind( ":version", edition.version() );
//     _pimpl->append_named_dependency_entry_cmd->bind( ":release", edition.release() );
//     _pimpl->append_named_dependency_entry_cmd->bind( ":epoch", static_cast<int>( edition.epoch() ) );
//     _pimpl->append_named_dependency_entry_cmd->bind( ":relation", zypp_rel2db_rel( rel ) );
//     _pimpl->append_named_dependency_entry_cmd->executenonquery();
//     id = _pimpl->con.insertid();
//     return static_cast<data::RecordId>(id);
//   }
//   return static_cast<data::RecordId>(id);
// }

void CacheStore::appendNamedDependency( const data::RecordId &resolvable_id, zypp::Dep deptype, capability::NamedCap::Ptr cap )
{
  if ( !cap )
    ZYPP_THROW(Exception("bad versioned dep"));
  //DBG << "versioned : " << cap << endl;
 
  //data::RecordId capability_id = appendDependencyEntry( resolvable_id, deptype, cap->refers() );
  data::RecordId name_id = lookupOrAppendName(cap->name());
  
  _pimpl->append_named_dependency_cmd->bind( ":resolvable_id", resolvable_id );
  _pimpl->append_named_dependency_cmd->bind( ":dependency_type", zypp_deptype2db_deptype(deptype) );
  _pimpl->append_named_dependency_cmd->bind( ":refers_kind", zypp_kind2db_kind(cap->refers()) );
  
  //_pimpl->append_named_dependency_cmd->bind( ":capability_id", capability_id);
  _pimpl->append_named_dependency_cmd->bind( ":name_id", name_id);
  _pimpl->append_named_dependency_cmd->bind( ":version", cap->edition().version() );
  _pimpl->append_named_dependency_cmd->bind( ":release", cap->edition().release() );
  _pimpl->append_named_dependency_cmd->bind( ":epoch", static_cast<int>( cap->edition().epoch() ) );
  _pimpl->append_named_dependency_cmd->bind( ":relation", zypp_rel2db_rel( cap->op() ) );
  _pimpl->append_named_dependency_cmd->executenonquery();

  //delete cmd;
}

void CacheStore::appendFileDependency( const data::RecordId &resolvable_id, zypp::Dep deptype, capability::FileCap::Ptr cap )
{
  if ( !cap )
    ZYPP_THROW(Exception("bad file cap"));

  //data::RecordId capability_id = appendDependencyEntry( resolvable_id, deptype, cap->refers() );
  data::RecordId file_id = lookupOrAppendFile(cap->filename());
  
  _pimpl->append_file_dependency_cmd->bind( ":resolvable_id", resolvable_id );
  _pimpl->append_file_dependency_cmd->bind( ":dependency_type", zypp_deptype2db_deptype(deptype) );
  _pimpl->append_file_dependency_cmd->bind( ":refers_kind", zypp_kind2db_kind(cap->refers()) );
  
  //_pimpl->append_file_dependency_cmd->bind( ":capability_id", capability_id);
  _pimpl->append_file_dependency_cmd->bind( ":file_id", file_id);

  _pimpl->append_file_dependency_cmd->executenonquery();
  //delete cmd;
}

// data::RecordId CacheStore::appendDependencyEntry( const data::RecordId &resolvable_id, zypp::Dep deptype, const Resolvable::Kind &refers )
// {
//   //DBG << "rid: " << resolvable_id << " deptype: " << deptype << " " << "refers: " << refers << endl;
//   _pimpl->insert_dependency_entry_cmd->bind( ":resolvable_id", resolvable_id );
//   
//   db::DependencyType dt = zypp_deptype2db_deptype(deptype);
//   if ( dt == db::DEP_TYPE_UNKNOWN )
//   {
//     ZYPP_THROW(Exception("Unknown depenency type"));
//   }
// 
//   _pimpl->insert_dependency_entry_cmd->bind( ":dependency_type", zypp_deptype2db_deptype(deptype) );
//   _pimpl->insert_dependency_entry_cmd->bind( ":refers_kind", zypp_kind2db_kind(refers) );
//   
//   _pimpl->insert_dependency_entry_cmd->executenonquery();
//   //delete cmd;
//   long long id = _pimpl->con.insertid();
//   return static_cast<data::RecordId>(id);
// }

data::RecordId CacheStore::lookupOrAppendFile( const Pathname &path )
{
  data::RecordId dir_name_id = lookupOrAppendDirName(path.dirname().asString());
  data::RecordId file_name_id = lookupOrAppendFileName(path.basename());
  
  _pimpl->select_file_cmd->bind(":dir_name_id", dir_name_id);
  _pimpl->select_file_cmd->bind(":file_name_id", file_name_id);
  long long id = 0;
  try {
    id = _pimpl->select_file_cmd->executeint64();
  }
  catch ( const sqlite3x::database_error &e )
  {
    // does not exist
    _pimpl->insert_file_cmd->bind(":dir_name_id", dir_name_id);
    _pimpl->insert_file_cmd->bind(":file_name_id", file_name_id);
    _pimpl->insert_file_cmd->executenonquery();
    id = _pimpl->con.insertid();
    return static_cast<data::RecordId>(id);
    
  }
  return static_cast<data::RecordId>(id);
}

void CacheStore::updateCatalog( const data::RecordId &id, 
                    const std::string &checksum, 
                    const Date &timestamp )
{
  _pimpl->update_catalog_cmd->bind(":catalog_id", id);
  _pimpl->update_catalog_cmd->bind(":checksum", checksum);
  _pimpl->update_catalog_cmd->bind(":timestamp", static_cast<int>((Date::ValueType) timestamp) );
  _pimpl->insert_catalog_cmd->executenonquery();
}

data::RecordId CacheStore::lookupOrAppendCatalog( const Url &url, const Pathname &path )
{
  _pimpl->select_catalog_cmd->bind(":url", url.asString());
  _pimpl->select_catalog_cmd->bind(":path", path.asString());
  
  long long id = 0;
  try {
    id = _pimpl->select_catalog_cmd->executeint64();
  }
  catch ( const sqlite3x::database_error &e )
  {
    // does not exist
    _pimpl->insert_catalog_cmd->bind(":url", url.asString());
    _pimpl->insert_catalog_cmd->bind(":path", path.asString());
    _pimpl->insert_catalog_cmd->bind(":timestamp", static_cast<int>((Date::ValueType) Date::now()) );
    _pimpl->insert_catalog_cmd->executenonquery();
    id = _pimpl->con.insertid();
    return static_cast<data::RecordId>(id);
    
  }
  return static_cast<data::RecordId>(id);
}

data::RecordId CacheStore::lookupOrAppendName( const std::string &name )
{
  if ( _pimpl->name_cache.find(name) != _pimpl->name_cache.end() )
  {
    _pimpl->name_cache_hits++;
    return _pimpl->name_cache[name];
  }
  
  _pimpl->select_name_cmd->bind(":name", name);
  long long id = 0;
  try {
    id = _pimpl->select_name_cmd->executeint64();
    _pimpl->name_cache[name] = id;
  }
  catch ( const sqlite3x::database_error &e )
  {
    // does not exist
    _pimpl->insert_name_cmd->bind(":name", name);
    _pimpl->insert_name_cmd->executenonquery();
    id = _pimpl->con.insertid();
    return static_cast<data::RecordId>(id);
    
  }
  return static_cast<data::RecordId>(id);
}

data::RecordId CacheStore::lookupOrAppendDirName( const std::string &name )
{
  _pimpl->select_dirname_cmd->bind(":name", name);
  long long id = 0;
  try {
    id = _pimpl->select_dirname_cmd->executeint64();
  }
  catch ( const sqlite3x::database_error &e )
  {
    // does not exist
    _pimpl->insert_dirname_cmd->bind(":name", name);
    _pimpl->insert_dirname_cmd->executenonquery();
    id = _pimpl->con.insertid();
    return static_cast<data::RecordId>(id);
    
  }
  return static_cast<data::RecordId>(id);
}

data::RecordId CacheStore::lookupOrAppendFileName( const std::string &name )
{
  _pimpl->select_filename_cmd->bind(":name", name);
  long long id = 0;
  try {
    id = _pimpl->select_filename_cmd->executeint64();
  }
  catch ( const sqlite3x::database_error &e )
  {
    // does not exist
    _pimpl->insert_filename_cmd->bind(":name", name);
    _pimpl->insert_filename_cmd->executenonquery();
    id = _pimpl->con.insertid();
    return static_cast<data::RecordId>(id);
    
  }
  return static_cast<data::RecordId>(id);
}
    
// data::RecordId CacheStore::insertResObject( const Resolvable::Kind &kind, const data::ResObject &res )
// {
//   _insert_resolvable_cmd->bind(1,  res.name.c_str(), -1);
//   _insert_resolvable_cmd->bind(2,  res.edition.version().c_str(), -1);
//   _insert_resolvable_cmd->bind(3,  res.edition.release().c_str(), -1);
//   _insert_resolvable_cmd->bind(4,  static_cast<int>( res.edition.epoch() ));
//   _insert_resolvable_cmd->bind(5,  zypp_arch2db_arch(res.arch));
//   _insert_resolvable_cmd->bind(6,  db_kind2zypp_kind(kind));
//   _insert_resolvable_cmd->bind(7,  res.summary.text().c_str(), -1);
//   _insert_resolvable_cmd->bind(8,  res.description.text().c_str(), -1);
//   _insert_resolvable_cmd->bind(9,  res.insnotify.c_str(), -1);
//   _insert_resolvable_cmd->bind(10, res.delnotify.c_str(), -1);
//   _insert_resolvable_cmd->bind(11, res.license_to_confirm.c_str(), -1);
//   _insert_resolvable_cmd->bind(12, res.vendor.c_str(), -1);
//   _insert_resolvable_cmd->bind(13, res.size ); // FIX cast?
//   _insert_resolvable_cmd->bind(14, res.archive_size ); // FIX cast?
//   _insert_resolvable_cmd->bind(15, res.source.c_str(), -1);
//   
//   _insert_resolvable_cmd->bind(16, res.source_media_nr);
//   _insert_resolvable_cmd->bind(17, static_cast<int>(res.install_only));
//   _insert_resolvable_cmd->bind(18, static_cast<int>(res.build_time) ); // FIX cast?
//   _insert_resolvable_cmd->bind(19, static_cast<int>(res.install_time) ); // FIX cast?
//   _insert_resolvable_cmd->executenonquery();
// 
//   return static_cast<data::RecordId>( _con->insertid() );
// }


}
}

