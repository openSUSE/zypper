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
#include "satsolver/attr_store.h"

using namespace std;
using namespace zypp;
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
  , _cachedir(solvdir)
  {
    _pool = pool_create();
    _attr = new_store(_pool);
    
    _attr_package_authors = str2id(_pool, "package:authors", 1);
    _attr_package_description = str2id(_pool, "package:description", 1);
    _attr_package_diskusage = str2id(_pool, "package:diskusage", 1);
    _attr_package_downloadsize = str2id(_pool, "package:downloadsize", 1);
    _attr_package_eula = str2id(_pool, "package:eula", 1);
    _attr_package_group = str2id(_pool, "package:group", 1);
    _attr_package_installsize = str2id(_pool, "package:authors", 1);
    _attr_package_keywords = str2id(_pool, "package:keywords", 1);
    _attr_package_license = str2id(_pool, "package:license", 1);
    _attr_package_messagedel = str2id(_pool, "package:messagedel", 1);
    _attr_package_messageins = str2id(_pool, "package:messageins", 1);
    _attr_package_mediadir = str2id(_pool, "package:mediadir", 1);
    _attr_package_mediafile = str2id(_pool, "package:mediafile", 1);
    _attr_package_medianr = str2id(_pool, "package:medianr", 1);
    _attr_package_nosource = str2id(_pool, "package:nosource", 1);
    _attr_package_source = str2id(_pool, "package:source", 1);
    _attr_package_sourceid = str2id(_pool, "package:sourceid", 1);
    _attr_package_summary = str2id(_pool, "package:summary", 1);
    _attr_package_time = str2id(_pool, "package:time", 1);
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

  map<string,Repo*> _repoid2repo;
  
  Pathname _cachedir;
  Attrstore *_attr;

  Id _attr_package_authors;
  Id _attr_package_description;
  Id _attr_package_diskusage;
  Id _attr_package_downloadsize;
  Id _attr_package_eula;
  Id _attr_package_group;
  Id _attr_package_installsize;
  Id _attr_package_keywords;
  Id _attr_package_license;
  Id _attr_package_messagedel;
  Id _attr_package_messageins;
  Id _attr_package_mediadir;
  Id _attr_package_mediafile;
  Id _attr_package_medianr;
  Id _attr_package_nosource;
  Id _attr_package_source;
  Id _attr_package_sourceid;
  Id _attr_package_summary;
  Id _attr_package_time;
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
  
//   appendTranslatedStringAttribute( rid, attrResObjectDescription(), res->description );
//   appendTranslatedStringAttribute( rid, attrResObjectSummary(), res->summary );
//   appendNumericAttribute( rid, attrResObjectInstalledSize(), res->installedSize );
//   appendNumericAttribute( rid, attrResObjectBuildTime(), res->buildTime );
//   appendBooleanAttribute( rid, attrResObjectInstallOnly(), res->installOnly );
//   appendStringAttribute( rid, attrResObjectVendor(), res->vendor );
//   appendTranslatedStringAttribute( rid, attrResObjectLicenseToConfirm(), res->licenseToConfirm );
//   appendTranslatedStringAttribute( rid, attrResObjectInsnotify(), res->insnotify );
//   appendTranslatedStringAttribute( rid, attrResObjectDelnotify(), res->delnotify );
}


void SolvStore::appendPackageBaseAttributes( const RecordId & pkgid,
                                              const data::Packagebase_Ptr & package )
{
    
//   appendStringAttribute( pkgid, attrPackageBuildhost(), package->buildhost );
//   appendStringAttribute( pkgid, attrPackageDistribution(), package->distribution );
//   appendStringAttribute( pkgid, attrPackageLicense(), package->license );
//   appendStringAttribute( pkgid, attrPackageGroup(), package->group );
//   appendStringAttribute( pkgid, attrPackagePackager(), package->packager );
//   appendStringAttribute( pkgid, attrPackageUrl(), package->url );
//   appendStringAttribute( pkgid, attrPackageOperatingSystem(), package->operatingSystem );
//   appendStringAttribute( pkgid, attrPackagePrein(), package->prein );
//   appendStringAttribute( pkgid, attrPackagePostin(), package->postin );
//   appendStringAttribute( pkgid, attrPackagePreun(), package->preun );
//   appendStringAttribute( pkgid, attrPackagePostun(), package->postun );
//   appendStringContainerAttribute( pkgid, attrPackageKeywords(), package->keywords.begin(), package->keywords.end() );
//   appendStringContainerAttribute( pkgid, attrPackageAuthors(), package->authors.begin(), package->authors.end() );
// 
//   appendOnMediaLocation( pkgid, attrPackageLocation, package->repositoryLocation );
}

