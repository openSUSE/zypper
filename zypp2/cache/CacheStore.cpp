
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
using zypp::data::RecordId;
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

  sqlite3_command_ptr select_type_cmd;
  sqlite3_command_ptr insert_type_cmd;
  
  //sqlite3_command_ptr insert_dependency_entry_cmd;

  sqlite3_command_ptr append_file_dependency_cmd;
  sqlite3_command_ptr append_named_dependency_cmd;
  sqlite3_command_ptr append_modalias_dependency_cmd;
  sqlite3_command_ptr append_hal_dependency_cmd;
  sqlite3_command_ptr append_other_dependency_cmd;

  sqlite3_command_ptr append_resolvable_cmd;

  sqlite3_command_ptr append_text_attribute_cmd;
  
  map<string, RecordId> name_cache;
  map< pair<string,string>, RecordId> type_cache;
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

  _pimpl->select_type_cmd.reset( new sqlite3_command( _pimpl->con, "select id from types where class=:class and name=:name;" ));
  _pimpl->insert_type_cmd.reset( new sqlite3_command( _pimpl->con, "insert into types (class,name) values (:class,:name);" ));

  _pimpl->append_text_attribute_cmd.reset( new sqlite3_command( _pimpl->con, "insert into text_attributes ( weak_resolvable_id, lang_id, attr_id, text ) values ( :rid, :lang_id, :attr_id, :text );" ));
  
  //_pimpl->insert_dependency_entry_cmd.reset( new sqlite3_command( _pimpl->con, "insert into capabilities ( resolvable_id, dependency_type, refers_kind ) values ( :resolvable_id, :dependency_type, :refers_kind );" ));
  _pimpl->append_file_dependency_cmd.reset( new sqlite3_command( _pimpl->con, "insert into file_capabilities ( resolvable_id, dependency_type, refers_kind, file_id ) values ( :resolvable_id, :dependency_type, :refers_kind, :file_id );" ));
  _pimpl->append_named_dependency_cmd.reset( new sqlite3_command( _pimpl->con, "insert into named_capabilities ( resolvable_id, dependency_type, refers_kind, name_id, version, release, epoch, relation ) values ( :resolvable_id, :dependency_type, :refers_kind, :name_id, :version, :release, :epoch, :relation );" ));

   _pimpl->append_modalias_dependency_cmd.reset( new sqlite3_command( _pimpl->con, "insert into modalias_capabilities ( resolvable_id, dependency_type, refers_kind, name, value, relation ) values ( :resolvable_id, :dependency_type, :refers_kind, :name, :value, :relation );" ));

   _pimpl->append_hal_dependency_cmd.reset( new sqlite3_command( _pimpl->con, "insert into hal_capabilities ( resolvable_id, dependency_type, refers_kind, name, value, relation ) values ( :resolvable_id, :dependency_type, :refers_kind, :name, :value, :relation );" ));

   _pimpl->append_other_dependency_cmd.reset( new sqlite3_command( _pimpl->con, "insert into other_capabilities ( resolvable_id, dependency_type, refers_kind, value ) values ( :resolvable_id, :dependency_type, :refers_kind, :value );" ));

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
  //RecordId pkgid = appendResolvable( ResTraits<Package>::kind, NVRA( package.name, package.edition, package.arch ), package.deps );
}

RecordId CacheStore::appendResolvable( const RecordId &catalog_id,
                                             const Resolvable::Kind &kind,
                                             const NVRA &nvra,
                                             const data::Dependencies &deps )
{
  _pimpl->append_resolvable_cmd->bind( ":name", nvra.name );
  _pimpl->append_resolvable_cmd->bind( ":version", nvra.edition.version() );
  _pimpl->append_resolvable_cmd->bind( ":release", nvra.edition.release() );
  _pimpl->append_resolvable_cmd->bind( ":epoch", static_cast<int>( nvra.edition.epoch() ) );
  _pimpl->append_resolvable_cmd->bind( ":arch", lookupOrAppendType("arch", nvra.arch.asString()) );
  _pimpl->append_resolvable_cmd->bind( ":kind", lookupOrAppendType("kind", kind.asString()) );
  _pimpl->append_resolvable_cmd->bind( ":catalog_id", catalog_id );

  _pimpl->append_resolvable_cmd->executenonquery();

  long long id = _pimpl->con.insertid();

  appendDependencies( id, deps );
  /*
  _pimpl->insert_resolvable_in_catalog_cmd->bind(":catalog_id", catalog_id);
  _pimpl->insert_resolvable_in_catalog_cmd->bind(":resolvable_id", id);
  _pimpl->insert_resolvable_in_catalog_cmd->executenonquery();*/

  return static_cast<RecordId>(id);
  return 1;
}

