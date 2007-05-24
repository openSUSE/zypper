
#include <sqlite3.h>
#include <map>
#include "zypp2/cache/sqlite3x/sqlite3x.hpp"

#include "zypp/base/Logger.h"
#include "zypp/base/Measure.h"
#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"

#include "zypp2/cache/CacheInitializer.h"
#include "zypp2/cache/CacheStore.h"

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
  Impl( const Pathname &dbdir )
  : name_cache_hits(0)
  {
    cache::CacheInitializer initializer(dbdir, "zypp.db");
    if ( initializer.justInitialized() )
    {
      MIL << "database " << (dbdir + "zypp.db") << " was just created" << endl;
    }

    try
    {
      con.open( (dbdir + "zypp.db").asString().c_str());
      //_insert_resolvable_cmd = new sqlite3_command( *_con, INSERT_RESOLVABLE_QUERY );
      //_insert_package_cmd = new sqlite3_command( *_con, INSERT_PACKAGE_QUERY );
    }
    catch(exception &ex)
    {
      //ZYPP_CAUGHT(ex);
      ZYPP_THROW(Exception(ex.what()));
    }


    // initialize all pre-compiled statements

    insert_resolvable_in_repository_cmd.reset( new sqlite3_command( con, "insert into resolvables_repositories (resolvable_id, repository_id) values (:resolvable_id, :repository_id);" ));

    update_repository_cmd.reset( new sqlite3_command( con, "update repositories set checksum=:checksum, timestamp=:timestamp where id=:repository_id;" ));

    select_repository_cmd.reset( new sqlite3_command( con, "select id from repositories where url=:url and path=:path;" ));
    insert_repository_cmd.reset( new sqlite3_command( con, "insert into repositories (url,path,timestamp) values (:url,:path,:timestamp);" ));

    select_name_cmd.reset( new sqlite3_command( con, "select id from names where name=:name;" ));
    insert_name_cmd.reset( new sqlite3_command( con, "insert into names (name) values (:name);" ));

    select_dirname_cmd.reset( new sqlite3_command( con, "select id from dir_names where name=:name;" ));
    insert_dirname_cmd.reset( new sqlite3_command( con, "insert into dir_names (name) values (:name);" ));

    select_filename_cmd.reset( new sqlite3_command( con, "select id from file_names where name=:name;" ));
    insert_filename_cmd.reset( new sqlite3_command( con, "insert into file_names (name) values (:name);" ));

    select_file_cmd.reset( new sqlite3_command( con, "select id from files where dir_name_id=:dir_name_id and file_name_id=:file_name_id;" ));
    insert_file_cmd.reset( new sqlite3_command( con, "insert into files (dir_name_id,file_name_id) values (:dir_name_id,:file_name_id);" ));

    select_type_cmd.reset( new sqlite3_command( con, "select id from types where class=:class and name=:name;" ));
    insert_type_cmd.reset( new sqlite3_command( con, "insert into types (class,name) values (:class,:name);" ));

    set_shared_flag_cmd.reset( new sqlite3_command( con, "update resolvables set shared_id=:shared_id where id=:resolvable_id;" ));

    append_text_attribute_cmd.reset( new sqlite3_command( con, "insert into text_attributes ( weak_resolvable_id, lang_id, attr_id, text ) values ( :rid, :lang_id, :attr_id, :text );" ));
    append_num_attribute_cmd.reset( new sqlite3_command( con, "insert into numeric_attributes ( weak_resolvable_id, attr_id, value ) values ( :rid, :attr_id, :value );" ));

    //insert_dependency_entry_cmd.reset( new sqlite3_command( con, "insert into capabilities ( resolvable_id, dependency_type, refers_kind ) values ( :resolvable_id, :dependency_type, :refers_kind );" ));
    append_file_dependency_cmd.reset( new sqlite3_command( con, "insert into file_capabilities ( resolvable_id, dependency_type, refers_kind, file_id ) values ( :resolvable_id, :dependency_type, :refers_kind, :file_id );" ));
    append_named_dependency_cmd.reset( new sqlite3_command( con, "insert into named_capabilities ( resolvable_id, dependency_type, refers_kind, name_id, version, release, epoch, relation ) values ( :resolvable_id, :dependency_type, :refers_kind, :name_id, :version, :release, :epoch, :relation );" ));

    append_modalias_dependency_cmd.reset( new sqlite3_command( con, "insert into modalias_capabilities ( resolvable_id, dependency_type, refers_kind, name, value, relation ) values ( :resolvable_id, :dependency_type, :refers_kind, :name, :value, :relation );" ));

    append_hal_dependency_cmd.reset( new sqlite3_command( con, "insert into hal_capabilities ( resolvable_id, dependency_type, refers_kind, name, value, relation ) values ( :resolvable_id, :dependency_type, :refers_kind, :name, :value, :relation );" ));

    append_other_dependency_cmd.reset( new sqlite3_command( con, "insert into other_capabilities ( resolvable_id, dependency_type, refers_kind, value ) values ( :resolvable_id, :dependency_type, :refers_kind, :value );" ));

    append_resolvable_cmd.reset( new sqlite3_command( con, "insert into resolvables ( name, version, release, epoch, arch, kind, repository_id ) values ( :name, :version, :release, :epoch, :arch, :kind, :repository_id );" ));

    count_shared_cmd.reset( new sqlite3_command( con, "select count(id) from resolvables where shared_id=:rid;" ));



    // disable autocommit
    con.executenonquery("BEGIN;");
  }

  Impl()
  {
    Impl( getZYpp()->homePath() );
  }

  ~Impl()
  {
    MIL << "name cache hits: " << name_cache_hits << " | cache size: " << name_cache.size() << endl;
  }

 /**
  * SQL statements
  * (we precompile them
  */
  sqlite3_connection con;

  sqlite3_command_ptr update_repository_cmd;
  sqlite3_command_ptr insert_resolvable_in_repository_cmd;

  sqlite3_command_ptr select_name_cmd;
  sqlite3_command_ptr insert_name_cmd;

  sqlite3_command_ptr select_dirname_cmd;
  sqlite3_command_ptr insert_dirname_cmd;

  sqlite3_command_ptr select_filename_cmd;
  sqlite3_command_ptr insert_filename_cmd;

  sqlite3_command_ptr select_repository_cmd;
  sqlite3_command_ptr insert_repository_cmd;

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
  sqlite3_command_ptr append_num_attribute_cmd;

  sqlite3_command_ptr set_shared_flag_cmd;

  sqlite3_command_ptr count_shared_cmd;

  map<string, RecordId> name_cache;
  map< pair<string,string>, RecordId> type_cache;
  int name_cache_hits;
};


