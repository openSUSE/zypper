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
#include "zypp/SilentCallbacks.h"

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

      Date YUMSourceImpl::timestamp() const
      {
        return PathInfo(repomdFile()).mtime();
      }
      
      bool YUMSourceImpl::cacheExists()
      {
        bool exists = PathInfo(repomdFile()).isExist();
        if (exists)
          MIL << "YUM cache found at " << _cache_dir << std::endl;
        else
          MIL << "YUM cache not found" << std::endl;

        return exists;
      }
      
      const Pathname YUMSourceImpl::metadataRoot() const
      {
        return _cache_dir.empty() ? _tmp_metadata_dir : _cache_dir;
      }
      
      const Pathname YUMSourceImpl::repomdFile() const
      {
        return metadataRoot() + "/repodata/repomd.xml";
      }
      
      const Pathname YUMSourceImpl::repomdFileSignature() const
      {
        return metadataRoot() + "/repodata/repomd.xml.asc";
      }
      
      const Pathname YUMSourceImpl::repomdFileKey() const
      {
        return metadataRoot() + "/repodata/repomd.xml.key";
      }
      
      const TmpDir YUMSourceImpl::downloadMetadata()
      {
        TmpDir tmpdir;
        int copy_result;
        MIL << "Downloading metadata to " << tmpdir.path() << std::endl;
        
        Pathname local_dir = tmpdir.path();
        if (0 != assert_dir(local_dir + "/repodata" , 0755))
          ZYPP_THROW(Exception("Cannot create /repodata in download directory"));
        
        MIL << "Storing data to tmp dir " << local_dir << endl;
        
        // first read list of all files in the repository
        Pathname remote_repomd;
        try
        {
          remote_repomd = provideFile(_path + "/repodata/repomd.xml");
        }
        catch(Exception &e)
        {
          ZYPP_THROW(Exception("Can't provide " + _path.asString() + "/repodata/repomd.xml from " + url().asString() ));
        }

        // provide optional files
        Pathname remote_repomd_key;
        Pathname remote_repomd_signature;
        try {
          remote_repomd_key = tryToProvideFile( _path + "/repodata/repomd.xml.key");
        }
        catch( const Exception &e ) {
          WAR << "Repository does not contain repomd signing key" << std::endl;
        }
        
        try {
          remote_repomd_signature = tryToProvideFile( _path + "/repodata/repomd.xml.asc");
        }
        catch( const Exception &e ) {
          WAR << "Repository does not contain repomd signinature" << std::endl;
        }
        
        copy_result = filesystem::copy( remote_repomd, local_dir + "/repodata/repomd.xml");
        if ( copy_result != 0 )
          ZYPP_THROW(Exception("Can't copy " + remote_repomd.asString() + " to " + local_dir.asString() + "/repodata/repomd.xml"));
          
        if (PathInfo(remote_repomd_key).isExist())
        {
          copy_result = filesystem::copy( remote_repomd_key, local_dir + "/repodata/repomd.xml.key");
          if ( copy_result != 0 )
            ZYPP_THROW(Exception("Can't copy " + remote_repomd_key.asString() + " to " + local_dir.asString() + "/repodata/repomd.xml.key"));
          getZYpp()->keyRing()->importKey(local_dir + "/repodata/repomd.xml.key" , false);
        }
        
        if (PathInfo(remote_repomd_signature).isExist())
        {  
          copy_result = filesystem::copy( remote_repomd_signature, local_dir + "/repodata/repomd.xml.asc");
          if ( copy_result != 0 )
            ZYPP_THROW(Exception("Can't copy " + remote_repomd_signature.asString() + " to " + local_dir.asString() + "/repodata/repomd.xml.asc"));
        }
        
        DBG << "Reading file " << remote_repomd << endl;
        ifstream repo_st(remote_repomd.asString().c_str());
        YUMRepomdParser repomd(repo_st, "");

        for(; ! repomd.atEnd(); ++repomd)
        {
          if ((*repomd)->type == "other")     // don't parse 'other.xml' (#159316)
            continue;

          Pathname src;
          try
          {
            src = provideFile(_path + (*repomd)->location);
          }
          catch (const Exception &e)
          {
            ZYPP_THROW(Exception("Can't provide " + _path.asString() + (*repomd)->location + " from " + url().asString() ));
          }

          Pathname dst = local_dir + (*repomd)->location;
          
          //if (0 != assert_dir(dst, 0755))
          //  ZYPP_THROW(Exception("Cannot create directory: " + dst.asString()));
          
          if ( filesystem::copy(src, dst) != 0 )
            ZYPP_THROW(Exception("Can't copy " + src.asString() + " to " + dst.asString()));
          
          if (! checkCheckSum( dst, (*repomd)->checksumType, (*repomd)->checksum))
            ZYPP_THROW(Exception( (*repomd)->location + " " + N_(" fails checksum verification.") ));
          
            
          // if it is a patch, we read the patches individually
          if ((*repomd)->type == "patches")
          {
            // use the local copy now
            Pathname patches_list = dst;
            MIL << "Reading patches file " << patches_list << std::endl;
            ifgzstream st ( patches_list.asString().c_str() );
            YUMPatchesParser patch(st, "");
            for (; !patch.atEnd(); ++patch)
            {
              string filename = (*patch)->location;
              Pathname patch_src;
              Pathname patch_dst;
              try
              {
                patch_src = provideFile(_path + filename);
              }
              catch (const Exception &e)
              {
                ZYPP_CAUGHT(e);
                ZYPP_THROW(Exception("Can't provide patch " + _path.asString() + (*repomd)->location + " from " + url().asString()));
              }
              
              patch_dst = local_dir + filename;
                
              if ( filesystem::copy(patch_src, patch_dst) != 0 )
                ZYPP_THROW(Exception("Can't copy patch file " + patch_src.asString() + " to " + patch_dst.asString()));
                
              // check patch checksum
              if (! checkCheckSum( patch_dst, (*patch)->checksumType, (*patch)->checksum))
                ZYPP_THROW(Exception( (*repomd)->location + " " + N_(" fails checksum verification.") ));
            } // end of single patch parsing
          }// end of patches file parsing
        } // end of copying
        
        // check signature
        MIL << "Checking [" << (local_dir + "/repodata/repomd.xml") << "] signature"  << endl;
        if (! getZYpp()->keyRing()->verifyFileSignatureWorkflow(local_dir + "/repodata/repomd.xml", (_path + "/repodata/repomd.xml").asString()+ " (" + url().asString() + ")", local_dir + "/repodata/repomd.xml.asc"))
          ZYPP_THROW(Exception(N_("Signed repomd.xml file fails signature check")));
        
        // ok, now we have a consistent repo in the tmpdir.
        return tmpdir;
      }
      
      void YUMSourceImpl::factoryInit()
      {
        try
        {
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
        
        bool cache = cacheExists();
        if ( cache )
        {
          DBG << "Cached metadata found in [" << _cache_dir << "]." << endl;
        }
        else
        {
          if ( _cache_dir.empty() || !PathInfo(_cache_dir).isExist() )
          {
            DBG << "Cache dir not set. Downloading to temp dir: " << _tmp_metadata_dir << std::endl;
            // as we have no local dir set we use a tmp one, but we use a member variable because
            // it cant go out of scope while the source exists.
            storeMetadata(_tmp_metadata_dir);
          }
          else
          {
            DBG << "Cached metadata not found in [" << _cache_dir << "]. Will download." << std::endl;
            storeMetadata(_cache_dir);
          }
        }
      }

      void YUMSourceImpl::checkMetadataChecksums() const
      {
        DBG << "Reading file " << repomdFile() << " to check integrity of metadata." << endl;
        ifstream repo_st(repomdFile().asString().c_str());
        YUMRepomdParser repomd(repo_st, "");

        for(; ! repomd.atEnd(); ++repomd)
        {
          if ((*repomd)->type == "other")
          {             // dont parse other.xml (#159316)
            continue;
          }
          else
          {
            Pathname file_to_check = metadataRoot() + _path + (*repomd)->location;
            if (! checkCheckSum( file_to_check, (*repomd)->checksumType, (*repomd)->checksum))
            {
              ZYPP_THROW(Exception( (*repomd)->location + " " + N_("fails checksum verification.") ));
            }

            // now parse patches mentioned if we are in patches.xml

            if ((*repomd)->type == "patches")
            {
              Pathname patch_index = file_to_check;
              DBG << "reading patches from file " << patch_index << endl;
              ifgzstream st ( patch_index.asString().c_str() );
              YUMPatchesParser patch(st, "");
              for (; !patch.atEnd(); ++patch)
              {
                Pathname patch_filename = metadataRoot() + _path + (*patch)->location;
                if (! checkCheckSum(patch_filename, (*patch)->checksumType, (*patch)->checksum))
                {
                  ZYPP_THROW(Exception( (*patch)->location + " " + N_("fails checksum verification.") ));
                }
              }
            }
          }
        }
      }

      bool YUMSourceImpl::downloadNeeded()
      {
        // we can only assume repomd intact means the source changed if the source is signed.
        if ( cacheExists() && PathInfo( repomdFileSignature() ).isExist() )
        {
          Pathname remote_repomd;
          try
          {
            remote_repomd = provideFile(_path + "/repodata/repomd.xml");
          }
          catch(Exception &e)
          {
            ZYPP_THROW(Exception("Can't provide " + _path.asString() + "/repodata/repomd.xml from " + url().asString() ));
          }
          
          CheckSum old_repomd_checksum( "SHA1", filesystem::sha1sum(repomdFile()));
          CheckSum new_repomd_checksum( "SHA1", filesystem::sha1sum(remote_repomd));
          if ( (new_repomd_checksum == old_repomd_checksum) && (!new_repomd_checksum.empty()) && (! old_repomd_checksum.empty()))
          {
            return false;
          }
        }
        return true;
      }
      
      void YUMSourceImpl::storeMetadata(const Pathname & cache_dir_r)
      {
        TmpDir download_tmp_dir;
        
        bool need_to_refresh = true;
        try
        {
          need_to_refresh = downloadNeeded();
        }
        catch(Exception &e)
        {
          ZYPP_THROW(Exception("Can't check if source has changed or not. Aborting refresh."));
        }
        
        if ( need_to_refresh )
        {
          MIL << "YUM source " << alias() << "has changed since last download. Re-reading metadata into " << cache_dir_r << endl;
        }
        else
        {
          MIL << "YUM source " << alias() << "has not changed. Refresh completed. SHA1 of repomd.xml file is  the same." << std::endl;    
          return;
        }
        
        try
        {
          download_tmp_dir = downloadMetadata();
        }
        catch(Exception &e)
        {
          ZYPP_THROW(Exception("Downloading metadata failed (is YUM source?) or user did not accept remote source. Aborting refresh."));
        }
        
        // refuse to use stupid paths as cache dir
        if (cache_dir_r == Pathname("/") )
          ZYPP_THROW(Exception("I refuse to use / as cache dir"));

        if (0 != assert_dir(cache_dir_r, 0755))
          ZYPP_THROW(Exception("Cannot create cache directory" + cache_dir_r.asString()));

        MIL << "Cleaning up cache dir" << std::endl;
        filesystem::clean_dir(cache_dir_r);
        MIL << "Copying " << download_tmp_dir << " content to cache : " << cache_dir_r << std::endl;
       
        if ( copy_dir_content( download_tmp_dir, cache_dir_r) != 0)
        {
          filesystem::clean_dir(cache_dir_r);
          ZYPP_THROW(Exception( "Can't copy downloaded data to cache dir. Cache cleaned."));
        }
        // download_tmp_dir go out of scope now but it is ok as we already copied the content.
      }

      void YUMSourceImpl::createResolvables(Source_Ref source_r)
      {
	std::list<YUMRepomdData_Ptr> repo_primary;
	std::list<YUMRepomdData_Ptr> repo_files;
	//std::list<YUMRepomdData_Ptr> repo_other;
	std::list<YUMRepomdData_Ptr> repo_group;
	std::list<YUMRepomdData_Ptr> repo_pattern;
	std::list<YUMRepomdData_Ptr> repo_product;
	std::list<YUMRepomdData_Ptr> repo_patches;

	callback::SendReport<CreateSourceReport> report;

	report->startData( url() );

        //---------------------------------
        // repomd
        
        try
        {
          DBG << "Reading ifgz file " << repomdFile() << endl;
          ifgzstream repo_st(repomdFile().asString().c_str());
	  YUMRepomdParser repomd(repo_st, "");
          for(; ! repomd.atEnd(); ++repomd)
	  {
            // note that we skip adding other.xml to the list of files to provide
	    if ((*repomd)->type == "primary")
	      repo_primary.push_back(*repomd);
	    else if ((*repomd)->type == "filelists")
	      repo_files.push_back(*repomd);
	    else if ((*repomd)->type == "group")
	      repo_group.push_back(*repomd);
	    else if ((*repomd)->type == "pattern")
	      repo_pattern.push_back(*repomd);
	    else if ((*repomd)->type == "product")
	      repo_product.push_back(*repomd);
	    else if ((*repomd)->type == "patches")
	      repo_patches.push_back(*repomd);
	    else if ((*repomd)->type != "other")	// type "other" is ok, anything else not
	      ERR << "Unknown type of repo file: " << (*repomd)->type << endl;
	   }
        }
        catch( const Exception &  excpt_r )
        {
	  ZYPP_CAUGHT( excpt_r );	// log the caught exception
	  ZYPP_THROW( Exception("Cannot read repomd file, cannot initialize source") );
        }

        //---------------------------------
        // files mentioned within repomd

        try
        {
	  // now put other and filelist data to structures for easier find
	  map<NVRA, YUMFileListData_Ptr> files_data;
	  map<NVRA, YUMOtherData_Ptr> other_data;
	  for (std::list<YUMRepomdData_Ptr>::const_iterator it
		  = repo_files.begin();
	      it != repo_files.end();
	      it++)
	  {
            Pathname filename = metadataRoot() + (*it)->location;
	    DBG << "Reading ifgz file " << filename << endl;
	    ifgzstream st( filename.asString().c_str() );

	    YUMFileListParser filelist ( st, "" );
	    for (; ! filelist.atEnd(); ++filelist)
	    {
              if (*filelist == NULL) continue;	// skip incompatible archs
		NVRA nvra( (*filelist)->name,
			   Edition( (*filelist)->ver, (*filelist)->rel, str::strtonum<int>( (*filelist)->epoch ) ),
			   Arch ( (*filelist)->arch ) );
		files_data[nvra] = *filelist;
	    }
	    if (filelist.errorStatus())
              ZYPP_THROW(Exception(filelist.errorStatus()->msg()));
	  }

#if 0	// don't parse 'other.xml' (#159316)

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
		if (*other == NULL) continue;	// skip incompatible archs
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
#endif

	// now read primary data, merge them with filelist and changelog
	  for (std::list<YUMRepomdData_Ptr>::const_iterator it = repo_primary.begin(); it != repo_primary.end(); it++)
          {
            Pathname filename = metadataRoot() + (*it)->location;
            DBG << "Reading file " << filename << endl;
	    ifgzstream st ( filename.asString().c_str() );
	    YUMPrimaryParser prim(st, "");
	    for (; !prim.atEnd(); ++prim)
	    {
	      if (*prim == NULL) continue;	// incompatible arch detected during parsing
              
              Arch arch;
              if (!(*prim)->arch.empty())
                arch = Arch((*prim)->arch);

              NVRA nvra( (*prim)->name,
			   Edition( (*prim)->ver, (*prim)->rel, str::strtonum<int>( (*prim)->epoch ) ),
			   arch );
              map<NVRA, YUMOtherData_Ptr>::iterator found_other = other_data.find( nvra );
              map<NVRA, YUMFileListData_Ptr>::iterator found_files = files_data.find( nvra );

              YUMFileListData filelist_empty;
              YUMOtherData other_empty;
              ResImplTraits<YUMPackageImpl>::Ptr impl;
              Package::Ptr p = createPackage( source_r, **prim, found_files != files_data.end()
		    ? *(found_files->second)
		    : filelist_empty,
		  found_other != other_data.end()
		    ? *(found_other->second)
	       	     : other_empty,
		  impl
		);
		ImplAndPackage iap = { impl, p };
		_package_impl[nvra] = iap;
//                MIL << "inserting package "<< p->name() << std::endl;
		_store.insert (p);
	     }
	     if (prim.errorStatus())
              ZYPP_THROW(Exception(prim.errorStatus()->msg()));
	   }
        }
        catch (...)
        {
	 ERR << "Cannot read package information" << endl;
        }

        //---------------------------------
        // groups
        try
        {
          for (std::list<YUMRepomdData_Ptr>::const_iterator it = repo_group.begin();
	        it != repo_group.end();
	        it++)
          {
            Pathname filename = metadataRoot() + (*it)->location;
            DBG << "Reading file " << filename << endl;
	    ifgzstream st ( filename.asString().c_str() );
	    YUMGroupParser group(st, "");
	    for (; !group.atEnd(); ++group)
	    {
              Selection::Ptr p = createGroup( source_r, **group );
              _store.insert (p);
	    }
	    if (group.errorStatus())
              ZYPP_THROW(Exception(group.errorStatus()->msg()));
          }
        }
        catch (...)
        {
          ERR << "Cannot read package groups information" << endl;
        }

        //---------------------------------
        // patterns

        try
        {
          for (std::list<YUMRepomdData_Ptr>::const_iterator it = repo_pattern.begin();
               it != repo_pattern.end(); it++)
          {
            Pathname filename = metadataRoot() + (*it)->location;

	    DBG << "Reading file " << filename << endl;
	    ifgzstream st ( filename.asString().c_str() );
	    YUMPatternParser pattern(st, "");
	    for (; !pattern.atEnd(); ++pattern)
	    {
              Pattern::Ptr p = createPattern( source_r, **pattern );
              _store.insert (p);
	    }
	    if (pattern.errorStatus())
              ZYPP_THROW(Exception(pattern.errorStatus()->msg()));
          }
        }
        catch (...) {
	  ERR << "Cannot read installation patterns information" << endl;
        }

        //---------------------------------
        // products
        try 
        {
          for (std::list<YUMRepomdData_Ptr>::const_iterator it = repo_product.begin();
            it != repo_product.end();
            it++)
          {
            Pathname filename = metadataRoot() + (*it)->location;
            ifgzstream st ( filename.asString().c_str() );
            YUMProductParser product(st, "");
            for (; !product.atEnd(); ++product)
            {
              Product::Ptr p = createProduct( source_r, **product );
              _store.insert (p);
            }
            if (product.errorStatus())
              ZYPP_THROW(Exception(product.errorStatus()->msg()));
          }
        }
        catch (...) {
        ERR << "Cannot read products information" << endl;
        }

        //---------------------------------
        // patches, first the patches.xml index
        try
        {
          std::list<std::string> patch_files;
          for (std::list<YUMRepomdData_Ptr>::const_iterator it = repo_patches.begin();
              it != repo_patches.end();
              it++)
          {
            Pathname filename = metadataRoot() + (*it)->location;

            DBG << "Reading file " << filename << endl;
            ifgzstream st ( filename.asString().c_str() );
            YUMPatchesParser patch(st, "");
            
            for (; !patch.atEnd(); ++patch)
            {
              string filename = (*patch)->location;
              patch_files.push_back(filename);
            }
            
            if (patch.errorStatus())
              ZYPP_THROW(Exception(patch.errorStatus()->msg()));            
           }

            //---------------------------------
            // now the individual patch files
    
           for (std::list<std::string>::const_iterator it = patch_files.begin();
              it != patch_files.end();
              it++)
           {
             Pathname filename = metadataRoot() + *it;
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
                  ZYPP_THROW(Exception(ptch.errorStatus()->msg()));
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

      Dependencies deps( createDependencies( parsed, ResTraits<Package>::kind ) );

      CapFactory f;

	for (std::list<FileData>::const_iterator it = filelist.files.begin();
	     it != filelist.files.end();
	     it++)
	{
	    deps[Dep::PROVIDES].insert( f.parse( ResTraits<Package>::kind, it->name ) );
	}

      Arch arch;
      if (!parsed.arch.empty())
        arch = Arch(parsed.arch);

      // Collect basic Resolvable data
      NVRAD dataCollect( parsed.name,
			 Edition( parsed.ver, parsed.rel, parsed.epoch ),
			 arch,
			 deps
		       );
      Package::Ptr package = detail::makeResolvableFromImpl(
	dataCollect, impl
      );
      return package;
    }
    catch (const Exception & excpt_r)
    {
      ZYPP_CAUGHT(excpt_r);
      ZYPP_THROW(Exception("Cannot create package object"));
    }
    return 0L;
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
          arch = Arch( parsed.arch );

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
        Atom::Ptr atom = detail::makeResolvableFromImpl( atomdata, atomimpl );

        //source_r
	PackageImplMapT::const_iterator it = _package_impl.find( nvra );
	if (it == _package_impl.end()) {
	    WAR << "Patch augments non-existant package " << nvra << endl;
	}
        else
        {
          ResImplTraits<YUMPackageImpl>::Ptr impl = it->second.impl;

          if (!parsed.location.empty()) {
              impl->_location = parsed.location;
              impl->_mediaNumber = str::strtonum<unsigned>( parsed.media );
	      impl->_checksum = CheckSum(parsed.checksumType, parsed.checksum);
          }
	  impl->_install_only = parsed.installOnly;

	  //DBG << "Inserting patch RPMs" << endl;
	  impl->_patch_rpms = std::list<PatchRpm>();
	  for (std::list<YUMPatchRpm>::const_iterator it = parsed.patchRpms.begin();
	    it != parsed.patchRpms.end(); ++it)
	  {
	    std::list<BaseVersion> bv_list;
	    for (std::list<YUMBaseVersion>::const_iterator bvit = it->baseVersions.begin();
	      bvit != it->baseVersions.end(); ++bvit)
	    {
	      BaseVersion bv(
		Edition (bvit->ver, bvit->rel, bvit->epoch),
                CheckSum("md5", bvit->md5sum),
		strtol(bvit->buildtime.c_str(), 0, 10)
	      );
	      bv_list.push_back(bv);
	    }
	    PatchRpm patch_rpm(
	      Arch(it->arch),
	      Pathname(it->location),
	      strtol(it->downloadsize.c_str(), 0, 10),
	      CheckSum (it->checksumType, it->checksum),
	      strtol(it->buildtime.c_str(), 0, 10),
	      bv_list,
	      strtol(it->media.c_str(), 0, 10)
	    );
	    impl->_patch_rpms.push_back(patch_rpm);
	  }

	  //DBG << "Inserting delta RPMs" << endl;

	  impl->_delta_rpms = std::list<DeltaRpm>();
	  for (std::list<YUMDeltaRpm>::const_iterator it = parsed.deltaRpms.begin();
	    it != parsed.deltaRpms.end(); ++it)
	  {
	    DeltaRpm delta_rpm(
	      Arch(it->arch),
	      Pathname(it->location),
	      strtol(it->downloadsize.c_str(), 0, 10),
	      CheckSum (it->checksumType, it->checksum),
	      strtol(it->buildtime.c_str(), 0, 10),
	      BaseVersion(
		Edition (it->baseVersion.ver, it->baseVersion.rel, it->baseVersion.epoch),
                CheckSum("md5", it->baseVersion.md5sum),
		strtol(it->baseVersion.buildtime.c_str(), 0, 10)
	      ),
	      strtol(it->media.c_str(), 0, 10)
	    );
	    impl->_delta_rpms.push_back(delta_rpm);
	  }
        }
	return atom;
    }
    catch (const Exception & excpt_r)
    {
      ZYPP_CAUGHT(excpt_r);
      ZYPP_THROW(Exception("Cannot create augmented package object"));
    }
    return 0L;
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
      ZYPP_CAUGHT(excpt_r);
      ZYPP_THROW(Exception("Cannot create package group object"));
    }
    return 0L;
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
      ZYPP_CAUGHT(excpt_r);
      ZYPP_THROW(Exception("Cannot create installation pattern object"));
    }
    return 0L;
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
      ZYPP_CAUGHT(excpt_r);
      ZYPP_THROW(Exception("Cannot create message object"));
    }
    return 0L;
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
      ZYPP_CAUGHT(excpt_r);
      ZYPP_THROW(Exception("Cannot create script object"));
    }
    return 0L;
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
      ZYPP_CAUGHT(excpt_r);
      ZYPP_THROW(Exception("Cannot create product object"));
    }
    return 0L;
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

	// maps name to parser data in order to find 'best' architectureC
	typedef std::map<std::string, shared_ptr<YUMPatchPackage> > PkgAtomsMap;
	PkgAtomsMap pkg_atoms;

	for (std::list<shared_ptr<YUMPatchAtom> >::const_iterator it
					= parsed.atoms.begin();
	     it != parsed.atoms.end();
	     it++)
	{
	  switch ((*it)->atomType())
	  {
	    // for packages, try to find best architecture for name-version-release first (#168840)
	    // we can't use the name alone as there might be different editions for the same name
	    // with different architecture.
	    // So we only choose the best architecture if name-version-edition matches (#170098)

	    case YUMPatchAtom::Package: {
	      shared_ptr<YUMPatchPackage> package_data
		= dynamic_pointer_cast<YUMPatchPackage>(*it);
	      string atomkey( package_data->name + "-" + package_data->epoch + ":" + package_data->ver + "-" + package_data->rel );

	      // check if atomkey is already known
	      PkgAtomsMap::iterator pa_pos = pkg_atoms.find( atomkey );
	      if (pa_pos != pkg_atoms.end()) {
		try {
		  Arch oldarch, newarch;
		  if (!(pa_pos->second->arch.empty())) oldarch = Arch( pa_pos->second->arch );
		  if (!(package_data->arch.empty())) newarch = Arch( package_data->arch );
		  if (newarch.compatibleWith( getZYpp()->architecture() ) ) {			// new one is compatible (if not, we don't care)

		    if (!oldarch.compatibleWith( getZYpp()->architecture() )			// old one is not compatible
			|| oldarch.compare( newarch ) < 0)					//  or compatible but worse
		    {
		      pa_pos->second = package_data;				// new one is it !
		    }
		  }
		}
		catch( const Exception & excpt_r ) {
		    ZYPP_CAUGHT( excpt_r );
		    ERR << "Package " << package_data->name << " in patch's atomlist has bad architecture '" << package_data->arch << "'" << endl;
		}
	      }
	      else {
		pkg_atoms[atomkey] = package_data;					// first occurence of this atomkey
	      }
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
#if 0					// atoms require their patch, why ?
	  for (Patch::AtomList::iterator it = impl->_atoms.begin();
	       it != impl->_atoms.end();
	       it++)
	  {
	    (*it)->injectRequires(cap);
	  }
#endif
	}

	for (PkgAtomsMap::const_iterator pa_pos = pkg_atoms.begin(); pa_pos != pkg_atoms.end(); ++pa_pos) {
	  Atom::Ptr atom = augmentPackage( source_r, *(pa_pos->second) );
	  impl->_atoms.push_back(atom);
	}

      return patch;
    }
    catch (const Exception & excpt_r)
    {
      ZYPP_CAUGHT(excpt_r);
      ZYPP_THROW(Exception("Cannot create patch object"));
    }
    return 0L;
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

    for (std::list<YUMDependency>::const_iterator it = parsed.prerequires.begin();
         it != parsed.prerequires.end();
         it++)
    {
        _deps[Dep::PREREQUIRES].insert(createCapability(*it, my_kind));
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
	  return false;
	}
	dig = str::toLower (dig);
	bool ret = (dig == str::toLower(csum));
	if (ret)
        {
	  MIL << "Checksums are the same" << endl;
          return true;
        }
	else
        {
          WAR << "Checksum missmatch: metadata: " << csum << "; real: " << dig << endl;
          return false;
        }
        return false;
      }

    } // namespace yum
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
