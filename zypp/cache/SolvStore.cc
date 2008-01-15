#include <map>

#include "zypp/base/Easy.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/Measure.h"
#include "zypp/ZYppFactory.h"
#include "zypp/ZYpp.h"
#include "zypp/Package.h"
#include "zypp/PathInfo.h"
#include "zypp/cache/SolvStore.h"
#include "zypp/cache/CacheException.h"
#include "zypp/cache/CacheAttributes.h"

#include "satsolver/repo.h"
#include "satsolver/solvable.h"

using namespace std;
using namespace zypp;
using namespace zypp::capability;
using namespace zypp::cache;
using zypp::data::RecordId;

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

struct SolvStore::Impl
{
  Impl( const Pathname &solvdir )
  : name_cache_hits(0)
  , dir_cache_hits(0)
  , _last_repoid(0)
  , _cachedir(solvdir)
  {
    _pool = pool_create();

  }

  Impl()
  {
    Impl( getZYpp()->homePath() );
  }

  ~Impl()
  {
    MIL << "name cache hits: " << name_cache_hits << " | cache size: " << name_cache.size() << endl;
    pool_free(_pool);
  }

  map<string, RecordId> name_cache;
  map< pair<string,string>, RecordId> type_cache;
  map<string, RecordId> dir_cache;
  int name_cache_hits;
  int dir_cache_hits;

  _Pool *_pool;

  map<RecordId,Repo*> _id2repo;
  map<string, RecordId> _name2repoid;
  RecordId _last_repoid;
  Pathname _cachedir;
};


SolvStore::SolvStore( const Pathname &solvdir )
  : _pimpl( new Impl(solvdir) )
{

}

SolvStore::SolvStore()
    : _pimpl( new Impl() )
{

}

SolvStore::~SolvStore()
{

}

void SolvStore::commit()
{
  // NOOP
}