void CacheStore::appendDependencies( const RecordId &resolvable_id, const data::Dependencies &deps )
{
  for ( data::Dependencies::const_iterator it = deps.begin(); it != deps.end(); ++it )
  {
    appendDependencyList( resolvable_id, it->first, it->second );
  }
}

void CacheStore::appendDependencyList( const RecordId &resolvable_id, zypp::Dep deptype, const data::DependencyList &caps )
{
  for ( data::DependencyList::const_iterator it = caps.begin(); it != caps.end(); ++it )
  {
    appendDependency( resolvable_id, deptype, *it );
  }
}

void CacheStore::appendDependency( const RecordId &resolvable_id, zypp::Dep deptype, capability::CapabilityImpl::Ptr cap )
{
  if ( cap == 0 )
  {
    DBG << "invalid capability" << endl;
    return;
  }

  if ( capability::isKind<NamedCap>(cap) )
  {
      appendNamedDependency( resolvable_id, deptype, capability::asKind<NamedCap>(cap) );
  }
  else if ( capability::isKind<FileCap>(cap) )
  {
    appendFileDependency( resolvable_id, deptype, capability::asKind<FileCap>(cap) );
    return;
  }
  else if ( capability::isKind<ModaliasCap>(cap) )
  {
      appendModaliasDependency( resolvable_id, deptype, capability::asKind<ModaliasCap>(cap) );
  }
  else if ( capability::isKind<HalCap>(cap) )
  {
      appendHalDependency( resolvable_id, deptype, capability::asKind<HalCap>(cap) );
  }
  else
  {
      appendUnknownDependency( resolvable_id, deptype, cap );
  }
}

// RecordId CacheStore::lookupOrAppendNamedDependencyEntry( const RecordId name_id, const Edition &edition, const zypp::Rel &rel )
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
//     return static_cast<RecordId>(id);
//   }
//   return static_cast<RecordId>(id);
// }

void CacheStore::appendNamedDependency( const RecordId &resolvable_id, zypp::Dep deptype, capability::NamedCap::Ptr cap )
{
  if ( !cap )
    ZYPP_THROW(Exception("bad versioned dep"));
  //DBG << "versioned : " << cap << endl;

  //RecordId capability_id = appendDependencyEntry( resolvable_id, deptype, cap->refers() );
  RecordId name_id = lookupOrAppendName(cap->name());

  _pimpl->append_named_dependency_cmd->bind( ":resolvable_id", resolvable_id );
  _pimpl->append_named_dependency_cmd->bind( ":dependency_type", lookupOrAppendType("deptype", deptype.asString()) );
  _pimpl->append_named_dependency_cmd->bind( ":refers_kind", lookupOrAppendType("kind", cap->refers().asString()) );

  //_pimpl->append_named_dependency_cmd->bind( ":capability_id", capability_id);
  _pimpl->append_named_dependency_cmd->bind( ":name_id", name_id);
  _pimpl->append_named_dependency_cmd->bind( ":version", cap->edition().version() );
  _pimpl->append_named_dependency_cmd->bind( ":release", cap->edition().release() );
  _pimpl->append_named_dependency_cmd->bind( ":epoch", static_cast<int>( cap->edition().epoch() ) );
  _pimpl->append_named_dependency_cmd->bind( ":relation", lookupOrAppendType("rel", cap->op().asString()) );
  _pimpl->append_named_dependency_cmd->executenonquery();

  //delete cmd;
}