RecordId SolvStore::consumePackage( const std::string & repository_id,
				     const data::Package_Ptr & package )
{
//   RecordId id = appendResolvable( repository_id, ResTraits<Package>::kind,
//                                   NVRA( package->name, package->edition, package->arch ), package->deps, package->shareDataWith );
//   appendResObjectAttributes( id, package );
//   appendPackageBaseAttributes( id, package );
// 
//   if ( ! package->srcPackageIdent.name.empty() )
//   {
//     appendStringAttribute( id, attrPackageSourcePkgName(),    package->srcPackageIdent.name );
//     appendStringAttribute( id, attrPackageSourcePkgEdition(), package->srcPackageIdent.edition.asString() );
//   }

  //return id;
  return 0;
}

RecordId SolvStore::consumeSourcePackage( const std::string & repository_id,
                                       const data::SrcPackage_Ptr & package )
{
//   RecordId id = appendResolvable( repository_id, ResTraits<SrcPackage>::kind,
//       NVRA( package->name, package->edition, package->arch ), package->deps, package->shareDataWith );
//   appendResObjectAttributes( id, package );
// 
//   appendOnMediaLocation( id, attrSrcPackageLocation, package->repositoryLocation );
  //return id;
  return 0;
}

RecordId SolvStore::consumePatch( const std::string & repository_id,
                               const data::Patch_Ptr & patch)
{/*
  RecordId id = appendResolvable(
      repository_id, ResTraits<Patch>::kind,
      NVRA( patch->name, patch->edition, patch->arch ), patch->deps );*/

//   appendResObjectAttributes( id, patch );
// 
//   // patch attributes
//   appendNumericAttribute( id, attrPatchTimestamp(),         patch->timestamp );
//   appendStringAttribute(  id, attrPatchCategory(),          patch->category );
//   appendStringAttribute(  id, attrPatchId(),                patch->id );
//   appendBooleanAttribute( id, attrPatchRebootNeeded(),      patch->rebootNeeded );
//   appendBooleanAttribute( id, attrPatchAffectsPkgManager(), patch->affectsPkgManager );
// 
// 
//   DBG << "got patch " << patch->name << ", atoms: ";
//   // cosume atoms
//   for (set<data::ResObject_Ptr>::const_iterator p = patch->atoms.begin();
//        p != patch->atoms.end(); ++p)
//   {
//     data::PackageAtom_Ptr atom = dynamic_pointer_cast<data::PackageAtom>(*p);
//     if (atom)
//     {
//       DBG << atom->name << "(atom) ";
//       consumePackageAtom(repository_id, atom);
//       continue;
//     }
// 
//     data::Script_Ptr script = dynamic_pointer_cast<data::Script>(*p);
//     if (script)
//     {
//       DBG << script->name << "(script) ";
//       consumeScript(repository_id, script);
//       continue;
//     }
// 
//     data::Message_Ptr message = dynamic_pointer_cast<data::Message>(*p);
//     if (message)
//     {
//       DBG << message->name << "(message) ";
//       consumeMessage(repository_id, message);
//       continue;
//     }
// 
//     ERR << " ignoring !badatom! ";
//     if (*p) ERR << (*p)->name;
//     ERR << endl;
//   }
// 
//   DBG << endl;
  //return id;
  return 0;
}

RecordId SolvStore::consumePackageAtom( const std::string & repository_id,
                                     const data::PackageAtom_Ptr & atom )
{
//   RecordId id = appendResolvable( repository_id, ResTraits<Atom>::kind,
//       NVRA( atom->name, atom->edition, atom->arch ), atom->deps );
//   appendResObjectAttributes( id, atom );
//   appendPackageBaseAttributes( id, atom );
// 
//   for (set<data::PatchRpm_Ptr>::const_iterator p = atom->patchRpms.begin();
//        p != atom->patchRpms.end(); ++p)
//     appendPatchRpm(repository_id, *p);
// 
//   for (set<data::DeltaRpm_Ptr>::const_iterator d = atom->deltaRpms.begin();
//        d != atom->deltaRpms.end(); ++d)
//     appendDeltaRpm(repository_id, *d);
  //7return id;
  return 0;
}

RecordId SolvStore::consumeMessage( const std::string & repository_id,
                                 const data::Message_Ptr & message )
{
//   RecordId id = appendResolvable( repository_id, ResTraits<Message>::kind,
//       NVRA( message->name, message->edition, message->arch ), message->deps );
//   appendResObjectAttributes( id, message );
// 
//   appendTranslatedStringAttribute( id, attrMessageText(), message->text );
  //7return id;
  return 0;
}

RecordId SolvStore::consumeScript( const std::string & repository_id,
                                const data::Script_Ptr & script )
{
  //RecordId id = appendResolvable( repository_id, ResTraits<Script>::kind,
      //NVRA( script->name, script->edition, script->arch ), script->deps );
//   appendResObjectAttributes( id, script );
// 
//   appendStringAttribute( id, attrScriptDoScript(), script->doScript );
//   appendOnMediaLocation( id, attrScriptDoScriptLocation, script->doScriptLocation );
//   appendStringAttribute( id, attrScriptUndoScript(), script->undoScript );
//   appendOnMediaLocation( id, attrScriptUndoScriptLocation, script->undoScriptLocation );
  //return id;
  return 0;
}

