/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/susetags/SuseTagsImpl.cc
*
*/
#include <iostream>
#include <fstream>
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"

#include "zypp/PathInfo.h"

#include "zypp/source/susetags/SuseTagsImpl.h"
#include "zypp/source/susetags/PackagesParser.h"
#include "zypp/source/susetags/PackagesLangParser.h"
#include "zypp/source/susetags/SelectionTagFileParser.h"
#include "zypp/source/susetags/PatternTagFileParser.h"
#include "zypp/source/susetags/ProductMetadataParser.h"

#include "zypp/SourceFactory.h"
#include "zypp/ZYppCallbacks.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : SuseTagsImpl::SuseTagsImpl
      //	METHOD TYPE : Ctor
      //
      SuseTagsImpl::SuseTagsImpl()
      {}

      void SuseTagsImpl::initCacheDir(const Pathname & cache_dir_r)
      {
        // refuse to use stupid paths as cache dir
        if (cache_dir_r == Pathname("/") )
          ZYPP_THROW(Exception("I refuse to use / as cache dir"));
          
        if (0 != assert_dir(cache_dir_r.dirname(), 0700))
          ZYPP_THROW(Exception("Cannot create cache directory"));
        
        filesystem::clean_dir(cache_dir_r);
        filesystem::mkdir(cache_dir_r + "DATA");
        filesystem::mkdir(cache_dir_r + "MEDIA");
        filesystem::mkdir(cache_dir_r + "DESCRIPTION");
      }
      
      void SuseTagsImpl::storeMetadata(const Pathname & cache_dir_r)
      {
        initCacheDir(cache_dir_r);
        //suse/setup/descr
        //packages.* *.sel
        media::MediaManager media_mgr;
        media::MediaAccessId media_num = _media_set->getMediaAccessId(1);
        INT << "Storing data to cache" << endl;
        media_mgr.provideDirTree(media_num, "suse/setup/descr");
        Pathname descr_src = media_mgr.localPath(media_num, "suse/setup/descr"); 
        Pathname media_src = media_mgr.localPath(media_num, "media.1"); 
        Pathname content_src = media_mgr.localPath(media_num, "content"); 
        if (0 != assert_dir((cache_dir_r + "DATA").dirname(), 0700))
        {
          ZYPP_THROW(Exception("Cannot create cache directory"));
        }
        else
        {
          filesystem::copy_dir(descr_src, cache_dir_r + "DATA");
          filesystem::copy(content_src, cache_dir_r + "DATA/content");
          filesystem::copy_dir(media_src, cache_dir_r + "MEDIA");
        }
        
        //releaseDir(media_num, "suse/setup/descr");
      }
      
      bool SuseTagsImpl::cacheExists()
      {
        bool exists = true;
	
        exists = exists && PathInfo(_cache_dir + "DATA").isExist();
        exists = exists && PathInfo(_cache_dir + "DESCRIPTION").isExist();
        exists = exists && PathInfo(_cache_dir + "MEDIA").isExist();

        DBG << exists << std::endl;
        return exists;
      }
      
      void SuseTagsImpl::factoryInit()
      {
        media::MediaManager media_mgr;
  
        std::string vendor;
        std::string media_id;

        // dont initialize media if we are reading from cache
        if ( cacheExists() )
          return;
        
        try {
          media::MediaAccessId _media = _media_set->getMediaAccessId(1);
          Pathname media_file = Pathname("media.1/media");
          media_mgr.provideFile (_media, media_file);
          media_file = media_mgr.localPath (_media, media_file);
    
          std::ifstream pfile( media_file.asString().c_str() );

          if ( pfile.bad() ) {
            ERR << "Error parsing media.1/media" << endl;
            ZYPP_THROW(Exception("Error parsing media.1/media") );
          }

          _vendor = str::getline( pfile, str::TRIM );

          if ( pfile.fail() ) {
            ERR << "Error parsing media.1/media" << endl;
            ZYPP_THROW(Exception("Error parsing media.1/media") );
          }
  
          _media_id = str::getline( pfile, str::TRIM );
      
          if ( pfile.fail() ) {
            ERR << "Error parsing media.1/media" << endl;
            ZYPP_THROW(Exception("Error parsing media.1/media") );
          }

          std::string media_count_str = str::getline( pfile, str::TRIM );
    
          if ( pfile.fail() ) {
            ERR << "Error parsing media.1/media" << endl;
            ZYPP_THROW(Exception("Error parsing media.1/media") );
          }
    
          _media_count = str::strtonum<unsigned>( media_count_str );

        }
        catch ( const Exception & excpt_r )
        {
          ERR << "Cannot read /media.1/media file, cannot initialize source" << endl;
          ZYPP_THROW( Exception("Cannot read /media.1/media file, cannot initialize source") );
        }

        try {
          MIL << "Adding susetags media verifier: " << endl;
          MIL << "Vendor: " << _vendor << endl;
          MIL << "Media ID: " << _media_id << endl;

          media::MediaAccessId _media = _media_set->getMediaAccessId(1);
                media_mgr.delVerifier(_media);
                media_mgr.addVerifier(_media, media::MediaVerifierRef(
                new SourceImpl::Verifier (_vendor, _media_id) ));
        }
        catch (const Exception & excpt_r)
        {
#warning FIXME: If media data is not set, verifier is not set. Should the media
          ZYPP_CAUGHT(excpt_r);
          WAR << "Verifier not found" << endl;
        }
      }
      
      void SuseTagsImpl::createResolvables(Source_Ref source_r)
      {
        callback::SendReport<CreateSourceReport> report;
        Pathname p;
        report->startData( url() );
        
        bool cache = cacheExists();

        if ( cache )
        {
          DBG << "Cached metadata found. Reading from " << _cache_dir << endl;  
          _data_dir  = _cache_dir + "DATA/descr";
          _content_file = _cache_dir + "DATA/content";
        }
        else
        {
          DBG << "Cached metadata not found. Reading from " << _path << endl;  
          _data_dir = _path + "suse/setup/descr";
          _content_file = provideFile(_path + "content");
        }

        SourceFactory factory;

        try {
          DBG << "Going to parse content file " << _content_file << endl;  
          Product::Ptr product = parseContentFile( _content_file, factory.createFrom(this) );
      
          MIL << "Product: " << product->displayName() << endl;
          _store.insert( product );
        }
        catch (Exception & excpt_r) {
          ERR << "cannot parse content file" << endl;
        }
  
#warning We use suse instead of <DATADIR> for now
        p = cache ? _data_dir + "packages" : provideFile( _data_dir + "packages");
        DBG << "Going to parse " << p << endl;
        PkgContent content( parsePackages( source_r, this, p ) );

#warning Should use correct locale and locale fallback list
        try {
          Locale lang;
          p = cache ? _data_dir + "packages.en" : provideFile( _data_dir + "packages.en");
          DBG << "Going to parse " << p << endl;
          parsePackagesLang( p, lang, content );
        }
        catch (Exception & excpt_r) {
          WAR << "packages.en not found" << endl;
        }

        PkgDiskUsage du;
        try
        {
          p = cache ? _data_dir + "packages.DU" : provideFile( _data_dir + "packages.DU");
          du = parsePackagesDiskUsage(p);
        }
        catch (Exception & excpt_r)
        {
            WAR << "Problem trying to parse the disk usage info" << endl;
        }

        for (PkgContent::const_iterator it = content.begin(); it != content.end(); ++it)
        {
          it->second->_diskusage = du[it->first /* NVRAD */];
          Package::Ptr pkg = detail::makeResolvableFromImpl( it->first, it->second );
          _store.insert( pkg );
        }
        DBG << "SuseTagsImpl (fake) from " << p << ": "
            << content.size() << " packages" << endl;

        bool file_found = true;
        // parse selections
        try {
          p = cache ? _data_dir + "selections" : provideFile( _data_dir + "selections");
        }
        catch (Exception & excpt_r)
        {
          MIL << "'selections' file not found" << endl;
          file_found = false;
        }

        if (file_found)
        {
          std::ifstream sels (p.asString().c_str());
      
          while (sels && !sels.eof())
          {
            std::string selfile;
            getline(sels,selfile);
      
            if (selfile.empty() ) continue;
              DBG << "Going to parse selection " << selfile << endl;
      
              Pathname file = cache ? _data_dir + selfile : provideFile( _data_dir + selfile);
            MIL << "Selection file to parse " << file << endl;
            Selection::Ptr sel( parseSelection( file ) );
      
            DBG << "Selection:" << sel << endl;
      
            if (sel)
              _store.insert( sel );
      
            DBG << "Parsing of " << file << " done" << endl;
          }
        }

        // parse patterns
        file_found = true;

        try {
          p = cache ? _data_dir + "patterns" : provideFile( _data_dir + "patterns");
        }
        catch (Exception & excpt_r)
        {
          MIL << "'patterns' file not found" << endl;
          file_found = false;
        }

        if ( file_found )
        {
          std::ifstream pats (p.asString().c_str());
      
          while (pats && !pats.eof())
          {
            std::string patfile;
            getline(pats,patfile);
      
            if (patfile.empty() ) continue;
      
            DBG << "Going to parse pattern " << patfile << endl;
      
            Pathname file = cache ? _data_dir + patfile : provideFile( _data_dir + patfile);
            MIL << "Pattern file to parse " << file << endl;
      
            Pattern::Ptr pat( parsePattern( file ) );
      
            DBG << "Pattern:" << pat << endl;
      
            if (pat)
              _store.insert( pat );
      
            DBG << "Parsing of " << file << " done" << endl;
          }
        }
      
        report->finishData( url(), CreateSourceReport::NO_ERROR, "" );
      }
      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : SuseTagsImpl::~SuseTagsImpl
      //	METHOD TYPE : Dtor
      //
      SuseTagsImpl::~SuseTagsImpl()
      {}

      Pathname SuseTagsImpl::sourceDir( const NVRAD& nvrad )
      {
#warning Not using <DATADIR>
        return Pathname( "/suse/" + nvrad.arch.asString() + "/");
      }

      media::MediaVerifierRef SuseTagsImpl::verifier(media::MediaNr media_nr)
      {
    return media::MediaVerifierRef(
      new SourceImpl::Verifier (_vendor, _media_id, media_nr));
      }

      unsigned SuseTagsImpl::numberOfMedia(void) const
      { return _media_count; }

      std::string SuseTagsImpl::vendor (void) const
      { return _vendor; }

      std::string SuseTagsImpl::unique_id (void) const
      { return _media_id; }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : SuseTagsImpl::dumpOn
      //	METHOD TYPE : std::ostream &
      //
      std::ostream & SuseTagsImpl::dumpOn( std::ostream & str ) const
      {
        return str << "SuseTagsImpl";
      }

      /////////////////////////////////////////////////////////////////
    } // namespace susetags
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