void SolvStore::appendResObjectAttributes( const data::RecordId &rid,
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


void SolvStore::appendPackageBaseAttributes( const RecordId & pkgid,
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

RecordId SolvStore::consumePackage( const RecordId & repository_id,
				     const data::Package_Ptr & package )
{
  RecordId id = appendResolvable( repository_id, ResTraits<Package>::kind,
      _NVRA( package->name, package->edition, package->arch ), package->deps, package->shareDataWith );
  appendResObjectAttributes( id, package );
  appendPackageBaseAttributes( id, package );

  if ( ! package->srcPackageIdent.name.empty() )
  {
    appendStringAttribute( id, attrPackageSourcePkgName(),    package->srcPackageIdent.name );
    appendStringAttribute( id, attrPackageSourcePkgEdition(), package->srcPackageIdent.edition.asString() );
  }

  return id;
}

RecordId SolvStore::consumeSourcePackage( const data::RecordId & repository_id,
                                       const data::SrcPackage_Ptr & package )
{
  RecordId id = appendResolvable( repository_id, ResTraits<SrcPackage>::kind,
      _NVRA( package->name, package->edition, package->arch ), package->deps, package->shareDataWith );
  appendResObjectAttributes( id, package );

  appendOnMediaLocation( id, attrSrcPackageLocation, package->repositoryLocation );
  return id;
}

RecordId SolvStore::consumePatch( const data::RecordId & repository_id,
                               const data::Patch_Ptr & patch)
{
  RecordId id = appendResolvable(
      repository_id, ResTraits<Patch>::kind,
      _NVRA( patch->name, patch->edition, patch->arch ), patch->deps );

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

RecordId SolvStore::consumePackageAtom( const data::RecordId & repository_id,
                                     const data::PackageAtom_Ptr & atom )
{
  RecordId id = appendResolvable( repository_id, ResTraits<Atom>::kind,
      _NVRA( atom->name, atom->edition, atom->arch ), atom->deps );
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

RecordId SolvStore::consumeMessage( const data::RecordId & repository_id,
                                 const data::Message_Ptr & message )
{
  RecordId id = appendResolvable( repository_id, ResTraits<Message>::kind,
      _NVRA( message->name, message->edition, message->arch ), message->deps );
  appendResObjectAttributes( id, message );

  appendTranslatedStringAttribute( id, attrMessageText(), message->text );
  return id;
}

RecordId SolvStore::consumeScript( const data::RecordId & repository_id,
                                const data::Script_Ptr & script )
{
  RecordId id = appendResolvable( repository_id, ResTraits<Script>::kind,
      _NVRA( script->name, script->edition, script->arch ), script->deps );
  appendResObjectAttributes( id, script );

  appendStringAttribute( id, attrScriptDoScript(), script->doScript );
  appendOnMediaLocation( id, attrScriptDoScriptLocation, script->doScriptLocation );
  appendStringAttribute( id, attrScriptUndoScript(), script->undoScript );
  appendOnMediaLocation( id, attrScriptUndoScriptLocation, script->undoScriptLocation );
  return id;
}

RecordId SolvStore::consumePattern( const data::RecordId & repository_id,
                                     const data::Pattern_Ptr & pattern )
{
  RecordId id = appendResolvable( repository_id, ResTraits<Pattern>::kind,
      _NVRA( pattern->name, pattern->edition, pattern->arch ), pattern->deps );
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

RecordId SolvStore::consumeProduct( const data::RecordId & repository_id,
                                 const data::Product_Ptr & product )
{
  RecordId id = appendResolvable( repository_id, ResTraits<Product>::kind,
      _NVRA( product->name, product->edition, product->arch ), product->deps );
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

RecordId SolvStore::consumeChangelog( const data::RecordId &resolvable_id,
                                   const Changelog & changelog )
{
  //! \todo maybe appendChangelog(const data::RecordId & resolvable_id, Changelog changelog) will be needed
  //! for inserting the changelog using in-memory record id of corresponding resolvable.
  //! (first, we'll see how fast is the inserting without remembering those ids)
  return data::noRecordId;
}

RecordId SolvStore::consumeFilelist( const data::RecordId &resolvable_id,
                                  const data::Filenames & filenames )
{
  //! \todo maybe consumeFilelist(const data::RecordId & resolvable_id, data::Filenames &) will be needed
  return data::noRecordId;
}

void SolvStore::consumeDiskUsage( const data::RecordId &resolvable_id,
                                  const DiskUsage &disk )
{
  // iterate over entries
  for ( DiskUsage::const_iterator it = disk.begin();
        it != disk.end();
        ++it )
  {

  }
  //MIL << "disk usage for " << resolvable_id << " consumed" << endl;
}

void SolvStore::updatePackageLang( const data::RecordId & resolvable_id,
				    const data::Packagebase_Ptr & data_r )
{
  appendTranslatedStringAttribute( resolvable_id, attrResObjectSummary(),          data_r->summary );
  appendTranslatedStringAttribute( resolvable_id, attrResObjectDescription(),      data_r->description );
  appendTranslatedStringAttribute( resolvable_id, attrResObjectLicenseToConfirm(), data_r->licenseToConfirm );
  appendTranslatedStringAttribute( resolvable_id, attrResObjectInsnotify(),        data_r->insnotify );
  appendTranslatedStringAttribute( resolvable_id, attrResObjectDelnotify(),        data_r->delnotify );
}

RecordId SolvStore::appendResolvable( const RecordId &repository_id,
                                       const Resolvable::Kind &kind,
                                       const _NVRA &nvra,
                                       const data::Dependencies &deps )
{
  return appendResolvable( repository_id,
                           kind,
                           nvra,
                           deps,
                           data::noRecordId );
}

data::RecordId
    SolvStore::appendResolvable( const data::RecordId &repository_id,
                                  const Resolvable::Kind &kind,
                                  const _NVRA &nvra,
                                  const data::Dependencies &deps,
                                  const data::RecordId &shared_id )
{

//   Solvable *s = pool_id2solvable(pool, repo_add_solvable(pd->repo));
//   s->evr = makeevr_atts(pool, pd, atts);
//   s->provides = adddep(pool, pd, s->provides, atts, 0);
// 
//   s->name = str2id(pool, nvra.name.c_str(), 1);
//   s->arch = str2id(pool, nvra.arch.c_str(), 1);
//   s->vendor = str2id(pool, nvra.vendor.c_str(), 1);
// 
//   if (!s->arch)
//     s->arch = ARCH_NOARCH;
// 
//   if (s->arch != ARCH_SRC && s->arch != ARCH_NOSRC)
//     s->provides = repo_addid_dep(pd->repo, s->provides, rel2id(pool, s->name, s->evr, REL_EQ, 1), 0);
//   
//    s->supplements = repo_fix_legacy(pd->repo, s->provides, s->supplements);

  // file
  //id = str2id(pool, pd->content, 1);
  //s->provides = repo_addid(pd->repo, s->provides, id);

 
//   long long id = _pimpl->con.insertid();
// 
//   appendDependencies( id, deps );
 
  return 0;
}

void SolvStore::appendDependencies( const RecordId &resolvable_id, const data::Dependencies &deps )
{
  for ( data::Dependencies::const_iterator it = deps.begin(); it != deps.end(); ++it )
  {
    appendDependencyList( resolvable_id, it->first, it->second );
  }
}

void SolvStore::appendDependencyList( const RecordId &resolvable_id, zypp::Dep deptype, const data::DependencyList &caps )
{
  for ( data::DependencyList::const_iterator it = caps.begin(); it != caps.end(); ++it )
  {
    appendDependency( resolvable_id, deptype, *it );
  }
}

void SolvStore::appendDependency( const RecordId &resolvable_id, zypp::Dep deptype, capability::CapabilityImpl::Ptr cap )
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


void SolvStore::appendNamedDependency( const RecordId &resolvable_id, zypp::Dep deptype, capability::NamedCap::Ptr cap )
{
  if ( !cap )
    ZYPP_THROW(Exception("bad versioned dep"));
  
}

void SolvStore::appendModaliasDependency( const RecordId &resolvable_id,
                                                 zypp::Dep deptype,
                                                 capability::ModaliasCap::Ptr cap )
{
  if ( !cap )
    ZYPP_THROW(Exception("Null modalias capability"));
}

void SolvStore::appendHalDependency( const RecordId &resolvable_id,
                                                 zypp::Dep deptype,
                                                 capability::HalCap::Ptr cap )
{
  if ( !cap )
    ZYPP_THROW(Exception("Null HAL capability"));
}

void SolvStore::appendFileDependency( const RecordId &resolvable_id, zypp::Dep deptype,
                                       capability::FileCap::Ptr cap )
{
  if ( !cap )
    ZYPP_THROW(Exception("Null file capability"));
}

void SolvStore::appendFilesystemDependency( const data::RecordId &resolvable_id,
                                             zypp::Dep deptype,
                                             capability::FilesystemCap::Ptr cap )
{
  if ( !cap )
    ZYPP_THROW(Exception("bad versioned dep"));
}


void SolvStore::appendSplitDependency( const data::RecordId &resolvable_id,
                                        zypp::Dep deptype,
                                        capability::SplitCap::Ptr cap )
{
  if ( !cap )
    ZYPP_THROW(Exception("bad versioned dep"));
  //DBG << "versioned : " << cap << endl;
}

void SolvStore::appendUnknownDependency( const RecordId &resolvable_id,
                                               zypp::Dep deptype,
                                               capability::CapabilityImpl::Ptr cap )
{
  if ( !cap )
    ZYPP_THROW(Exception("Null unknown capability"));
}


/** \todo lookupOrAppend ? */
RecordId SolvStore::appendPatchRpm(const zypp::data::RecordId &repository_id, const data::PatchRpm_Ptr & prpm)
{
  RecordId id;
return id;
}


/** \todo lookupOrAppend ? */
RecordId SolvStore::appendDeltaRpm(const zypp::data::RecordId &repository_id, const data::DeltaRpm_Ptr & drpm)
{
  RecordId id;
  return id;
}


RecordId SolvStore::lookupOrAppendFile( const Pathname &path )
{
  long long id = 0;
  return id;
}

void SolvStore::updateRepositoryStatus( const RecordId &id,
                                         const RepoStatus &status )
{
  // NO OP for now
}

RecordId SolvStore::lookupOrAppendRepository( const string &alias )
{
  map<string,RecordId>::const_iterator it = _pimpl->_name2repoid.find(alias);
  if (it == _pimpl->_name2repoid.end())
  {
    _pimpl->_name2repoid[alias] = ++_pimpl->_last_repoid;
  }
  return _pimpl->_name2repoid[alias];
}

void SolvStore::cleanRepository( const data::RecordId &id,
                                  const ProgressData::ReceiverFnc & progressrcv )
{
  // just delete the solv file
  //cleanupRepository(lookupRepository(
}

void SolvStore::cleanRepository( const std::string &alias,
                                  const ProgressData::ReceiverFnc & progressrcv )
{
  // just delete the solv file
  Pathname file( (_pimpl->_cachedir + alias).extend(".solv") );
  filesystem::unlink(file);
}

RepoStatus SolvStore::repositoryStatus( const string &alias )
{
  return RepoStatus();
}

bool SolvStore::isCached( const string &alias )
{
  return _pimpl->_name2repoid.find(alias) != _pimpl->_name2repoid.end();
}

RecordId SolvStore::lookupRepository( const string &alias )
{
  map<string,RecordId>::const_iterator it = _pimpl->_name2repoid.find(alias);
  if (it == _pimpl->_name2repoid.end())
    ZYPP_THROW(CacheRecordNotFoundException());

  return _pimpl->_name2repoid[alias];
}

RecordId SolvStore::lookupOrAppendType( const string &klass, const string &name )
{
  long long id = 0;
  return id;
}

RecordId SolvStore::lookupOrAppendName( const string &name )
{
  long long id = 0;
  return id;
}

RecordId SolvStore::lookupOrAppendDirName( const string &name )
{
  long long id = 0;
  return id;
}

RecordId SolvStore::lookupOrAppendFileName( const string &name )
{
  long long id = 0;
  return id;
}

void SolvStore::setSharedData( const data::RecordId &resolvable_id,
                                const data::RecordId &shared_id )
{
}

void SolvStore::appendBooleanAttribute( const data::RecordId & resolvable_id,
                                         const std::string & klass,
                                         const std::string & name,
                                         bool value)
{
  RecordId type_id = lookupOrAppendType( klass, name );
  appendNumericAttribute( resolvable_id, type_id, value ? 1 : 0 );
}

void SolvStore::appendNumericAttribute( const data::RecordId &resolvable_id,
                                         const std::string &klass,
                                         const std::string &name,
                                         int value )
{
  RecordId type_id = lookupOrAppendType( klass, name );
  appendNumericAttribute( resolvable_id, type_id, value );
}

void SolvStore::appendNumericAttribute( const RecordId &resolvable_id,
                                         const RecordId &type_id,
                                         int value )
{
  // weak resolvable_id
}


void SolvStore::appendTranslatedStringAttribute( const data::RecordId &resolvable_id,
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


void SolvStore::appendStringAttributeTranslation( const data::RecordId &resolvable_id,
                                                   const Locale &locale,
                                                   const std::string &klass,
                                                   const std::string &name,
                                                   const std::string &text )
{
  // don't bother with writing if the string is empty
  if (text.empty()) return;
}

void SolvStore::appendStringAttribute( const data::RecordId &resolvable_id,
                                        const std::string &klass,
                                        const std::string &name,
                                        const std::string &value )
{
  // don't bother with writing if the string is empty
  if (value.empty()) return;
}

void SolvStore::appendStringAttribute( const RecordId &resolvable_id,
                                        const RecordId &type_id,
                                        const std::string &value )
{
  // don't bother with writing if the string is empty
  if (value.empty()) return;
}

void SolvStore::appendStringAttribute( const RecordId &resolvable_id,
                            const RecordId &lang_id,
                            const RecordId &type_id,
                            const string &value )
{
  // don't bother with writing if the string is empty
  if (value.empty()) return;
}

}
}