RecordId SolvStore::consumePattern( const std::string & repository_id,
                                     const data::Pattern_Ptr & pattern )
{
//   RecordId id = appendResolvable( repository_id, ResTraits<Pattern>::kind,
//       NVRA( pattern->name, pattern->edition, pattern->arch ), pattern->deps );
//   appendResObjectAttributes( id, pattern );
// 
//   appendBooleanAttribute( id, attrPatternIsDefault(), pattern->isDefault );
//   appendBooleanAttribute( id, attrPatternUserVisible(), pattern->userVisible );
//   appendTranslatedStringAttribute( id, attrPatternCategory(), pattern->category );
//   appendStringAttribute( id, attrPatternIcon(), pattern->icon );
//   appendStringAttribute( id, attrPatternOrder(), pattern->order );

  // We store them as string. They are
  // (sometimes) evaluated by the YaST UI.
//   appendStringContainerAttribute( id, attrPatternUiIncludes(), pattern->includes.begin(), pattern->includes.end() );
//   appendStringContainerAttribute( id, attrPatternUiExtends(),  pattern->extends.begin(),  pattern->extends.end() );

  //return id;
  return 0;
}

RecordId SolvStore::consumeProduct( const std::string & repository_id,
                                 const data::Product_Ptr & product )
{
//   RecordId id = appendResolvable( repository_id, ResTraits<Product>::kind,
//       NVRA( product->name, product->edition, product->arch ), product->deps );
//   appendResObjectAttributes( id, product );
// 
//   appendStringAttribute( id, attrProductType(), product->type );
//   appendTranslatedStringAttribute( id, attrProductShortName(), product->shortName );
//   appendTranslatedStringAttribute( id, attrProductLongName(), product->longName );
//   appendStringContainerAttribute( id, attrProductFlags(), product->flags.begin(), product->flags.end() );
//   appendStringAttribute( id, attrProductReleasenotesUrl(), product->releasenotesUrl.asString() );
//   appendStringContainerAttribute( id, attrProductUpdateUrls(), product->updateUrls );
//   appendStringContainerAttribute( id, attrProductExtraUrls(), product->extraUrls );
//   appendStringContainerAttribute( id, attrProductOptionalUrls(), product->optionalUrls );
//   appendStringAttribute( id, attrProductDistributionName(), product->distributionName );
//   appendStringAttribute( id, attrProductDistributionEdition(), product->distributionEdition.asString() );
  //return id;
  return 0;
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
//   appendTranslatedStringAttribute( resolvable_id, attrResObjectSummary(),          data_r->summary );
//   appendTranslatedStringAttribute( resolvable_id, attrResObjectDescription(),      data_r->description );
//   appendTranslatedStringAttribute( resolvable_id, attrResObjectLicenseToConfirm(), data_r->licenseToConfirm );
//   appendTranslatedStringAttribute( resolvable_id, attrResObjectInsnotify(),        data_r->insnotify );
//   appendTranslatedStringAttribute( resolvable_id, attrResObjectDelnotify(),        data_r->delnotify );
}

_Solvable* SolvStore::appendResolvable( const std::string & repository_id,
                                          const data::Resolvable_Ptr &res )
{
  Repo *repo;
  map<string, Repo*>::const_iterator it = _pimpl->_repoid2repo.find(repository_id);
  if ( it == _pimpl->_repoid2repo.end() )
  {
    // throw
  }
  repo = it->second;

  //Id
  _Solvable *s = pool_id2solvable(_pimpl->_pool, repo_add_solvable(repo));
  s->evr = str2id(_pimpl->_pool, res->edition.c_str(), 1);
//   s->provides = adddep(pool, pd, s->provides, atts, 0);
//
//   s->name = str2id(pool, nvra.name.c_str(), 1);
//   s->arch = str2id(pool, nvra.arch.c_str(), 1);
//   s->vendor = str2id(pool, nvra.vendor.c_str(), 1);
//
//   if (!s->arch)
//     s->arch = ARCH_NOARCH;

  return s;
}


/** \todo lookupOrAppend ? */
RecordId SolvStore::appendPatchRpm( const std::string & repository_id, const data::PatchRpm_Ptr & prpm)
{
  RecordId id;
return id;
}


/** \todo lookupOrAppend ? */
RecordId SolvStore::appendDeltaRpm( const std::string & repository_id, const data::DeltaRpm_Ptr & drpm)
{
  RecordId id;
  return id;
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
  return _pimpl->_repoid2repo.find(alias) != _pimpl->_repoid2repo.end();
}

}
}

