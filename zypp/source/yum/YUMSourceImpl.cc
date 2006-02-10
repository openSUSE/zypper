/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/source/yum/YUMSourceImpl.cc
 *
*/

#include "zypp/source/yum/YUMSourceImpl.h"
#include "zypp/source/yum/YUMPackageImpl.h"
#include "zypp/source/yum/YUMScriptImpl.h"
#include "zypp/source/yum/YUMMessageImpl.h"
#include "zypp/source/yum/YUMPatchImpl.h"
#include "zypp/source/yum/YUMProductImpl.h"
#include "zypp/source/yum/YUMGroupImpl.h"
#include "zypp/source/yum/YUMPatternImpl.h"

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/CapFactory.h"

#include "zypp/parser/yum/YUMParser.h"
#include "zypp/SourceFactory.h"

#include <fstream>

using namespace std;
using namespace zypp::detail;
using namespace zypp::parser::yum;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    namespace yum
    {
      ///////////////////////////////////////////////////////////////////
      //
      //        CLASS NAME : YUMSourceImpl
      //
      ///////////////////////////////////////////////////////////////////

      YUMSourceImpl::YUMSourceImpl()
      {}

      void YUMSourceImpl::factoryInit()
      {
        try {

	  media::MediaManager media_mgr;

	  MIL << "Adding no media verifier" << endl;
	  media::MediaAccessId _media = _media_set->getMediaAccessId(1);
	  media_mgr.delVerifier(_media);
	  media_mgr.addVerifier(_media, media::MediaVerifierRef(new media::NoVerifier()));
        }
        catch (const Exception & excpt_r)
        {
#warning FIXME: If media data is not set, verifier is not set. Should the media be refused instead?
	  ZYPP_CAUGHT(excpt_r);
	  WAR << "Verifier not found" << endl;
        }

       try {


	// first read list of all files in the repository
	Pathname filename;
	if (_cache_dir.empty())
	{
	  // now, the file exists, try to read it
	  filename = provideFile(_path + "/repodata/repomd.xml");
	}
	else
	{
	  filename = _cache_dir + "/repodata/repomd.xml";
	}

	DBG << "Reading file " << filename << endl;
	ifstream repo_st(filename.asString().c_str());
	YUMRepomdParser repomd(repo_st, "");

	for(;
	    ! repomd.atEnd();
	    ++repomd)
	{
	}
       }
       catch (...)
       {
	ERR << "Cannot read repomd file, cannot initialize source" << endl;
	ZYPP_THROW( Exception("Cannot read repomd file, cannot initialize source") );
       }
      }

      void YUMSourceImpl::storeMetadata(const Pathname & cache_dir_r)
      {
INT << "Storing data to cache" << endl;
	resolvables(SourceFactory().createFrom(this));
	for (std::list<Pathname>::const_iterator it = _metadata_files.begin();
	  it != _metadata_files.end(); it++)
	{
	  Pathname src = provideFile(_path + *it);
	  Pathname dst = cache_dir_r + *it;
	  if (0 != assert_dir(dst.dirname(), 0700))
	    ZYPP_THROW(Exception("Cannot create cache directory"));
	  filesystem::copy(src, dst);
	}
      }

      void YUMSourceImpl::createResolvables(Source_Ref source_r)
      {
       std::list<YUMRepomdData_Ptr> repo_primary;
       std::list<YUMRepomdData_Ptr> repo_files;
       std::list<YUMRepomdData_Ptr> repo_other;
       std::list<YUMRepomdData_Ptr> repo_group;
       std::list<YUMRepomdData_Ptr> repo_pattern;
       std::list<YUMRepomdData_Ptr> repo_product;
       std::list<YUMRepomdData_Ptr> repo_patches;

       try {
	// first read list of all files in the reposotory
        Pathname filename = _cache_dir.empty()
	  ? provideFile(_path + "/repodata/repomd.xml")
	  : _cache_dir + "/repodata/repomd.xml";
	_metadata_files.push_back("/repodata/repomd.xml");
	DBG << "Reading file " << filename << endl;
	ifstream repo_st(filename.asString().c_str());
	YUMRepomdParser repomd(repo_st, "");

	for(;
	    ! repomd.atEnd();
	    ++repomd)
	{
	  if ((*repomd)->type == "primary")
	    repo_primary.push_back(*repomd);
	  else if ((*repomd)->type == "filelists")
	    repo_files.push_back(*repomd);
	  else if ((*repomd)->type == "other")
	    repo_other.push_back(*repomd);
	  else if ((*repomd)->type == "group")
	    repo_group.push_back(*repomd);
	  else if ((*repomd)->type == "pattern")
	    repo_pattern.push_back(*repomd);
	  else if ((*repomd)->type == "product")
	    repo_product.push_back(*repomd);
	  else if ((*repomd)->type == "patches")
	    repo_patches.push_back(*repomd);
	  else
	    ERR << "Unknown type of repo file: " << (*repomd)->type << endl;
	}
       }
       catch (...)
       {
	ERR << "Cannot read repomd file, cannot initialize source" << endl;
	ZYPP_THROW( Exception("Cannot read repomd file, cannot initialize source") );
       }
       try {
	// now put other and filelist data to structures for easier find
	map<PackageID, YUMFileListData_Ptr> files_data;
	map<PackageID, YUMOtherData_Ptr> other_data;
	for (std::list<YUMRepomdData_Ptr>::const_iterator it
					= repo_files.begin();
	     it != repo_files.end();
	     it++)
	{
	  // TODO check checksum
	  Pathname filename = _cache_dir.empty()
	    ? provideFile(_path + (*it)->location)
	    : _cache_dir + (*it)->location;
	  _metadata_files.push_back((*it)->location);
	  DBG << "Reading file " << filename << endl;
	  ifstream st(filename.asString().c_str());
	  YUMFileListParser filelist(st, "");
	  for (;
	       ! filelist.atEnd();
	       ++filelist)
	  {
	    PackageID id((*filelist)->name,
	                 (*filelist)->ver,
	                 (*filelist)->rel,
	                 (*filelist)->arch);
	    files_data[id] = *filelist;
	  }
	  if (filelist.errorStatus())
	    throw *filelist.errorStatus();
	}

	for (std::list<YUMRepomdData_Ptr>::const_iterator it
					= repo_other.begin();
	     it != repo_other.end();
	     it++)
	{
	  // TODO check checksum
	  Pathname filename = _cache_dir.empty()
	    ? provideFile(_path + (*it)->location)
	    : _cache_dir + (*it)->location;
	  _metadata_files.push_back((*it)->location);
	  DBG << "Reading file " << filename << endl;
	  ifstream st(filename.asString().c_str());
	  YUMOtherParser other(st, "");
	  for (;
	       ! other.atEnd();
	       ++other)
	  {
	    PackageID id((*other)->name,
	                 (*other)->ver,
	                 (*other)->rel,
	                 (*other)->arch);
	    other_data[id] = *other;
	  }
	  if (other.errorStatus())
	    throw *other.errorStatus();
	}

	// now read primary data, merge them with filelist and changelog
	for (std::list<YUMRepomdData_Ptr>::const_iterator it
					 = repo_primary.begin();
	     it != repo_primary.end();
	     it++)
	{
	  // TODO check checksum
	  Pathname filename = _cache_dir.empty()
	    ? provideFile(_path + (*it)->location)
	    : _cache_dir + (*it)->location;
	  _metadata_files.push_back((*it)->location);
	  DBG << "Reading file " << filename << endl;
	  ifstream st(filename.asString().c_str());
	  YUMPrimaryParser prim(st, "");
	  for (;
	       !prim.atEnd();
	       ++prim)
	  {
	    PackageID id((*prim)->name,
	                 (*prim)->ver,
	                 (*prim)->rel,
	                 (*prim)->arch);
	    map<PackageID, YUMOtherData_Ptr>::iterator found_other
	        = other_data.find(id);
	    map<PackageID, YUMFileListData_Ptr>::iterator found_files
	        = files_data.find(id);

	    YUMFileListData filelist_empty;
	    YUMOtherData other_empty;
	    Package::Ptr p = createPackage(
	      source_r,
	      **prim,
	      found_files != files_data.end()
	        ? *found_files->second
	        : filelist_empty,
	      found_other != other_data.end()
	        ? *found_other->second
	        : other_empty
	    );
	    _store.insert (p);
	  }
	  if (prim.errorStatus())
	    throw *prim.errorStatus();
	}
       }
       catch (...) {
	ERR << "Cannot read package information" << endl;
       }

       try {
	// groups
	for (std::list<YUMRepomdData_Ptr>::const_iterator it = repo_group.begin();
	     it != repo_group.end();
	     it++)
	{
	  // TODO check checksum
	  Pathname filename = _cache_dir.empty()
	    ? provideFile(_path + (*it)->location)
	    : _cache_dir + (*it)->location;
	  _metadata_files.push_back((*it)->location);
	  DBG << "Reading file " << filename << endl;
	  ifstream st(filename.asString().c_str());
	  YUMGroupParser group(st, "");
	  for (;
	       !group.atEnd();
	       ++group)
	  {
	    Selection::Ptr p = createGroup(
	      source_r,
	      **group
	    );
	    _store.insert (p);
	  }
	  if (group.errorStatus())
	    throw *group.errorStatus();
	}
       }
       catch (...) {
	ERR << "Cannot read package groups information" << endl;
       }

       try {
	// patterns
	for (std::list<YUMRepomdData_Ptr>::const_iterator it = repo_pattern.begin();
	     it != repo_pattern.end();
	     it++)
	{
	  // TODO check checksum
	  Pathname filename = _cache_dir.empty()
	    ? provideFile(_path + (*it)->location)
	    : _cache_dir + (*it)->location;
	  _metadata_files.push_back((*it)->location);
	  DBG << "Reading file " << filename << endl;
	  ifstream st(filename.asString().c_str());
	  YUMPatternParser pattern(st, "");
	  for (;
	       !pattern.atEnd();
	       ++pattern)
	  {
	    Pattern::Ptr p = createPattern(
	      source_r,
	      **pattern
	    );
	    _store.insert (p);
	  }
	  if (pattern.errorStatus())
	    throw *pattern.errorStatus();
	}
       }
       catch (...) {
	ERR << "Cannot read installation patterns information" << endl;
       }

       try {
	// products
	for (std::list<YUMRepomdData_Ptr>::const_iterator it = repo_product.begin();
	     it != repo_product.end();
	     it++)
	{
	  // TODO check checksum
	  Pathname filename = _cache_dir.empty()
	    ? provideFile(_path + (*it)->location)
	    : _cache_dir + (*it)->location;
	  _metadata_files.push_back((*it)->location);
	  DBG << "Reading file " << filename << endl;
	  ifstream st(filename.asString().c_str());
	  YUMProductParser product(st, "");
	  for (;
	       !product.atEnd();
	       ++product)
	  {
	    Product::Ptr p = createProduct(
	      source_r,
	      **product
	    );
	    _store.insert (p);
	  }
	  if (product.errorStatus())
	    throw *product.errorStatus();
	}
       }
       catch (...) {
	ERR << "Cannot read products information" << endl;
       }

       try {
	// patches
	std::list<std::string> patch_files;
	for (std::list<YUMRepomdData_Ptr>::const_iterator it = repo_patches.begin();
	     it != repo_patches.end();
	     it++)
	{
	  // TODO check checksum
	  Pathname filename = _cache_dir.empty()
	    ? provideFile(_path + (*it)->location)
	    : _cache_dir + (*it)->location;
	  _metadata_files.push_back((*it)->location);
	  DBG << "Reading file " << filename << endl;
	  ifstream st(filename.asString().c_str());
	  YUMPatchesParser patch(st, "");
	  for (;
	       !patch.atEnd();
	       ++patch)
	  {
	    // TODO check checksum
	    string filename = (*patch)->location;
	    patch_files.push_back(filename);
	  }
	  if (patch.errorStatus())
	    throw *patch.errorStatus();
	}

	for (std::list<std::string>::const_iterator it = patch_files.begin();
	     it != patch_files.end();
	     it++)
	{
	    Pathname filename = _cache_dir.empty()
	      ? provideFile(_path + *it)
	      : _cache_dir + *it;
	    _metadata_files.push_back(*it);
	    DBG << "Reading file " << filename << endl;
	    ifstream st(filename.asString().c_str());
	    YUMPatchParser ptch(st, "");
	    for(;
	        !ptch.atEnd();
	        ++ptch)
	    {
	      Patch::Ptr p = createPatch(
	        source_r,
		**ptch
	      );
	      _store.insert (p);
	      Patch::AtomList atoms = p->atoms();
	      for (Patch::AtomList::iterator at = atoms.begin();
		at != atoms.end();
		at++)
	      {
	        _store.insert (*at);
	      }
	    }
	    if (ptch.errorStatus())
	      throw *ptch.errorStatus();
	}
       }
       catch (...)
       {
	ERR << "Cannot read patch metadata" << endl;
       }
      }

	Package::Ptr YUMSourceImpl::createPackage(
	  Source_Ref source_r,
	  const zypp::parser::yum::YUMPrimaryData & parsed,
	  const zypp::parser::yum::YUMFileListData & filelist,
	  const zypp::parser::yum::YUMOtherData & other
	)
	{
	  try
	  {
	    shared_ptr<YUMPackageImpl> impl(
	      new YUMPackageImpl(source_r, parsed, filelist, other));

            // Collect basic Resolvable data
            NVRAD dataCollect( parsed.name,
                               Edition( parsed.ver, parsed.rel, parsed.epoch ),
                               Arch( parsed.arch ),
                               createDependencies(parsed,
                                                   ResTraits<Package>::kind)
                             );
	    Package::Ptr package = detail::makeResolvableFromImpl(
	      dataCollect, impl
	    );
	    return package;
	  }
	  catch (const Exception & excpt_r)
	  {
	    ERR << excpt_r << endl;
	    throw "Cannot create package object";
	  }
	}

	Package::Ptr YUMSourceImpl::createPackage(
	  Source_Ref source_r,
	  const zypp::parser::yum::YUMPatchPackage & parsed
	)
	{
	  try
	  {
	    shared_ptr<YUMPackageImpl> impl(new YUMPackageImpl(source_r, parsed));

            // Collect basic Resolvable data
            NVRAD dataCollect( parsed.name,
                               Edition( parsed.ver, parsed.rel, parsed.epoch ),
                               Arch( parsed.arch ),
                               createDependencies(parsed,
                                                   ResTraits<Package>::kind)
                             );
	    Package::Ptr package = detail::makeResolvableFromImpl(
	      dataCollect, impl
	    );
	    return package;
	  }
	  catch (const Exception & excpt_r)
	  {
	    ERR << excpt_r << endl;
	    throw "Cannot create package object";
	  }
	}

	Selection::Ptr YUMSourceImpl::createGroup(
	  Source_Ref source_r,
	  const zypp::parser::yum::YUMGroupData & parsed
	)
	{
	  try
	  {
	    shared_ptr<YUMGroupImpl> impl(new YUMGroupImpl(source_r, parsed));
            // Collect basic Resolvable data
            NVRAD dataCollect( parsed.groupId,
                               Edition::noedition,
                               Arch_noarch,
                               createGroupDependencies(parsed));
 	    Selection::Ptr group = detail::makeResolvableFromImpl(
	      dataCollect, impl
	    );
	    return group;
	  }
	  catch (const Exception & excpt_r)
	  {
	    ERR << excpt_r << endl;
	    throw "Cannot create package group object";
	  }
	}

	Pattern::Ptr YUMSourceImpl::createPattern(
	  Source_Ref source_r,
	  const zypp::parser::yum::YUMPatternData & parsed
	)
	{
	  try
	  {
	    shared_ptr<YUMPatternImpl> impl(new YUMPatternImpl(source_r, parsed));
            // Collect basic Resolvable data
            NVRAD dataCollect( parsed.name,
                               Edition::noedition,
                               Arch_noarch,
                               createDependencies(parsed, ResTraits<Pattern>::kind));
 	    Pattern::Ptr pattern = detail::makeResolvableFromImpl(
	      dataCollect, impl
	    );
	    return pattern;
	  }
	  catch (const Exception & excpt_r)
	  {
	    ERR << excpt_r << endl;
	    throw "Cannot create installation pattern object";
	  }
	}

	Message::Ptr YUMSourceImpl::createMessage(
	  Source_Ref source_r,
	  const zypp::parser::yum::YUMPatchMessage & parsed
	)
	{
	  try
	  {
	    shared_ptr<YUMMessageImpl> impl(new YUMMessageImpl(source_r, parsed));

            // Collect basic Resolvable data
            NVRAD dataCollect( parsed.name,
                               Edition( parsed.ver, parsed.rel, parsed.epoch ),
                               Arch_noarch,
                               createDependencies(parsed,
                                                   ResTraits<Message>::kind)
                             );
	    Message::Ptr message = detail::makeResolvableFromImpl(
	      dataCollect, impl
	    );
	    return message;
	  }
	  catch (const Exception & excpt_r)
	  {
	    ERR << excpt_r << endl;
	    throw "Cannot create message object";
	  }
	}

	Script::Ptr YUMSourceImpl::createScript(
	  Source_Ref source_r,
	  const zypp::parser::yum::YUMPatchScript & parsed
	)
	{
	  try
	  {
	    shared_ptr<YUMScriptImpl> impl(new YUMScriptImpl(source_r, parsed));

            // Collect basic Resolvable data
            NVRAD dataCollect( parsed.name,
                               Edition( parsed.ver, parsed.rel, parsed.epoch ),
                               Arch_noarch,
                               createDependencies(parsed,
                                                   ResTraits<Script>::kind)
                             );
	    Script::Ptr script = detail::makeResolvableFromImpl(
	      dataCollect, impl
	    );
	    return script;
	  }
	  catch (const Exception & excpt_r)
	  {
	    ERR << excpt_r << endl;
	    throw "Cannot create script object";
	  }
	}

	Product::Ptr YUMSourceImpl::createProduct(
	  Source_Ref source_r,
	  const zypp::parser::yum::YUMProductData & parsed
	)
	{
	  try
	  {
	    shared_ptr<YUMProductImpl> impl(new YUMProductImpl(source_r, parsed));

            // Collect basic Resolvable data
            NVRAD dataCollect( parsed.name,
                               Edition( parsed.ver, parsed.rel, parsed.epoch ),
                               Arch_noarch,
                               createDependencies(parsed,
                                                   ResTraits<Product>::kind)
                             );
	    Product::Ptr product = detail::makeResolvableFromImpl(
	      dataCollect, impl
	    );
	    return product;
	  }
	  catch (const Exception & excpt_r)
	  {
	    ERR << excpt_r << endl;
	    throw "Cannot create product object";
	  }
	}

	Patch::Ptr YUMSourceImpl::createPatch(
	  Source_Ref source_r,
	  const zypp::parser::yum::YUMPatchData & parsed
	)
	{
	  try
	  {
	    shared_ptr<YUMPatchImpl> impl(new YUMPatchImpl(source_r, parsed, *this));

            // Collect basic Resolvable data
            NVRAD dataCollect( parsed.name,
                               Edition( parsed.ver, parsed.rel, parsed.epoch ),
                               Arch_noarch,
                               createDependencies(parsed,
                                                   ResTraits<Patch>::kind)
                             );
	    Patch::Ptr patch = detail::makeResolvableFromImpl(
	      dataCollect, impl
	    );
	    return patch;
	  }
	  catch (const Exception & excpt_r)
	  {
	    ERR << excpt_r << endl;
	    throw "Cannot create patch object";
	  }
	}

	Dependencies YUMSourceImpl::createDependencies(
	  const zypp::parser::yum::YUMObjectData & parsed,
	  const Resolvable::Kind my_kind
	)
	{
	  Dependencies _deps;
	  for (std::list<YUMDependency>::const_iterator it = parsed.provides.begin();
	       it != parsed.provides.end();
	       it++)
	  {
	    _deps[Dep::PROVIDES].insert(createCapability(*it, my_kind));
	  }

	  for (std::list<YUMDependency>::const_iterator it = parsed.conflicts.begin();
	       it != parsed.conflicts.end();
	       it++)
	  {
	    _deps[Dep::CONFLICTS].insert(createCapability(*it, my_kind));
	  }

	  for (std::list<YUMDependency>::const_iterator it = parsed.obsoletes.begin();
	       it != parsed.obsoletes.end();
	       it++)
	  {
	    _deps[Dep::OBSOLETES].insert(createCapability(*it, my_kind));
	  }

	  for (std::list<YUMDependency>::const_iterator it = parsed.freshen.begin();
	       it != parsed.freshen.end();
	       it++)
	  {
	    _deps[Dep::FRESHENS].insert(createCapability(*it, my_kind));
	  }

	  for (std::list<YUMDependency>::const_iterator it = parsed.recommends.begin();
	       it != parsed.recommends.end();
	       it++)
	  {
	    _deps[Dep::RECOMMENDS].insert(createCapability(*it, my_kind));
	  }

	  for (std::list<YUMDependency>::const_iterator it = parsed.suggests.begin();
	       it != parsed.suggests.end();
	       it++)
	  {
	    _deps[Dep::SUGGESTS].insert(createCapability(*it, my_kind));
	  }

	  for (std::list<YUMDependency>::const_iterator it = parsed.enhances.begin();
	       it != parsed.enhances.end();
	       it++)
	  {
	    _deps[Dep::ENHANCES].insert(createCapability(*it, my_kind));
	  }

	  for (std::list<YUMDependency>::const_iterator it = parsed.requires.begin();
	       it != parsed.requires.end();
	       it++)
	  {
	    if (it->pre == "1")
	      _deps[Dep::PREREQUIRES].insert(createCapability(*it, my_kind));
	    else
	      _deps[Dep::REQUIRES].insert(createCapability(*it, my_kind));
	  }

	  return _deps;
	}

	Dependencies YUMSourceImpl::createGroupDependencies(
	  const zypp::parser::yum::YUMGroupData & parsed
	)
	{
	  Dependencies _deps;

	  for (std::list<PackageReq>::const_iterator it = parsed.packageList.begin();
	    it != parsed.packageList.end();
	    it++)
	  {
	    if (it->type == "mandatory" || it->type == "")
	    {
	      _deps[Dep::REQUIRES].insert(createCapability(YUMDependency(
		  "",
		  it->name,
		  "EQ",
		  it->epoch,
		  it->ver,
		  it->rel,
		  ""
		),
		ResTraits<Package>::kind));
	    }
	  }
	  for (std::list<MetaPkg>::const_iterator it = parsed.grouplist.begin();
	    it != parsed.grouplist.end();
	    it++)
	  {
	    if (it->type == "mandatory" || it->type == "")
	    {
	      _deps[Dep::REQUIRES].insert(createCapability(YUMDependency(
		  "",
		  it->name,
		  "",
		  "",
		  "",
		  "",
		  ""
		),
		ResTraits<Selection>::kind));
	    }
	  }
	  return _deps;
	}

	Capability YUMSourceImpl::createCapability(const YUMDependency & dep,
					       const Resolvable::Kind & my_kind)
	{
	  CapFactory _f;
	  Resolvable::Kind _kind = dep.kind == "" ? my_kind : Resolvable::Kind(dep.kind);
	  Capability cap;
    if ( ! dep.isEncoded() )
    {
      cap = _f.parse(
	    _kind,
	    dep.name,
	    Rel(dep.flags),
	    Edition(dep.ver, dep.rel, dep.epoch)
	     );
    }
    else
    {
      cap = _f.parse( _kind, dep.encoded );
    }
	  return cap;
	}

    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
