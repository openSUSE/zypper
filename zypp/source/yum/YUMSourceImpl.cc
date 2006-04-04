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
#include "zypp/source/yum/YUMAtomImpl.h"
#include "zypp/source/yum/YUMPackageImpl.h"
#include "zypp/source/yum/YUMScriptImpl.h"
#include "zypp/source/yum/YUMMessageImpl.h"
#include "zypp/source/yum/YUMPatchImpl.h"
#include "zypp/source/yum/YUMProductImpl.h"
#include "zypp/source/yum/YUMGroupImpl.h"
#include "zypp/source/yum/YUMPatternImpl.h"

#include "zypp/NVRA.h"
#include "zypp/PathInfo.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/CapFactory.h"
#include "zypp/Digest.h"
#include "zypp/ExternalProgram.h"
#include "zypp/TmpPath.h"
#include "zypp/ZYppFactory.h"
#include "zypp/KeyRing.h"

#include "zypp/parser/yum/YUMParser.h"
#include "zypp/SourceFactory.h"
#include "zypp/ZYppCallbacks.h"

#include "zypp/base/GzStream.h"
#include "zypp/base/Gettext.h"
#include "zypp/PathInfo.h"

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

      bool YUMSourceImpl::cacheExists()
      {
        bool exists = PathInfo(_cache_dir + "/repodata/repomd.xml").isExist();
        if (exists)
          MIL << "YUM cache found at " << _cache_dir << std::endl;
        else
          MIL << "YUM cache not found" << std::endl;
        
        return exists;
      }
      
      void YUMSourceImpl::factoryInit()
      {
	try {
	  media::MediaManager media_mgr;
	  MIL << "Adding no media verifier" << endl;
	  
	  // don't try to attach media
	  media::MediaAccessId _media = _media_set->getMediaAccessId(1, true);
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
	  if (!cacheExists())
	  {
	    // now, the file exists, try to read it
	    filename = provideFile(_path + "/repodata/repomd.xml");
	  }
	  else
	  {
	    filename = _cache_dir + "/repodata/repomd.xml";
	  }
	
	  if ( ! PathInfo(filename).isExist() )
	    ZYPP_THROW(Exception("repodata/repomd.xml not found"));

	  // use tmpfile because of checking integrity - provideFile might release the medium
	  filesystem::TmpFile tmp;
	  filesystem::copy(filename, tmp.path());
	  filename = tmp.path();
	  if (!cacheExists())
	  {
	    MIL << "Trying to get the key" << endl;
	    Pathname key_local;
	    try {
	      key_local = provideFile(_path + "/repodata/repomd.xml.key");
	      MIL << "Importing key " << (_path + "/repodata/repomd.xml.key") << " (locally " << key_local << ")" << endl;
	      getZYpp()->keyRing()->importKey(key_local , false);
	    }
	    catch (const Exception & excpt_r)
	    {
	      WAR << "Failed to import key " << _path + ("/repodata/repomd.xml.key") << endl;
	      ZYPP_CAUGHT(excpt_r);
	    }
	    MIL << "Checking repomd.xml integrity" << endl;
	    Pathname asc_local;
	    try {
	      asc_local = provideFile(_path + "/repodata/repomd.xml.asc");
	    }
	    catch (const Exception & excpt_r)
	    {
	      ZYPP_CAUGHT(excpt_r);
	    }
	
	    if (! getZYpp()->keyRing()->verifyFileSignatureWorkflow(filename, asc_local))
	      ZYPP_THROW(Exception(N_("Failed check for the metadata file check sum")));
	  }

	  DBG << "Reading file " << filename << endl;
	  ifstream repo_st(filename.asString().c_str());
	  YUMRepomdParser repomd(repo_st, "");
	
	  for(;
	      ! repomd.atEnd();
	      ++repomd)
	  {
	    if (!cacheExists())
	    {
	      if (! checkCheckSum(provideFile(_path + (*repomd)->location), (*repomd)->checksumType, (*repomd)->checksum))
	      {
	        ZYPP_THROW(Exception(N_("Failed check for the metadata file check sum")));
	      }
	    }
	    if ((*repomd)->type == "patches")
	    {
              Pathname filename = cacheExists()
                  ? _cache_dir + (*repomd)->location
                  : provideFile(_path + (*repomd)->location);
	      DBG << "reading file " << filename << endl;
	      ifgzstream st ( filename.asString().c_str() );
	      YUMPatchesParser patch(st, "");
	      for (;
		  !patch.atEnd();
		  ++patch)
	      {
		string filename = (*patch)->location;
		if (!cacheExists())
		{
		  if (! checkCheckSum(provideFile(_path + filename), (*patch)->checksumType, (*patch)->checksum))
		  {
		    ZYPP_THROW(Exception(N_("Failed check for the metadata file check sum")));
		  }
		}
	      }
	    }
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
	_cache_dir = cache_dir_r;

	MIL << "Storing data to cache" << endl;
	// first read list of all files in the repository
	Pathname filename = provideFile(_path + "/repodata/repomd.xml");
	if ( ! PathInfo(filename).isExist() )
	  ZYPP_THROW(Exception("repodata/repomd.xml not found"));

	// use tmpfile because of checking integrity - provideFile might release the medium
	filesystem::TmpFile tmp;
	filesystem::copy(filename, tmp.path());
	filename = tmp.path();
	MIL << "Checking repomd.xml integrity" << endl;
	Pathname asc_local;
	try {
	  asc_local = provideFile(_path + "/repodata/repomd.xml.asc");
	}
	catch (const Exception & excpt_r)
	{
	  ZYPP_CAUGHT(excpt_r);
	}
	if (! getZYpp()->keyRing()->verifyFileSignatureWorkflow(filename, asc_local))
	  ZYPP_THROW(Exception(N_("Failed check for the metadata file check sum")));

	DBG << "Reading file " << filename << endl;
	ifstream repo_st(filename.asString().c_str());
	YUMRepomdParser repomd(repo_st, "");
	
	for(;
	      ! repomd.atEnd();
	      ++repomd)
	{
	    Pathname src = provideFile(_path + (*repomd)->location);
	    if (! checkCheckSum(src, (*repomd)->checksumType, (*repomd)->checksum))
	    {
	        ZYPP_THROW(Exception(N_("Failed check for the metadata file check sum")));
	    }
	    Pathname dst = cache_dir_r + (*repomd)->location;
	    if (0 != assert_dir(dst.dirname(), 0700))
	      ZYPP_THROW(Exception("Cannot create cache directory"));
	    filesystem::copy(src, dst);
	    if ((*repomd)->type == "patches")
	    {
	      Pathname filename = provideFile(_path + (*repomd)->location);
	      DBG << "reading file " << filename << endl;
	      ifgzstream st ( filename.asString().c_str() );
	      YUMPatchesParser patch(st, "");
	      for (;
		  !patch.atEnd();
		  ++patch)
	      {
		string filename = (*patch)->location;
		src = provideFile(_path + filename);
		if (! checkCheckSum(src, (*patch)->checksumType, (*patch)->checksum))
		{
		    ZYPP_THROW(Exception(N_("Failed check for the metadata file check sum")));
		}
		dst = cache_dir_r + filename;
		if (0 != assert_dir(dst.dirname(), 0700))
		    ZYPP_THROW(Exception("Cannot create cache directory"));
		filesystem::copy(src, dst);
	      }
	    }
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

	callback::SendReport<CreateSourceReport> report;
  
	report->startData( url() );

    //---------------------------------
    // repomd

      try {
	  // first read list of all files in the repository
        Pathname filename = cacheExists()
          ? _cache_dir + "/repodata/repomd.xml"
          : provideFile(_path + "/repodata/repomd.xml");
	  _metadata_files.push_back("/repodata/repomd.xml");
	  // use tmpfile because of checking integrity - provideFile might release the medium
	  filesystem::TmpFile tmp;
	  filesystem::copy(filename, tmp.path());
	  filename = tmp.path();
	  if (!cacheExists())
	  {
	    MIL << "Checking repomd.xml integrity" << endl;
	    Pathname asc_local;
	    try {
	      asc_local = provideFile(_path + "/repodata/repomd.xml.asc");
	    }
	    catch (Exception & excpt_r)
	    {
	      ZYPP_CAUGHT(excpt_r);
	    }
	    if (! getZYpp()->keyRing()->verifyFileSignatureWorkflow(filename, asc_local))
	      ZYPP_THROW(Exception(N_("Failed check for the metadata file check sum")));
	  }

	  DBG << "Reading ifgz file " << filename << endl;
	  ifgzstream repo_st(filename.asString().c_str());
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

    //---------------------------------
    // files mentioned within repomd

      try {
	  // now put other and filelist data to structures for easier find
	  map<NVRA, YUMFileListData_Ptr> files_data;
	  map<NVRA, YUMOtherData_Ptr> other_data;
	  for (std::list<YUMRepomdData_Ptr>::const_iterator it
		  = repo_files.begin();
	      it != repo_files.end();
	      it++)
	  {
            Pathname filename = cacheExists()
              ? _cache_dir + (*it)->location
              :  provideFile(_path + (*it)->location);
            if (!cacheExists())
	    {
	      if (! checkCheckSum(filename, (*it)->checksumType, (*it)->checksum))
	      {
	        ZYPP_THROW(Exception(N_("Failed check for the metadata file check sum")));
	      }
	    }
	    _metadata_files.push_back((*it)->location);
	    DBG << "Reading ifgz file " << filename << endl;

	    ifgzstream st( filename.asString().c_str() );

	    YUMFileListParser filelist ( st, "" );
	    for (;
		  ! filelist.atEnd();
		  ++filelist)
	    {
		if (*filelist == NULL) continue;	// skip incompatible archs

		NVRA nvra( (*filelist)->name,
			   Edition( (*filelist)->ver, (*filelist)->rel, str::strtonum<int>( (*filelist)->epoch ) ),
			   Arch ( (*filelist)->arch ) );
		files_data[nvra] = *filelist;
	    }
	    if (filelist.errorStatus())
	      throw *filelist.errorStatus();
	  }

	  for (std::list<YUMRepomdData_Ptr>::const_iterator it
		  = repo_other.begin();
	      it != repo_other.end();
	      it++)
	  {
            Pathname filename = cacheExists()
              ? _cache_dir + (*it)->location
              : provideFile(_path + (*it)->location);
            if (!cacheExists())
	    {
	      if (! checkCheckSum(filename, (*it)->checksumType, (*it)->checksum))
	      {
	        ZYPP_THROW(Exception(N_("Failed check for the metadata file check sum")));
	      }
	    }
	    _metadata_files.push_back((*it)->location);
	    DBG << "Reading file " << filename << endl;

	    ifgzstream st ( filename.asString().c_str() );
	    YUMOtherParser other(st, "");
	    for (;
		  ! other.atEnd();
		  ++other)
	    {
                Arch arch;
                if (!(*other)->arch.empty())
                  arch = Arch((*other)->arch);
              
		NVRA nvra( (*other)->name,
			   Edition( (*other)->ver, (*other)->rel, str::strtonum<int>( (*other)->epoch ) ),
			   arch );
		other_data[nvra] = *other;
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
          Pathname filename = cacheExists()
              ? _cache_dir + (*it)->location
            : provideFile(_path + (*it)->location);
          if (!cacheExists())
	    {
	      if (! checkCheckSum(filename, (*it)->checksumType, (*it)->checksum))
	      {
	        ZYPP_THROW(Exception(N_("Failed check for the metadata file check sum")));
	      }
	    }
	    _metadata_files.push_back((*it)->location);
	    DBG << "Reading file " << filename << endl;
	    ifgzstream st ( filename.asString().c_str() );
	    YUMPrimaryParser prim(st, "");
	    for (;
		  !prim.atEnd();
		  ++prim)
	    {
	      if (*prim == NULL) continue;	// incompatible arch detected during parsing

//              MIL << "found package "<< (*prim)->name << std::endl;
              
                Arch arch;
                if (!(*prim)->arch.empty())
                  arch = Arch((*prim)->arch);
              
		NVRA nvra( (*prim)->name,
			   Edition( (*prim)->ver, (*prim)->rel, str::strtonum<int>( (*prim)->epoch ) ),
			   arch );
		map<NVRA, YUMOtherData_Ptr>::iterator found_other
		    = other_data.find( nvra );
		map<NVRA, YUMFileListData_Ptr>::iterator found_files
		    = files_data.find( nvra );
  
		YUMFileListData filelist_empty;
		YUMOtherData other_empty;
		ResImplTraits<YUMPackageImpl>::Ptr impl;
		Package::Ptr p = createPackage(
		  source_r,
		  **prim,
		  found_files != files_data.end()
		    ? *found_files->second
		    : filelist_empty,
		  found_other != other_data.end()
		    ? *found_other->second
	       	     : other_empty,
		  impl
		);
		ImplAndPackage iap = { impl, p };
		_package_impl[nvra] = iap;
//                MIL << "inserting package "<< p->name() << std::endl;
		_store.insert (p);
	    }
	    if (prim.errorStatus())
		throw *prim.errorStatus();
	}
    }
    catch (...) {
	ERR << "Cannot read package information" << endl;
    }

    //---------------------------------
    // groups
    try {
	for (std::list<YUMRepomdData_Ptr>::const_iterator it = repo_group.begin();
	      it != repo_group.end();
	      it++)
	{
          Pathname filename = cacheExists()
              ? _cache_dir + (*it)->location
            : provideFile(_path + (*it)->location);
	    if (!cacheExists())
	    {
	      if (! checkCheckSum(filename, (*it)->checksumType, (*it)->checksum))
	      {
	        ZYPP_THROW(Exception(N_("Failed check for the metadata file check sum")));
	      }
	    }
	    _metadata_files.push_back((*it)->location);
	    DBG << "Reading file " << filename << endl;
	    ifgzstream st ( filename.asString().c_str() );
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

    //---------------------------------
    // patterns

    try {
	for (std::list<YUMRepomdData_Ptr>::const_iterator it = repo_pattern.begin();
	      it != repo_pattern.end();
	      it++)
	{
          Pathname filename = cacheExists()
              ? _cache_dir + (*it)->location
            : provideFile(_path + (*it)->location);
	    if (!cacheExists())
	    {
	      if (! checkCheckSum(filename, (*it)->checksumType, (*it)->checksum))
	      {
	        ZYPP_THROW(Exception(N_("Failed check for the metadata file check sum")));
	      }
	    }
	    _metadata_files.push_back((*it)->location);
	    DBG << "Reading file " << filename << endl;
	    ifgzstream st ( filename.asString().c_str() );
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


    //---------------------------------
    // products
    try {
	for (std::list<YUMRepomdData_Ptr>::const_iterator it = repo_product.begin();
	      it != repo_product.end();
	      it++)
	{
          Pathname filename = cacheExists()
              ? _cache_dir + (*it)->location
            : provideFile(_path + (*it)->location);
	    if (!cacheExists())
	    {
	      if (! checkCheckSum(filename, (*it)->checksumType, (*it)->checksum))
	      {
	        ZYPP_THROW(Exception(N_("Failed check for the metadata file check sum")));
	      }
	    }
	    _metadata_files.push_back((*it)->location);
	    DBG << "Reading file " << filename << endl;
	    ifgzstream st ( filename.asString().c_str() );
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

    //---------------------------------
    // patches, first the patches.xml index
    try {
	  std::list<std::string> patch_files;
	  for (std::list<YUMRepomdData_Ptr>::const_iterator it = repo_patches.begin();
	      it != repo_patches.end();
	      it++)
	  {
            Pathname filename = cacheExists()
                ? _cache_dir + (*it)->location
              : provideFile(_path + (*it)->location);
	    if (!cacheExists())
	    {
	      if (! checkCheckSum(filename, (*it)->checksumType, (*it)->checksum))
	      {
	        ZYPP_THROW(Exception(N_("Failed check for the metadata file check sum")));
	      }
	    }
	    _metadata_files.push_back((*it)->location);
	    DBG << "Reading file " << filename << endl;
	    ifgzstream st ( filename.asString().c_str() );
	    YUMPatchesParser patch(st, "");
	    for (;
		  !patch.atEnd();
		  ++patch)
	    {
		string filename = (*patch)->location;
                if (!cacheExists())
		{
		  if (! checkCheckSum(provideFile(_path + filename), (*patch)->checksumType, (*patch)->checksum))
		  {
		    ZYPP_THROW(Exception(N_("Failed check for the metadata file check sum")));
		  }
		}
		patch_files.push_back(filename);
	    }
	    if (patch.errorStatus())
		throw *patch.errorStatus();
	}

	//---------------------------------
	// now the individual patch files

	for (std::list<std::string>::const_iterator it = patch_files.begin();
	  it != patch_files.end();
	  it++)
	{
          Pathname filename = cacheExists()
              ? _cache_dir + *it
            : provideFile(_path + *it);
	    _metadata_files.push_back(*it);
	    DBG << "Reading file " << filename << endl;
	    ifgzstream st ( filename.asString().c_str() );
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

    report->finishData( url(), CreateSourceReport::NO_ERROR, "" );
  }


  Package::Ptr YUMSourceImpl::createPackage(
    Source_Ref source_r,
    const zypp::parser::yum::YUMPrimaryData & parsed,
    const zypp::parser::yum::YUMFileListData & filelist,
    const zypp::parser::yum::YUMOtherData & other,
    ResImplTraits<YUMPackageImpl>::Ptr & impl
  )
  {
    try
    {
      impl = new YUMPackageImpl( source_r, parsed, filelist, other );

      Arch arch;
      if (!parsed.arch.empty())
        arch = Arch(parsed.arch);
      
      // Collect basic Resolvable data
      NVRAD dataCollect( parsed.name,
			 Edition( parsed.ver, parsed.rel, parsed.epoch ),
			 arch,
			 createDependencies( parsed,
					   ResTraits<Package>::kind )
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

  Atom::Ptr YUMSourceImpl::augmentPackage(
    Source_Ref source_r,
    const zypp::parser::yum::YUMPatchPackage & parsed
  )
  {
    try
    {
        Arch arch;
        if (!parsed.arch.empty())
          arch = Arch(parsed.arch);
      
	Edition edition( parsed.ver, parsed.rel, parsed.epoch );
	NVRA nvra( parsed.name,
		   edition,
		   arch );

	DBG << "augmentPackage(" << nvra << ")" << endl;
        
        // create Atom
        CapFactory f;
        Dependencies deps = createDependencies( parsed, ResTraits<Package>::kind );
//        deps[Dep::REQUIRES].insert( f.parse( ResTraits<Package>::kind, parsed.name, Rel::EQ, edition ) );
        NVRAD atomdata( nvra, deps );
        ResImplTraits<YUMAtomImpl>::Ptr atomimpl = new YUMAtomImpl( source_r );
        Atom::Ptr atom = detail::makeResolvableFromImpl( atomdata, atomimpl);
        
        //source_r
	PackageImplMapT::const_iterator it = _package_impl.find( nvra );
	if (it == _package_impl.end()) {
	    ERR << "Patch augments non-existant package " << nvra << endl;
	}
        else
        {
          ResImplTraits<YUMPackageImpl>::Ptr impl = it->second.impl;
          Package::Ptr package = it->second.package;
          //DBG << "found " << *package << ", impl " << impl << endl;
  
          _store.erase( package );
          impl->unmanage();
   
          // Atoms are inserted in the store after patch creation
          //DBG << "Inserting atom " << *atom << endl;
          //DBG << "with deps " << deps << endl;
          //_store.insert( atom );
  
          // Collect augmented package data
          NVRAD packagedata( nvra, package->deps() );
  
          if (!parsed.location.empty()) {
              impl->_location = parsed.location;
              impl->_mediaid = str::strtonum<unsigned>( parsed.media );
          }
	  impl->_install_only = parsed.installOnly;
  //	if (!parsed.plainRpms.empty()) impl->_plain_rpms = parsed.plainRpms;
  #warning Needs equal PatchRpm and DeltaRpm definition in parser/yum and source/yum
  #if 0
          if (!parsed.patchRpms.empty()) impl->_patch_rpms = parsed.patchRpms;
          if (!parsed.deltaRpms.empty()) impl->_delta_rpms = parsed.deltaRpms;
  #endif
          //DBG << "NVRAD " << (NVRA)packagedata << endl;
  
  
  
  #warning add patchrpm, deltarpm, etc. to YUMPackageImpl here
          Package::Ptr new_package = detail::makeResolvableFromImpl(
              packagedata, impl
          );
  
          //DBG << "new_package " << *new_package << endl;
  
          _store.insert( new_package );
        }
	return atom;
    }
    catch (const Exception & excpt_r)
    {
      ERR << excpt_r << endl;
      ZYPP_CAUGHT( excpt_r );
      throw "Cannot create augmented package object";
    }
  }

  Selection::Ptr YUMSourceImpl::createGroup(
    Source_Ref source_r,
    const zypp::parser::yum::YUMGroupData & parsed
  )
  {
    try
    {
      ResImplTraits<YUMGroupImpl>::Ptr impl(new YUMGroupImpl(source_r, parsed));
      // Collect basic Resolvable data
      NVRAD dataCollect( parsed.groupId,
			 Edition::noedition,			// group has just a name,
			 Arch_noarch,				//   pattern has edition & arch
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
      ResImplTraits<YUMPatternImpl>::Ptr impl(new YUMPatternImpl(source_r, parsed));
      // Collect basic Resolvable data
      Arch arch;
      if (!parsed.arch.empty())
        arch = Arch(parsed.arch);
      
      NVRAD dataCollect( parsed.name,
			 Edition( parsed.ver, parsed.rel, parsed.epoch ),
			 arch,
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
    const zypp::parser::yum::YUMPatchMessage & parsed,
    Patch::constPtr patch
  )
  {
    try
    {
      ResImplTraits<YUMMessageImpl>::Ptr impl(new YUMMessageImpl(source_r, parsed, patch));
      Arch arch;
      if (!parsed.arch.empty())
        arch = Arch(parsed.arch);
      // Collect basic Resolvable data
      NVRAD dataCollect( parsed.name,
			      Edition( parsed.ver, parsed.rel, parsed.epoch ),
			      arch,
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
      ResImplTraits<YUMScriptImpl>::Ptr impl(new YUMScriptImpl(source_r, parsed));
      Arch arch;
      if (!parsed.arch.empty())
        arch = Arch(parsed.arch);
      // Collect basic Resolvable data
      NVRAD dataCollect( parsed.name,
		      Edition( parsed.ver, parsed.rel, parsed.epoch ),
		      arch,
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
      ResImplTraits<YUMProductImpl>::Ptr impl(new YUMProductImpl(source_r, parsed));

	    // Collect basic Resolvable data
      Arch arch;
      if (!parsed.arch.empty())
        arch = Arch(parsed.arch);
      NVRAD dataCollect( parsed.name,
			      Edition( parsed.ver, parsed.rel, parsed.epoch ),
			      arch,
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
      ResImplTraits<YUMPatchImpl>::Ptr impl(new YUMPatchImpl(source_r, parsed, *this));

      Arch arch;
      if (!parsed.arch.empty())
        arch = Arch(parsed.arch);
      
      // Collect basic Resolvable data
      NVRAD dataCollect( parsed.name,
		      Edition( parsed.ver, parsed.rel, parsed.epoch ),
		      arch,
		      createDependencies( parsed,
					  ResTraits<Patch>::kind)
		    );
      Patch::Ptr patch = detail::makeResolvableFromImpl(
	dataCollect, impl
      );
	// now process the atoms
	CapFactory _f;
	Capability cap( _f.parse(
	  Patch::TraitsType::kind,
	  parsed.name,
	  Rel::EQ,
	  Edition(parsed.ver, parsed.rel, parsed.epoch)
	  ));
	for (std::list<shared_ptr<YUMPatchAtom> >::const_iterator it
					= parsed.atoms.begin();
	     it != parsed.atoms.end();
	     it++)
	{
	  switch ((*it)->atomType())
	  {
	    case YUMPatchAtom::Package: {
	      shared_ptr<YUMPatchPackage> package_data
		= dynamic_pointer_cast<YUMPatchPackage>(*it);
              Atom::Ptr atom = augmentPackage(source_r, *package_data );
              impl->_atoms.push_back(atom);
	      break;
	    }
	    case YUMPatchAtom::Message: {
	      shared_ptr<YUMPatchMessage> message_data
		= dynamic_pointer_cast<YUMPatchMessage>(*it);
	      Message::Ptr message = createMessage(source_r, *message_data, patch);
	      impl->_atoms.push_back(message);
	      break;
	    }
	    case YUMPatchAtom::Script: {
	      shared_ptr<YUMPatchScript> script_data
		= dynamic_pointer_cast<YUMPatchScript>(*it);
	      Script::Ptr script = createScript(source_r, *script_data);
	      impl->_atoms.push_back(script);
	      break;
	    }
	    default:
	      ERR << "Unknown type of atom" << endl;
	  }
	  for (Patch::AtomList::iterator it = impl->_atoms.begin();
	       it != impl->_atoms.end();
	       it++)
	  {
	    (*it)->injectRequires(cap);
	  }
	}
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

    for (std::list<YUMDependency>::const_iterator it = parsed.freshens.begin();
	it != parsed.freshens.end();
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

    for (std::list<YUMDependency>::const_iterator it = parsed.supplements.begin();
	it != parsed.supplements.end();
	it++)
    {
      _deps[Dep::SUPPLEMENTS].insert(createCapability(*it, my_kind));
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
      Dep _dep_kind = Dep::REQUIRES;
      if (it->type == "mandatory" || it->type == "")
      {
	_dep_kind = Dep::REQUIRES;
      }
      else if (it->type == "default")
      {
	_dep_kind = Dep::RECOMMENDS;
      }
      else if (it->type == "optional")
      {
	_dep_kind = Dep::SUGGESTS;
      }
      _deps[_dep_kind].insert(createCapability(YUMDependency(
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
    for (std::list<MetaPkg>::const_iterator it = parsed.grouplist.begin();
      it != parsed.grouplist.end();
      it++)
    {
      Dep _dep_kind = Dep::REQUIRES;
      if (it->type == "mandatory" || it->type == "")
      {
	_dep_kind = Dep::REQUIRES;
      }
      else if (it->type == "default")
      {
	_dep_kind = Dep::RECOMMENDS;
      }
      else if (it->type == "optional")
      {
	_dep_kind = Dep::SUGGESTS;
      }
      _deps[_dep_kind].insert(createCapability(YUMDependency(
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




      bool YUMSourceImpl::checkCheckSum (const Pathname & filename, std::string csum_type, const std::string & csum)
      {
	MIL << "Checking checksum for " << filename << " as type: " << csum_type << "; value: " << csum << endl;
	if (str::toLower(csum_type) == "sha")
	{
	  if (csum.size() == 40)
	    csum_type = "sha1";
	  else if (csum.size() == 64)
	    csum_type = "sha256";
	  DBG << "Checksum size is " << csum.size() << ", checksum type set to " << csum_type << endl;
	}
	ifstream st(filename.asString().c_str());
	std::string dig = Digest::digest (csum_type, st, 4096);
	if (dig == "")
	{
	  ERR << "Cannot compute the checksum" << endl;
	  return true;
#warning TODO Define behavior if checksum computing fails
	}
	dig = str::toLower (dig);
	bool ret = (dig == str::toLower(csum));
	if (ret)
	  MIL << "Checksums are the same" << endl;
	else
	  WAR << "Checksum missmatch: metadata: " << csum << "; real: " << dig << endl;
	return true;
#warning remove line above, ask user what to do...
	return ret;
      }

    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