CacheStore::CacheStore( const Pathname &dbdir )
  : _pimpl( new Impl(dbdir) )
{

}

CacheStore::CacheStore()
    : _pimpl( new Impl() )
{

}

CacheStore::~CacheStore()
{

}

void CacheStore::commit()
{
  _pimpl->con.executenonquery("COMMIT;");
}

void CacheStore::consumePackage( const RecordId &repository_id, data::Package_Ptr package )
{
  RecordId pkgid = appendResolvable( repository_id, ResTraits<Package>::kind, NVRA( package->name, package->edition, package->arch ), package->deps );
  consumeResObject( pkgid, package );

  appendStringAttribute( pkgid, "Package", "checksum", package->repositoryLocation.fileChecksum.checksum() );
  appendStringAttribute( pkgid, "Package", "buildhost", package->buildhost );
  appendStringAttribute( pkgid, "Package", "distribution", package->distribution );
  appendStringAttribute( pkgid, "Package", "license", package->license );
  appendStringAttribute( pkgid, "Package", "group", package->packager );
  appendStringAttribute( pkgid, "Package", "url", package->url );
  appendStringAttribute( pkgid, "Package", "operatingSystem", package->operatingSystem );
  appendStringAttribute( pkgid, "Package", "prein", package->prein );
  appendStringAttribute( pkgid, "Package", "postin", package->postin );
  appendStringAttribute( pkgid, "Package", "preun", package->preun );
  appendStringAttribute( pkgid, "Package", "postun", package->postun );

  //FIXME save authors and keyword (lists) for packages

  appendStringAttribute( pkgid, "Package", "location", package->repositoryLocation.filePath.asString() );
}