void CacheStore::appendModaliasDependency( const RecordId &resolvable_id,
                                                 zypp::Dep deptype,
                                                 capability::ModaliasCap::Ptr cap )
{
  if ( !cap )
    ZYPP_THROW(Exception("Null modalias capability"));

  _pimpl->append_modalias_dependency_cmd->bind( ":resolvable_id", resolvable_id );
  _pimpl->append_modalias_dependency_cmd->bind( ":dependency_type", lookupOrAppendType("deptype", deptype.asString()) );
  _pimpl->append_modalias_dependency_cmd->bind( ":refers_kind", lookupOrAppendType("kind", cap->refers().asString()) );

  //_pimpl->append_modalias_dependency_cmd->bind( ":capability_id", capability_id);
  _pimpl->append_modalias_dependency_cmd->bind( ":name", cap->name());
  _pimpl->append_modalias_dependency_cmd->bind( ":value", cap->value());
  _pimpl->append_modalias_dependency_cmd->bind( ":relation", lookupOrAppendType("rel", cap->op().asString()) );

  _pimpl->append_modalias_dependency_cmd->executenonquery();
  //delete cmd;
}

void CacheStore::appendHalDependency( const RecordId &resolvable_id,
                                                 zypp::Dep deptype,
                                                 capability::HalCap::Ptr cap )
{
  if ( !cap )
    ZYPP_THROW(Exception("Null HAL capability"));

  _pimpl->append_hal_dependency_cmd->bind( ":resolvable_id", resolvable_id );
  _pimpl->append_hal_dependency_cmd->bind( ":dependency_type", lookupOrAppendType("deptype", deptype.asString()) );
  _pimpl->append_hal_dependency_cmd->bind( ":refers_kind", lookupOrAppendType("kind", cap->refers().asString()) );

  //_pimpl->append_hal_dependency_cmd->bind( ":capability_id", capability_id);
  _pimpl->append_hal_dependency_cmd->bind( ":name", cap->name());
  _pimpl->append_hal_dependency_cmd->bind( ":value", cap->value());
  _pimpl->append_hal_dependency_cmd->bind( ":relation", lookupOrAppendType("rel", cap->op().asString()) );

  _pimpl->append_hal_dependency_cmd->executenonquery();
  //delete cmd;
}

void CacheStore::appendFileDependency( const RecordId &resolvable_id, zypp::Dep deptype,
                                       capability::FileCap::Ptr cap )
{
  if ( !cap )
    ZYPP_THROW(Exception("Null file capability"));

  //RecordId capability_id = appendDependencyEntry( resolvable_id, deptype, cap->refers() );
  RecordId file_id = lookupOrAppendFile(cap->filename());

  _pimpl->append_file_dependency_cmd->bind( ":resolvable_id", resolvable_id );
  _pimpl->append_file_dependency_cmd->bind( ":dependency_type", lookupOrAppendType("deptype", deptype.asString()) );
  _pimpl->append_file_dependency_cmd->bind( ":refers_kind", lookupOrAppendType("kind", cap->refers().asString()) );

  //_pimpl->append_file_dependency_cmd->bind( ":capability_id", capability_id);
  _pimpl->append_file_dependency_cmd->bind( ":file_id", file_id);

  _pimpl->append_file_dependency_cmd->executenonquery();
  //delete cmd;
}

void CacheStore::appendUnknownDependency( const RecordId &resolvable_id,
                                               zypp::Dep deptype,
                                               capability::CapabilityImpl::Ptr cap )
{
  if ( !cap )
    ZYPP_THROW(Exception("Null unknown capability"));

  _pimpl->append_other_dependency_cmd->bind( ":resolvable_id", resolvable_id );
  _pimpl->append_other_dependency_cmd->bind( ":dependency_type", lookupOrAppendType("deptype", deptype.asString()) );
  _pimpl->append_other_dependency_cmd->bind( ":refers_kind", lookupOrAppendType("kind", cap->refers().asString()) );
  _pimpl->append_other_dependency_cmd->bind( ":value", cap->encode());

  _pimpl->append_hal_dependency_cmd->executenonquery();
  //delete cmd;
}


// RecordId CacheStore::appendDependencyEntry( const RecordId &resolvable_id, zypp::Dep deptype, const Resolvable::Kind &refers )
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
//   return static_cast<RecordId>(id);
// }

RecordId CacheStore::lookupOrAppendFile( const Pathname &path )
{
  RecordId dir_name_id = lookupOrAppendDirName(path.dirname().asString());
  RecordId file_name_id = lookupOrAppendFileName(path.basename());

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
    return static_cast<RecordId>(id);

  }
  return static_cast<RecordId>(id);
}

