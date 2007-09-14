#include <sqlite3.h>
#include <map>
#include "zypp/cache/sqlite3x/sqlite3x.hpp"

#include "zypp/base/Easy.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/Measure.h"
#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"
#include "zypp/Package.h"
#include "zypp/cache/CacheInitializer.h"
#include "zypp/cache/CacheStore.h"
#include "zypp/cache/CacheException.h"
#include "zypp/cache/CacheAttributes.h"

using namespace std;
using namespace zypp;
using namespace zypp::capability;
using namespace zypp::cache;
using zypp::data::RecordId;
using namespace sqlite3x;

using zypp::debug::Measure;

/** Append OnMediaLocation attributes to resolvable with ID.
 * \code
 * appendOnMediaLocation( pkgid, attrPackageLocation, package->repositoryLocation );
 * \endcode
 * Pass the OnMediaLocation attributes common prefix as 2nd arg, the OnMediaLocation
 * object as third arg. This macro assumes that the attribute names follow this schema:
*/
#define appendOnMediaLocation(ID,OMLATTRPREFIX,OML)                                              \
do {                                                                                             \
  appendNumericAttribute( ID, OMLATTRPREFIX##MediaNr(),          OML.medianr() );                \
  appendStringAttribute ( ID, OMLATTRPREFIX##Filename(),         OML.filename().asString() );    \
  appendNumericAttribute( ID, OMLATTRPREFIX##DownloadSize(),     OML.downloadSize() );           \
  appendStringAttribute ( ID, OMLATTRPREFIX##ChecksumType(),     OML.checksum().type() );        \
  appendStringAttribute ( ID, OMLATTRPREFIX##Checksum(),         OML.checksum().checksum() );    \
  appendNumericAttribute( ID, OMLATTRPREFIX##OpenSize(),         OML.openSize() );               \
  appendStringAttribute ( ID, OMLATTRPREFIX##OpenChecksumType(), OML.openChecksum().type() );    \
  appendStringAttribute ( ID, OMLATTRPREFIX##OpenChecksum(),     OML.openChecksum().checksum() );\
} while(false)

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
  , dir_cache_hits(0)
  {
    // let the initializer go out of scope after it executes
    {
      cache::CacheInitializer initializer(dbdir, "zypp.db");
      if ( initializer.justInitialized() )
      {
        MIL << "database " << (dbdir + "zypp.db") << " was just created" << endl;
      }
    }

    try
    {
      con.open( (dbdir + "zypp.db").asString().c_str());
    }
    catch(exception &ex)
    {
      //ZYPP_CAUGHT(ex);
      ZYPP_THROW(Exception(ex.what()));
    }


    // initialize all pre-compiled statements

    insert_resolvable_in_repository_cmd.reset( new sqlite3_command( con, "insert into resolvables_repositories (resolvable_id, repository_id) values (:resolvable_id, :repository_id);" ));

    select_repository_cmd.reset( new sqlite3_command( con, "select id from repositories where alias=:alias;" ));
    insert_repository_cmd.reset( new sqlite3_command( con, "insert into repositories (alias,timestamp) values (:alias, :timestamp);" ));

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

    append_modalias_dependency_cmd.reset( new sqlite3_command( con, "insert into modalias_capabilities ( resolvable_id, dependency_type, refers_kind, name, pkgname, value, relation ) values ( :resolvable_id, :dependency_type, :refers_kind, :name, :pkgname, :value, :relation );" ));

    append_hal_dependency_cmd.reset( new sqlite3_command( con, "insert into hal_capabilities ( resolvable_id, dependency_type, refers_kind, name, value, relation ) values ( :resolvable_id, :dependency_type, :refers_kind, :name, :value, :relation );" ));

    append_filesystem_dependency_cmd.reset( new sqlite3_command( con, "insert into filesystem_capabilities ( resolvable_id, dependency_type, refers_kind, name_id ) values ( :resolvable_id, :dependency_type, :refers_kind, :name_id );" ));

    append_split_dependency_cmd.reset( new sqlite3_command( con, "insert into split_capabilities ( resolvable_id, dependency_type, refers_kind, name_id, file_id ) values ( :resolvable_id, :dependency_type, :refers_kind, :name_id, :file_id );" ));

    append_other_dependency_cmd.reset( new sqlite3_command( con, "insert into other_capabilities ( resolvable_id, dependency_type, refers_kind, value ) values ( :resolvable_id, :dependency_type, :refers_kind, :value );" ));

    append_resolvable_cmd.reset( new sqlite3_command( con, "insert into resolvables ( name, version, release, epoch, arch, kind, shared_id, repository_id ) values ( :name, :version, :release, :epoch, :arch, :kind, :shared_id, :repository_id );" ));

    count_shared_cmd.reset( new sqlite3_command( con, "select count(id) from resolvables where shared_id=:rid;" ));

    insert_patchrpm_cmd.reset( new sqlite3_command (con,
      "insert into patch_packages (repository_id, name, version, release, epoch, arch, media_nr, location, checksum, checksum_type, download_size, build_time) "
      "values (:repository_id, :name, :version, :release, :epoch, :arch, :media_nr, :location, :checksum, :checksum_type, :download_size, :build_time);" ));
    insert_deltarpm_cmd.reset( new sqlite3_command (con,
      "insert into delta_packages (repository_id, name, version, release, epoch, arch, media_nr, location, checksum, checksum_type, download_size, build_time, "
        "baseversion_version, baseversion_release, baseversion_epoch, baseversion_checksum, "
        "baseversion_build_time, baseversion_sequence_info) "
      "values (:repository_id, :name, :version, :release, :epoch, :arch, :media_nr, :location, :checksum, :checksum_type, :download_size, :build_time, "
        ":baseversion_version, :baseversion_release, :baseversion_epoch, :baseversion_checksum, "
        ":baseversion_build_time, :baseversion_sequence_info);" ));
    append_patch_baseversion_cmd.reset( new sqlite3_command (con,
      "insert into patch_packages_baseversions (patch_package_id, version, release, epoch) "
      "values (:patch_package_id, :version, :release, :epoch)" ));

    update_disk_usage_cmd.reset( new sqlite3_command (con,
      "insert or replace into resolvable_disk_usage (resolvable_id, dir_name_id, files, size) "
      "values (:resolvable_id, :dir_name_id, :files, :size)" ));


    // disable autocommit
    con.executenonquery("PRAGMA synchronous = 0;");
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
  sqlite3_command_ptr append_filesystem_dependency_cmd;
  sqlite3_command_ptr append_split_dependency_cmd;
  sqlite3_command_ptr append_other_dependency_cmd;

  sqlite3_command_ptr append_resolvable_cmd;

  sqlite3_command_ptr append_text_attribute_cmd;
  sqlite3_command_ptr append_num_attribute_cmd;

  sqlite3_command_ptr set_shared_flag_cmd;

  sqlite3_command_ptr count_shared_cmd;

  sqlite3_command_ptr insert_patchrpm_cmd;
  sqlite3_command_ptr insert_deltarpm_cmd;
  sqlite3_command_ptr append_patch_baseversion_cmd;

  sqlite3_command_ptr update_disk_usage_cmd;

  map<string, RecordId> name_cache;
  map< pair<string,string>, RecordId> type_cache;
  map<string, RecordId> dir_cache;
  int name_cache_hits;
  int dir_cache_hits;
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

void CacheStore::appendResObjectAttributes( const data::RecordId &rid,
                                            const data::ResObject_Ptr & res )
{
  appendTranslatedStringAttribute( rid, attrResObjectDescription(), res->description );
  appendTranslatedStringAttribute( rid, attrResObjectSummary(), res->summary );
  appendNumericAttribute( rid, attrResObjectInstalledSize(), res->installedSize );
  appendNumericAttribute( rid, attrResObjectBuildTime(), res->buildTime );
  appendBooleanAttribute( rid, attrResObjectInstallOnly(), res->installOnly );
  appendStringAttribute( rid, attrResObjectVendor(), res->vendor );
  appendTranslatedStringAttribute( rid, attrResObjectLicenseToConfirm(), res->licenseToConfirm );
  appendTranslatedStringAttribute( rid, attrResObjectInsnotify(), res->insnotify );
  appendTranslatedStringAttribute( rid, attrResObjectDelnotify(), res->delnotify );
}


void CacheStore::appendPackageBaseAttributes( const RecordId & pkgid,
                                              const data::Packagebase_Ptr & package )
{
  appendStringAttribute( pkgid, attrPackageBuildhost(), package->buildhost );
  appendStringAttribute( pkgid, attrPackageDistribution(), package->distribution );
  appendStringAttribute( pkgid, attrPackageLicense(), package->license );
  appendStringAttribute( pkgid, attrPackageGroup(), package->group );
  appendStringAttribute( pkgid, attrPackagePackager(), package->packager );
  appendStringAttribute( pkgid, attrPackageUrl(), package->url );
  appendStringAttribute( pkgid, attrPackageOperatingSystem(), package->operatingSystem );
  appendStringAttribute( pkgid, attrPackagePrein(), package->prein );
  appendStringAttribute( pkgid, attrPackagePostin(), package->postin );
  appendStringAttribute( pkgid, attrPackagePreun(), package->preun );
  appendStringAttribute( pkgid, attrPackagePostun(), package->postun );
  appendStringContainerAttribute( pkgid, attrPackageKeywords(), package->keywords.begin(), package->keywords.end() );
  appendStringContainerAttribute( pkgid, attrPackageAuthors(), package->authors.begin(), package->authors.end() );

  appendOnMediaLocation( pkgid, attrPackageLocation, package->repositoryLocation );
}

RecordId CacheStore::consumePackage( const RecordId & repository_id,
				     const data::Package_Ptr & package )
{
  RecordId id = appendResolvable( repository_id, ResTraits<Package>::kind,
      NVRA( package->name, package->edition, package->arch ), package->deps, package->shareDataWith );
  appendResObjectAttributes( id, package );
  appendPackageBaseAttributes( id, package );

  if ( ! package->srcPackageIdent.name.empty() )
  {
    appendStringAttribute( id, attrPackageSourcePkgName(),    package->srcPackageIdent.name );
    appendStringAttribute( id, attrPackageSourcePkgEdition(), package->srcPackageIdent.edition.asString() );
  }

  return id;
}

RecordId CacheStore::consumeSourcePackage( const data::RecordId & repository_id,
                                       const data::SrcPackage_Ptr & package )
{
  RecordId id = appendResolvable( repository_id, ResTraits<SrcPackage>::kind,
      NVRA( package->name, package->edition, package->arch ), package->deps, package->shareDataWith );
  appendResObjectAttributes( id, package );

  appendOnMediaLocation( id, attrSrcPackageLocation, package->repositoryLocation );
  return id;
}

RecordId CacheStore::consumePatch( const data::RecordId & repository_id,
                               const data::Patch_Ptr & patch)
{
  RecordId id = appendResolvable(
      repository_id, ResTraits<Patch>::kind,
      NVRA( patch->name, patch->edition, patch->arch ), patch->deps );

  appendResObjectAttributes( id, patch );

  // patch attributes
  appendNumericAttribute( id, attrPatchTimestamp(),         patch->timestamp );
  appendStringAttribute(  id, attrPatchCategory(),          patch->category );
  appendStringAttribute(  id, attrPatchId(),                patch->id );
  appendBooleanAttribute( id, attrPatchRebootNeeded(),      patch->rebootNeeded );
  appendBooleanAttribute( id, attrPatchAffectsPkgManager(), patch->affectsPkgManager );


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
  return id;
}

RecordId CacheStore::consumePackageAtom( const data::RecordId & repository_id,
                                     const data::PackageAtom_Ptr & atom )
{
  RecordId id = appendResolvable( repository_id, ResTraits<Atom>::kind,
      NVRA( atom->name, atom->edition, atom->arch ), atom->deps );
  appendResObjectAttributes( id, atom );
  appendPackageBaseAttributes( id, atom );

  for (set<data::PatchRpm_Ptr>::const_iterator p = atom->patchRpms.begin();
       p != atom->patchRpms.end(); ++p)
    appendPatchRpm(repository_id, *p);

  for (set<data::DeltaRpm_Ptr>::const_iterator d = atom->deltaRpms.begin();
       d != atom->deltaRpms.end(); ++d)
    appendDeltaRpm(repository_id, *d);
  return id;
}

RecordId CacheStore::consumeMessage( const data::RecordId & repository_id,
                                 const data::Message_Ptr & message )
{
  RecordId id = appendResolvable( repository_id, ResTraits<Message>::kind,
      NVRA( message->name, message->edition, message->arch ), message->deps );
  appendResObjectAttributes( id, message );

  appendTranslatedStringAttribute( id, attrMessageText(), message->text );
  return id;
}

RecordId CacheStore::consumeScript( const data::RecordId & repository_id,
                                const data::Script_Ptr & script )
{
  RecordId id = appendResolvable( repository_id, ResTraits<Script>::kind,
      NVRA( script->name, script->edition, script->arch ), script->deps );
  appendResObjectAttributes( id, script );

  appendStringAttribute( id, attrScriptDoScript(), script->doScript );
  appendOnMediaLocation( id, attrScriptDoScriptLocation, script->doScriptLocation );
  appendStringAttribute( id, attrScriptUndoScript(), script->undoScript );
  appendOnMediaLocation( id, attrScriptUndoScriptLocation, script->undoScriptLocation );
  return id;
}

RecordId CacheStore::consumePattern( const data::RecordId & repository_id,
                                     const data::Pattern_Ptr & pattern )
{
  RecordId id = appendResolvable( repository_id, ResTraits<Pattern>::kind,
      NVRA( pattern->name, pattern->edition, pattern->arch ), pattern->deps );
  appendResObjectAttributes( id, pattern );

  appendBooleanAttribute( id, attrPatternIsDefault(), pattern->isDefault );
  appendBooleanAttribute( id, attrPatternUserVisible(), pattern->userVisible );
  appendTranslatedStringAttribute( id, attrPatternCategory(), pattern->category );
  appendStringAttribute( id, attrPatternIcon(), pattern->icon );
  appendStringAttribute( id, attrPatternOrder(), pattern->order );

  // We store them as string. They are
  // (sometimes) evaluated by the YaST UI.
  appendStringContainerAttribute( id, attrPatternUiIncludes(), pattern->includes.begin(), pattern->includes.end() );
  appendStringContainerAttribute( id, attrPatternUiExtends(),  pattern->extends.begin(),  pattern->extends.end() );

  return id;
}

RecordId CacheStore::consumeProduct( const data::RecordId & repository_id,
                                 const data::Product_Ptr & product )
{
  RecordId id = appendResolvable( repository_id, ResTraits<Product>::kind,
      NVRA( product->name, product->edition, product->arch ), product->deps );
  appendResObjectAttributes( id, product );

  appendStringAttribute( id, attrProductType(), product->type );
  appendTranslatedStringAttribute( id, attrProductShortName(), product->shortName );
  appendTranslatedStringAttribute( id, attrProductLongName(), product->longName );
  appendStringContainerAttribute( id, attrProductFlags(), product->flags.begin(), product->flags.end() );
  appendStringAttribute( id, attrProductReleasenotesUrl(), product->releasenotesUrl.asString() );
  appendStringContainerAttribute( id, attrProductUpdateUrls(), product->updateUrls );
  appendStringContainerAttribute( id, attrProductExtraUrls(), product->extraUrls );
  appendStringContainerAttribute( id, attrProductOptionalUrls(), product->optionalUrls );
  appendStringAttribute( id, attrProductDistributionName(), product->distributionName );
  appendStringAttribute( id, attrProductDistributionEdition(), product->distributionEdition.asString() );
  return id;
}

RecordId CacheStore::consumeChangelog( const data::RecordId &resolvable_id,
                                   const Changelog & changelog )
{
  //! \todo maybe appendChangelog(const data::RecordId & resolvable_id, Changelog changelog) will be needed
  //! for inserting the changelog using in-memory record id of corresponding resolvable.
  //! (first, we'll see how fast is the inserting without remembering those ids)
  return data::noRecordId;
}

RecordId CacheStore::consumeFilelist( const data::RecordId &resolvable_id,
                                  const data::Filenames & filenames )
{
  //! \todo maybe consumeFilelist(const data::RecordId & resolvable_id, data::Filenames &) will be needed
  return data::noRecordId;
}

void CacheStore::consumeDiskUsage( const data::RecordId &resolvable_id,
                                  const DiskUsage &disk )
{
  // iterate over entries
  for ( DiskUsage::const_iterator it = disk.begin();
        it != disk.end();
        ++it )
  {
    data::RecordId dirid = lookupOrAppendDirName( (*it).path );
    try
    {
      _pimpl->update_disk_usage_cmd->bind(":resolvable_id", resolvable_id);
      _pimpl->update_disk_usage_cmd->bind(":dir_name_id", dirid);
      _pimpl->update_disk_usage_cmd->bind(":files", (int)(*it)._files );
      _pimpl->update_disk_usage_cmd->bind(":size", (int)(*it)._size );
      _pimpl->update_disk_usage_cmd->executenonquery();
    }
    catch ( const sqlite3x::database_error &e )
    {
      ZYPP_RETHROW(e);
    }
  }
  //MIL << "disk usage for " << resolvable_id << " consumed" << endl;
}

void CacheStore::updatePackageLang( const data::RecordId & resolvable_id,
				    const data::Packagebase_Ptr & data_r )
{
  appendTranslatedStringAttribute( resolvable_id, attrResObjectSummary(),          data_r->summary );
  appendTranslatedStringAttribute( resolvable_id, attrResObjectDescription(),      data_r->description );
  appendTranslatedStringAttribute( resolvable_id, attrResObjectLicenseToConfirm(), data_r->licenseToConfirm );
  appendTranslatedStringAttribute( resolvable_id, attrResObjectInsnotify(),        data_r->insnotify );
  appendTranslatedStringAttribute( resolvable_id, attrResObjectDelnotify(),        data_r->delnotify );
}

RecordId CacheStore::appendResolvable( const RecordId &repository_id,
                                       const Resolvable::Kind &kind,
                                       const NVRA &nvra,
                                       const data::Dependencies &deps )
{
  return appendResolvable( repository_id,
                           kind,
                           nvra,
                           deps,
                           data::noRecordId );
}

data::RecordId
    CacheStore::appendResolvable( const data::RecordId &repository_id,
                                  const Resolvable::Kind &kind,
                                  const NVRA &nvra,
                                  const data::Dependencies &deps,
                                  const data::RecordId &shared_id )
{
  _pimpl->append_resolvable_cmd->bind( ":name", nvra.name );
  _pimpl->append_resolvable_cmd->bind( ":version", nvra.edition.version() );
  _pimpl->append_resolvable_cmd->bind( ":release", nvra.edition.release() );
  _pimpl->append_resolvable_cmd->bind( ":epoch", static_cast<int>( nvra.edition.epoch() ) );
  _pimpl->append_resolvable_cmd->bind( ":arch", lookupOrAppendType("arch", nvra.arch.asString()) );
  _pimpl->append_resolvable_cmd->bind( ":kind", lookupOrAppendType("kind", kind.asString()) );
  _pimpl->append_resolvable_cmd->bind( ":repository_id", repository_id );

  if ( shared_id == data::noRecordId )
    _pimpl->append_resolvable_cmd->bind(":shared_id");
  else
    _pimpl->append_resolvable_cmd->bind( ":shared_id", shared_id );

  _pimpl->append_resolvable_cmd->executenonquery();

  long long id = _pimpl->con.insertid();

  appendDependencies( id, deps );
  /*
  _pimpl->insert_resolvable_in_repository_cmd->bind(":repository_id", repository_id);
  _pimpl->insert_resolvable_in_repository_cmd->bind(":resolvable_id", id);
  _pimpl->insert_resolvable_in_repository_cmd->executenonquery();*/

  return id;
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
  else if ( capability::isKind<FilesystemCap>(cap) )
  {
      appendFilesystemDependency( resolvable_id, deptype, capability::asKind<FilesystemCap>(cap) );
  }
  else if ( capability::isKind<SplitCap>(cap) )
  {
      appendSplitDependency( resolvable_id, deptype, capability::asKind<SplitCap>(cap) );
  }
  else
  {
      appendUnknownDependency( resolvable_id, deptype, cap );
  }
}


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
  _pimpl->append_modalias_dependency_cmd->bind( ":pkgname", cap->pkgname());
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

  MIL << "HAL cap [ rid: " << resolvable_id << " | deptype: " << deptype << " | cap: "
    << cap << endl;

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

void CacheStore::appendFilesystemDependency( const data::RecordId &resolvable_id,
                                             zypp::Dep deptype,
                                             capability::FilesystemCap::Ptr cap )
{
  if ( !cap )
    ZYPP_THROW(Exception("bad versioned dep"));
  //DBG << "versioned : " << cap << endl;

  //RecordId capability_id = appendDependencyEntry( resolvable_id, deptype, cap->refers() );
  RecordId name_id = lookupOrAppendName(cap->name());

  _pimpl->append_filesystem_dependency_cmd->bind( ":resolvable_id", resolvable_id );
  _pimpl->append_filesystem_dependency_cmd->bind( ":dependency_type", lookupOrAppendType("deptype", deptype.asString()) );
  _pimpl->append_filesystem_dependency_cmd->bind( ":refers_kind", lookupOrAppendType("kind", cap->refers().asString()) );

  //_pimpl->append_filesystem_dependency_cmd->bind( ":capability_id", capability_id);
  _pimpl->append_filesystem_dependency_cmd->bind( ":name_id", name_id);
  _pimpl->append_filesystem_dependency_cmd->executenonquery();
}


void CacheStore::appendSplitDependency( const data::RecordId &resolvable_id,
                                        zypp::Dep deptype,
                                        capability::SplitCap::Ptr cap )
{
  if ( !cap )
    ZYPP_THROW(Exception("bad versioned dep"));
  //DBG << "versioned : " << cap << endl;

  //RecordId capability_id = appendDependencyEntry( resolvable_id, deptype, cap->refers() );
  RecordId name_id = lookupOrAppendName(cap->name());
  RecordId file_id = lookupOrAppendFile(cap->path());

  _pimpl->append_split_dependency_cmd->bind( ":resolvable_id", resolvable_id );
  _pimpl->append_split_dependency_cmd->bind( ":dependency_type", lookupOrAppendType("deptype", deptype.asString()) );
  _pimpl->append_split_dependency_cmd->bind( ":refers_kind", lookupOrAppendType("kind", cap->refers().asString()) );

  //_pimpl->append_split_dependency_cmd->bind( ":capability_id", capability_id);
  _pimpl->append_split_dependency_cmd->bind( ":name_id", name_id);
  _pimpl->append_split_dependency_cmd->bind( ":file_id", file_id);
  _pimpl->append_split_dependency_cmd->executenonquery();
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

  _pimpl->append_other_dependency_cmd->executenonquery();
  //delete cmd;
}


/** \todo lookupOrAppend ? */
RecordId CacheStore::appendPatchRpm(const zypp::data::RecordId &repository_id, const data::PatchRpm_Ptr & prpm)
{
  RecordId id;

  _pimpl->insert_patchrpm_cmd->bind(":repository_id", repository_id);

  _pimpl->insert_patchrpm_cmd->bind(":name",    prpm->name);
  _pimpl->insert_patchrpm_cmd->bind(":version", prpm->edition.version());
  _pimpl->insert_patchrpm_cmd->bind(":release", prpm->edition.release());
  _pimpl->insert_patchrpm_cmd->bind(":epoch",   (int) prpm->edition.epoch());
  _pimpl->insert_patchrpm_cmd->bind(":arch",    lookupOrAppendType("arch", prpm->arch.asString()));

  _pimpl->insert_patchrpm_cmd->bind(":media_nr", (int) prpm->location.medianr());
  _pimpl->insert_patchrpm_cmd->bind(":location", prpm->location.filename().asString());
  _pimpl->insert_patchrpm_cmd->bind(":checksum", prpm->location.checksum().checksum());
  _pimpl->insert_patchrpm_cmd->bind(":checksum_type", prpm->location.checksum().type());
  _pimpl->insert_patchrpm_cmd->bind(":download_size", static_cast<ByteCount::SizeType>(prpm->location.downloadSize()));
  _pimpl->insert_patchrpm_cmd->bind(":build_time", prpm->buildTime.asSeconds());
  _pimpl->insert_patchrpm_cmd->executenonquery();

  id = _pimpl->con.insertid();

  for (set<data::BaseVersion_Ptr>::const_iterator bv = prpm->baseVersions.begin();
       bv != prpm->baseVersions.end(); ++bv)
  {
    _pimpl->append_patch_baseversion_cmd->bind(":patch_package_id", id);
    _pimpl->append_patch_baseversion_cmd->bind(":version", (*bv)->edition.version());
    _pimpl->append_patch_baseversion_cmd->bind(":release", (*bv)->edition.release());
    _pimpl->append_patch_baseversion_cmd->bind(":epoch", (int) (*bv)->edition.epoch());
    _pimpl->append_patch_baseversion_cmd->executenonquery();
  }

  return id;
}


/** \todo lookupOrAppend ? */
RecordId CacheStore::appendDeltaRpm(const zypp::data::RecordId &repository_id, const data::DeltaRpm_Ptr & drpm)
{
  RecordId id;

  _pimpl->insert_deltarpm_cmd->bind(":repository_id", repository_id);

  _pimpl->insert_deltarpm_cmd->bind(":name",    drpm->name);
  _pimpl->insert_deltarpm_cmd->bind(":version", drpm->edition.version());
  _pimpl->insert_deltarpm_cmd->bind(":release", drpm->edition.release());
  _pimpl->insert_deltarpm_cmd->bind(":epoch",   (int) drpm->edition.epoch());
  _pimpl->insert_deltarpm_cmd->bind(":arch",    lookupOrAppendType("arch", drpm->arch.asString()));

  _pimpl->insert_deltarpm_cmd->bind(":media_nr", (int) drpm->location.medianr());
  _pimpl->insert_deltarpm_cmd->bind(":location", drpm->location.filename().asString());
  _pimpl->insert_deltarpm_cmd->bind(":checksum", drpm->location.checksum().checksum());
  _pimpl->insert_deltarpm_cmd->bind(":checksum", drpm->location.checksum().checksum());
  _pimpl->insert_deltarpm_cmd->bind(":checksum_type", drpm->location.checksum().type());
  _pimpl->insert_deltarpm_cmd->bind(":download_size", static_cast<ByteCount::SizeType>(drpm->location.downloadSize()));
  _pimpl->insert_deltarpm_cmd->bind(":build_time", drpm->buildTime.asSeconds());

  _pimpl->insert_deltarpm_cmd->bind(":baseversion_version", drpm->baseVersion.edition.version());
  _pimpl->insert_deltarpm_cmd->bind(":baseversion_release", drpm->baseVersion.edition.release());
  _pimpl->insert_deltarpm_cmd->bind(":baseversion_epoch", (int) drpm->baseVersion.edition.epoch());
  _pimpl->insert_deltarpm_cmd->bind(":baseversion_build_time", drpm->baseVersion.buildTime.asSeconds());
  _pimpl->insert_deltarpm_cmd->bind(":baseversion_checksum", drpm->baseVersion.checkSum.checksum());
  _pimpl->insert_deltarpm_cmd->bind(":baseversion_sequence_info", drpm->baseVersion.sequenceInfo);

  _pimpl->insert_deltarpm_cmd->executenonquery();
  id = _pimpl->con.insertid();

  return id;
}


RecordId CacheStore::lookupOrAppendFile( const Pathname &path )
{
  RecordId dir_name_id = lookupOrAppendDirName(path.dirname().asString());
  RecordId file_name_id = lookupOrAppendFileName(path.basename());

  _pimpl->select_file_cmd->bind(":dir_name_id", dir_name_id);
  _pimpl->select_file_cmd->bind(":file_name_id", file_name_id);
  long long id = 0;

  try
  {
    sqlite3_reader reader= _pimpl->select_file_cmd->executereader();
    if (!reader.read())
    {
      // does not exist
      _pimpl->insert_file_cmd->bind(":dir_name_id", dir_name_id);
      _pimpl->insert_file_cmd->bind(":file_name_id", file_name_id);
      _pimpl->insert_file_cmd->executenonquery();
      id = _pimpl->con.insertid();
      return id;
   }
   return reader.getint64(0);
  }
  catch ( const sqlite3x::database_error &e )
  {
    ZYPP_RETHROW(e);
  }
  return id;
}

void CacheStore::updateRepositoryStatus( const RecordId &id,
                                         const RepoStatus &status )
{
  sqlite3_command cmd( _pimpl->con, "update repositories set checksum=:checksum, timestamp=:timestamp where id=:repository_id;");
  cmd.bind(":repository_id", id);
  cmd.bind(":checksum", status.checksum());
  cmd.bind(":timestamp", static_cast<int>((Date::ValueType) status.timestamp()) );
  try
  {
    cmd.executenonquery();
  }
  catch ( const sqlite3x::database_error &e )
  {
    ZYPP_RETHROW(e);
  }
}

RecordId CacheStore::lookupOrAppendRepository( const string &alias )
{
  _pimpl->select_repository_cmd->bind(":alias", alias);
  long long id = 0;
  try
  {
    sqlite3_reader reader= _pimpl->select_repository_cmd->executereader();
    if (!reader.read())
    {
      // does not exist
      _pimpl->insert_repository_cmd->bind(":alias", alias);
      _pimpl->insert_repository_cmd->bind(":timestamp", static_cast<int>((Date::ValueType) Date::now()) );
      _pimpl->insert_repository_cmd->executenonquery();
      id = _pimpl->con.insertid();
      return id;
   }
   return reader.getint64(0);
  }
  catch ( const sqlite3x::database_error &e )
  {
    ZYPP_RETHROW(e);
  }
  return id;
}

void CacheStore::cleanRepository( const data::RecordId &id,
                                  const ProgressData::ReceiverFnc & progressrcv )
{
  sqlite3_command cmd( _pimpl->con, "delete from repositories where id=:id");
  _pimpl->con.setprogresshandler(progressrcv);
  cmd.bind(":id", id);

  try
  {
    cmd.executenonquery();
  }
  catch ( const sqlite3x::database_error &e )
  {
    ZYPP_CAUGHT(e);
    CacheRecordNotFoundException enew;
    enew.remember(e);
    _pimpl->con.resetprogresshandler();
    ZYPP_THROW(enew);
  }
  _pimpl->con.resetprogresshandler();
}

void CacheStore::cleanRepository( const std::string &alias,
                                  const ProgressData::ReceiverFnc & progressrcv )
{
  cleanRepository(lookupRepository(alias), progressrcv);
}

RepoStatus CacheStore::repositoryStatus( const data::RecordId &id )
{
  sqlite3_command cmd( _pimpl->con, "select id,alias,checksum,timestamp from repositories where id=:id");
  cmd.bind(":id", id);

  try
  {
    sqlite3_reader reader = cmd.executereader();
    RepoStatus status;
    if ( reader.read() )
    {
      status.setChecksum( reader.getstring(2) );
      status.setTimestamp( reader.getstring(3) );
      return status;
    }
    else
      ZYPP_THROW(CacheRecordNotFoundException());
  }
  catch ( const sqlite3x::database_error &e )
  {
    ZYPP_RETHROW(e);
  }
}

RepoStatus CacheStore::repositoryStatus( const string &alias )
{
  return repositoryStatus(lookupRepository(alias));
}

bool CacheStore::isCached( const string &alias )
{
  sqlite3_command cmd(_pimpl->con, "select id from repositories where alias=:alias;");
  cmd.bind(":alias", alias);
  try
  {
    sqlite3_reader reader= cmd.executereader();
    if (!reader.read())
    {
      return false;
   }
   return true;
  }
  catch ( const sqlite3x::database_error &e )
  {
    ZYPP_RETHROW(e);
  }
  return false;
}

RecordId CacheStore::lookupRepository( const string &alias )
{
  long long id = 0;
  sqlite3_command cmd(_pimpl->con, "select id from repositories where alias=:alias;");
  cmd.bind(":alias", alias);
  try
  {
    sqlite3_reader reader= cmd.executereader();
    if (!reader.read())
    {
      // does not exist
      ZYPP_THROW(CacheRecordNotFoundException());
   }
   return reader.getint64(0);
  }
  catch ( const sqlite3x::database_error &e )
  {
    ZYPP_RETHROW(e);
  }
  return id;
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
  try
  {
    sqlite3_reader reader= _pimpl->select_type_cmd->executereader();
    if (!reader.read())
    {
      // does not exist
      _pimpl->insert_type_cmd->bind(":class", klass);
      _pimpl->insert_type_cmd->bind(":name", name);
      _pimpl->insert_type_cmd->executenonquery();
      id = _pimpl->con.insertid();
   }
   else
   {
      id = reader.getint64(0);
   }
   _pimpl->type_cache[thetype] = id;
   return id;
  }
  catch ( const sqlite3x::database_error &e )
  {
    ZYPP_RETHROW(e);
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
  long long id = 0;
  try
  {
    _pimpl->select_name_cmd->bind(":name", name);
    sqlite3_reader reader= _pimpl->select_name_cmd->executereader();
    if (!reader.read())
    {
      // does not exist
      _pimpl->insert_name_cmd->bind(":name", name);
      _pimpl->insert_name_cmd->executenonquery();
      id = _pimpl->con.insertid();
    }
    else
    {
      id = reader.getint64(0);
    }
    _pimpl->name_cache[name] = id;
    return id;
  }
  catch ( const sqlite3x::database_error &e )
  {
    ZYPP_RETHROW(e);
  }
  return id;
}

RecordId CacheStore::lookupOrAppendDirName( const string &name )
{
  if ( _pimpl->dir_cache.find(name) != _pimpl->dir_cache.end() )
  {
    _pimpl->dir_cache_hits++;
    return _pimpl->dir_cache[name];
  }
  long long id = 0;
  try
  {
    _pimpl->select_dirname_cmd->bind(":name", name);
    sqlite3_reader reader= _pimpl->select_dirname_cmd->executereader();
    if (!reader.read())
    {
      // does not exist
      _pimpl->insert_dirname_cmd->bind(":name", name);
      _pimpl->insert_dirname_cmd->executenonquery();
      id = _pimpl->con.insertid();
    }
    else
    {
      id = reader.getint64(0);
    }
    _pimpl->dir_cache[name] = id;
    return id;
  }
  catch ( const sqlite3x::database_error &e )
  {
    ZYPP_RETHROW(e);
  }
  return id;
}

RecordId CacheStore::lookupOrAppendFileName( const string &name )
{
  long long id = 0;
  try
  {
    _pimpl->select_filename_cmd->bind(":name", name);
    sqlite3_reader reader= _pimpl->select_filename_cmd->executereader();
    if (!reader.read())
    {
      // does not exist
      _pimpl->insert_filename_cmd->bind(":name", name);
      _pimpl->insert_filename_cmd->executenonquery();
      id = _pimpl->con.insertid();
      return id;
   }
   return reader.getint64(0);
  }
  catch ( const sqlite3x::database_error &e )
  {
    ZYPP_RETHROW(e);
  }
  return id;
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

void CacheStore::appendBooleanAttribute( const data::RecordId & resolvable_id,
                                         const std::string & klass,
                                         const std::string & name,
                                         bool value)
{
  RecordId type_id = lookupOrAppendType( klass, name );
  appendNumericAttribute( resolvable_id, type_id, value ? 1 : 0 );
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
  // don't bother with writing if the string is empty
  if (text.empty()) return;

  RecordId lang_id = lookupOrAppendType("lang", locale.code() );
  RecordId type_id = lookupOrAppendType( klass, name );
  appendStringAttribute( resolvable_id, lang_id, type_id, text );
}

void CacheStore::appendStringAttribute( const data::RecordId &resolvable_id,
                                        const std::string &klass,
                                        const std::string &name,
                                        const std::string &value )
{
  // don't bother with writing if the string is empty
  if (value.empty()) return;

  RecordId type_id = lookupOrAppendType(klass, name);
  appendStringAttribute( resolvable_id, type_id, value );
}

void CacheStore::appendStringAttribute( const RecordId &resolvable_id,
                                        const RecordId &type_id,
                                        const std::string &value )
{
  // don't bother with writing if the string is empty
  if (value.empty()) return;

  RecordId lang_id = lookupOrAppendType("lang", "none");
  appendStringAttribute( resolvable_id, lang_id, type_id, value );
}

void CacheStore::appendStringAttribute( const RecordId &resolvable_id,
                            const RecordId &lang_id,
                            const RecordId &type_id,
                            const string &value )
{
  // don't bother with writing if the string is empty
  if (value.empty()) return;

  // weak resolvable_id
  _pimpl->append_text_attribute_cmd->bind(":rid", resolvable_id );
  _pimpl->append_text_attribute_cmd->bind(":lang_id", lang_id );
  _pimpl->append_text_attribute_cmd->bind(":attr_id", type_id );

  _pimpl->append_text_attribute_cmd->bind(":text", value );

  _pimpl->append_text_attribute_cmd->executenonquery();
}

}
}