void CacheStore::consumeSourcePackage( const data::RecordId &catalog_id, data::SrcPackage_Ptr srcpackage )
{
#warning TBD
}

void CacheStore::consumePatch( const data::RecordId &catalog_id, data::Patch_Ptr patch)
{
  RecordId id = appendResolvable( repository_id, ResTraits<Patch>::kind, NVRA( patch->name, patch->edition, patch->arch ), patch->deps );
  consumeResObject( id, patch );

  DBG << "got patch " << patch->name << ", atoms: ";
  // cosume atoms
  for (set<data::ResObject_Ptr>::const_iterator p = patch->atoms.begin();
       p != patch->atoms.end(); ++p)
  {
    data::PackageAtom_Ptr atom = dynamic_pointer_cast<data::PackageAtom>(*p);
    if (atom)
    {
      DBG << atom->name << "(atom) ";
      consumePackageAtom(repository_id, atom);
      continue;
    }

    data::Script_Ptr script = dynamic_pointer_cast<data::Script>(*p);
    if (script)
    {
      DBG << script->name << "(script) ";
      consumeScript(repository_id, script);
      continue;
    }

    data::Message_Ptr message = dynamic_pointer_cast<data::Message>(*p);
    if (message)
    {
      DBG << message->name << "(message) ";
      consumeMessage(repository_id, message);
      continue;
    }

    ERR << " ignoring !badatom! ";
    if (*p) ERR << (*p)->name;
    ERR << endl;
  }

  DBG << endl;
}

void CacheStore::consumePackageAtom( const data::RecordId &repository_id, const data::PackageAtom_Ptr & atom )
{
  RecordId id = appendResolvable( repository_id, ResTraits<Atom>::kind, NVRA( atom->name, atom->edition, atom->arch ), atom->deps );
  consumeResObject( id, atom );
}

void CacheStore::consumeMessage( const data::RecordId &repository_id, data::Message_Ptr message )
{
  RecordId id = appendResolvable( repository_id, ResTraits<Message>::kind, NVRA( message->name, message->edition, message->arch ), message->deps );
  consumeResObject( id, message );
}

void CacheStore::consumeScript( const data::RecordId &repository_id, data::Script_Ptr script )
{
  RecordId id = appendResolvable( repository_id, ResTraits<Script>::kind, NVRA( script->name, script->edition, script->arch ), script->deps );
  consumeResObject( id, script );
}

void CacheStore::consumePattern( const data::RecordId &repository_id, data::Pattern_Ptr pattern )
{
  RecordId id = appendResolvable( repository_id, ResTraits<Pattern>::kind, NVRA( pattern->name, pattern->edition, pattern->arch ), pattern->deps );
  consumeResObject( id, pattern );
}

void CacheStore::consumeProduct( const data::RecordId &repository_id, data::Product_Ptr product )
{
  RecordId id = appendResolvable( repository_id, ResTraits<Product>::kind, NVRA( product->name, product->edition, product->arch ), product->deps );
  consumeResObject( id, product );
}

void CacheStore::consumeChangelog( const data::RecordId &repository_id, const data::Resolvable_Ptr & resolvable, const Changelog & changelog )
{
  // TODO
  // maybe consumeChangelog(const data::RecordId & resolvable_id, Changelog changelog) will
  // be needed for inserting the changelog using in-memory record id of corresponding
  // resolvable. (first, we'll see how fast is the inserting without remembering those ids)
}

void CacheStore::consumeFilelist( const data::RecordId &repository_id, const data::Resolvable_Ptr & resolvable, const data::Filenames & filenames )
{
  // TODO
  // maybe consumeFilelist(const data::RecordId & resolvable_id, data::Filenames &) will
  // be needed
}


void CacheStore::consumeResObject( const data::RecordId &rid, data::ResObject_Ptr res )
{
  appendTranslatedStringAttribute( rid, "ResObject", "description", res->description );
  appendTranslatedStringAttribute( rid, "ResObject", "summary", res->summary );
  appendNumericAttribute( rid, "ResObject", "installedSize", res->installedSize );
  appendNumericAttribute( rid, "ResObject", "buildTime", res->buildTime );
}