void CacheStore::updateCatalog( const RecordId &id,
                    const string &checksum,
                    const Date &timestamp )
{
  _pimpl->update_catalog_cmd->bind(":catalog_id", id);
  _pimpl->update_catalog_cmd->bind(":checksum", checksum);
  _pimpl->update_catalog_cmd->bind(":timestamp", static_cast<int>((Date::ValueType) timestamp) );
  _pimpl->insert_catalog_cmd->executenonquery();
}

RecordId CacheStore::lookupOrAppendCatalog( const Url &url, const Pathname &path )
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
    return static_cast<RecordId>(id);

  }
  return static_cast<RecordId>(id);
}

RecordId CacheStore::lookupOrAppendType( const string &klass, const string &name )
{
  pair<string, string> thetype = make_pair(klass,name);
  if ( _pimpl->type_cache.find(thetype) != _pimpl->type_cache.end() )
  {
    //_pimpl->name_cache_hits++;
    return _pimpl->type_cache[thetype];
  }
  
  _pimpl->select_type_cmd->bind(":class", klass);
  _pimpl->select_type_cmd->bind(":name", name);
  long long id = 0;
  try {
    id = _pimpl->select_type_cmd->executeint64();
    _pimpl->type_cache[thetype] = id;
  }
  catch ( const sqlite3x::database_error &e )
  {
    // does not exist
    _pimpl->insert_type_cmd->bind(":class", klass);
    _pimpl->insert_type_cmd->bind(":name", name);
    _pimpl->insert_type_cmd->executenonquery();
    id = _pimpl->con.insertid();
    return id;
  }
  return id;
}

RecordId CacheStore::lookupOrAppendName( const string &name )
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
    return static_cast<RecordId>(id);

  }
  return static_cast<RecordId>(id);
}

RecordId CacheStore::lookupOrAppendDirName( const string &name )
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
    return static_cast<RecordId>(id);

  }
  return static_cast<RecordId>(id);
}

RecordId CacheStore::lookupOrAppendFileName( const string &name )
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
    return static_cast<RecordId>(id);

  }
  return static_cast<RecordId>(id);
}

void CacheStore::appendStringAttribute( const data::RecordId &resolvable_id,
                                        const TranslatedText &text )
{
  set<Locale> locales = text.locales();
  for ( set<Locale>::const_iterator it = locales.begin(); it != locales.end(); ++it )
  {
    appendStringAttribute( resolvable_id, *it, text.text(*it) );
  }
}


void CacheStore::appendStringAttribute( const data::RecordId &resolvable_id,
                                        const Locale &locale,
                                        const std::string &text )
{
  RecordId lang_id = lookupOrAppendType("lang", locale.code() );
  appendStringAttribute( resolvable_id, lang_id, text );
}

void CacheStore::appendStringAttribute( const data::RecordId &resolvable_id,
                                        const std::string &klass,
                                        const std::string &name,
                                        const std::string &value )
{
  RecordId type_id = lookupOrAppendType(klass, name);
  appendStringAttribute( resolvable_id, type_id, value );
}

void CacheStore::appendStringAttribute( const RecordId &resolvable_id,
                                        const RecordId &type_id,
                                        const std::string &value )
{
  RecordId lang_id = lookupOrAppendType("lang", "none");
  appendStringAttribute( resolvable_id, lang_id, type_id, value );
}

void CacheStore::appendStringAttribute( const RecordId &resolvable_id,
                            const RecordId &lang_id,
                            const RecordId &type_id,
                            const string &value )
{
  // weak resolvable_id
  _pimpl->append_text_attribute_cmd->bind(":rid", resolvable_id );
  _pimpl->append_text_attribute_cmd->bind(":lang_id", lang_id );
  _pimpl->append_text_attribute_cmd->bind(":type_id", type_id );
  
  _pimpl->append_text_attribute_cmd->bind(":value", value );
  
  _pimpl->append_text_attribute_cmd->executenonquery();
}

// RecordId CacheStore::insertResObject( const Resolvable::Kind &kind, const data::ResObject &res )
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
//   return static_cast<RecordId>( _con->insertid() );
// }


}
}