RecordId CacheStore::appendResolvable( const RecordId &repository_id,
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
  _pimpl->append_resolvable_cmd->bind( ":repository_id", repository_id );

  _pimpl->append_resolvable_cmd->executenonquery();

  long long id = _pimpl->con.insertid();

  appendDependencies( id, deps );
  /*
  _pimpl->insert_resolvable_in_repository_cmd->bind(":repository_id", repository_id);
  _pimpl->insert_resolvable_in_repository_cmd->bind(":resolvable_id", id);
  _pimpl->insert_resolvable_in_repository_cmd->executenonquery();*/

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
  _pimpl->update_repository_cmd->bind(":repository_id", id);
  _pimpl->update_repository_cmd->bind(":checksum", checksum);
  _pimpl->update_repository_cmd->bind(":timestamp", static_cast<int>((Date::ValueType) timestamp) );
  _pimpl->insert_repository_cmd->executenonquery();
}

RecordId CacheStore::lookupOrAppendCatalog( const Url &url, const Pathname &path )
{
  _pimpl->select_repository_cmd->bind(":url", url.asString());
  _pimpl->select_repository_cmd->bind(":path", path.asString());

  long long id = 0;
  try {
    id = _pimpl->select_repository_cmd->executeint64();
  }
  catch ( const sqlite3x::database_error &e )
  {
    // does not exist
    _pimpl->insert_repository_cmd->bind(":url", url.asString());
    _pimpl->insert_repository_cmd->bind(":path", path.asString());
    _pimpl->insert_repository_cmd->bind(":timestamp", static_cast<int>((Date::ValueType) Date::now()) );
    _pimpl->insert_repository_cmd->executenonquery();
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

void CacheStore::setSharedData( const data::RecordId &resolvable_id,
                                const data::RecordId &shared_id )
{
  _pimpl->set_shared_flag_cmd->bind(":resolvable_id", resolvable_id);

  if ( shared_id == data::noRecordId )
   _pimpl->set_shared_flag_cmd->bind(":shared_id");
  else
   _pimpl->set_shared_flag_cmd->bind(":shared_id", shared_id);

  _pimpl->set_shared_flag_cmd->executenonquery();
}

void CacheStore::appendNumericAttribute( const data::RecordId &resolvable_id,
                                         const std::string &klass,
                                         const std::string &name,
                                         int value )
{
  RecordId type_id = lookupOrAppendType( klass, name );
  appendNumericAttribute( resolvable_id, type_id, value );
}

void CacheStore::appendNumericAttribute( const RecordId &resolvable_id,
                                         const RecordId &type_id,
                                         int value )
{
  // weak resolvable_id
  _pimpl->append_num_attribute_cmd->bind(":rid", resolvable_id );
  _pimpl->append_num_attribute_cmd->bind(":attr_id", type_id );

  _pimpl->append_num_attribute_cmd->bind(":value", value );

  _pimpl->append_num_attribute_cmd->executenonquery();
}


void CacheStore::appendTranslatedStringAttribute( const data::RecordId &resolvable_id,
                                                  const std::string &klass,
                                                  const std::string &name,
                                                  const TranslatedText &text )
{
  set<Locale> locales = text.locales();
  for ( set<Locale>::const_iterator it = locales.begin(); it != locales.end(); ++it )
  {
    appendStringAttributeTranslation( resolvable_id, *it, klass, name, text.text(*it) );
  }
}


void CacheStore::appendStringAttributeTranslation( const data::RecordId &resolvable_id,
                                                   const Locale &locale,
                                                   const std::string &klass,
                                                   const std::string &name,
                                                   const std::string &text )
{
  RecordId lang_id = lookupOrAppendType("lang", locale.code() );
  RecordId type_id = lookupOrAppendType( klass, name );
  appendStringAttribute( resolvable_id, lang_id, type_id, text );
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
  _pimpl->append_text_attribute_cmd->bind(":attr_id", type_id );

  _pimpl->append_text_attribute_cmd->bind(":text", value );

  _pimpl->append_text_attribute_cmd->executenonquery();
}

}
}

