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
#include "zypp/ZYppFactory.h"

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
          ZYPP_THROW(Exception("Cannot create cache directory" + cache_dir_r.asString()));

        filesystem::clean_dir(cache_dir_r);
        filesystem::mkdir(cache_dir_r + "DATA");
        filesystem::mkdir(cache_dir_r + "MEDIA");
        //filesystem::mkdir(cache_dir_r + "DESCRIPTION");
        filesystem::mkdir(cache_dir_r + "PUBLICKEYS");
      }

      void SuseTagsImpl::storeMetadata(const Pathname & cache_dir_r)
      {
        _cache_dir = cache_dir_r;

        initCacheDir(cache_dir_r);
        //suse/setup/descr
        //packages.* *.sel
        media::MediaManager media_mgr;
        media::MediaAccessId media_num = _media_set->getMediaAccessId(1);
        
        INT << "Storing data to cache " << cache_dir_r << endl;
        Pathname descr_src = provideDirTree(_descr_dir);
        Pathname media_src = provideDirTree("media.1");
        Pathname content_src = provideFile("/content");

        // get the list of cache keys
        std::list<std::string> files;
        dirInfo( media_num, files, "/");
                
        if (0 != assert_dir((cache_dir_r + "PUBLICKEYS"), 0700))
        {
          ZYPP_THROW(Exception("Cannot create cache PUBLICKEYS directory: " + (cache_dir_r + "PUBLICKEYS").asString()));
        }
        else
        {
          // cache all the public keys
          for( std::list<std::string>::const_iterator it = files.begin(); it != files.end(); ++it)
          {
            std::string filename = *it;
            if ( filename.substr(0, 10) == "gpg-pubkey" )
            {
              Pathname key_src = provideFile("/" + filename);
              MIL << "Trying to cache " << key_src << std::endl;
              filesystem::copy(key_src, cache_dir_r + "PUBLICKEYS/" + filename);
              MIL << "cached " << filename << std::endl;
            }
          }
        }

        if (0 != assert_dir((cache_dir_r + "DATA"), 0700))
        {
          ZYPP_THROW(Exception("Cannot create cache DATA directory: " + (cache_dir_r + "DATA").asString()));
        }
        else
        {
          filesystem::copy_dir(descr_src, cache_dir_r + "DATA");
          MIL << "cached descr directory" << std::endl;
          filesystem::copy(content_src, cache_dir_r + "DATA/content");
          MIL << "cached content file" << std::endl;
        }

        if (0 != assert_dir((cache_dir_r + "MEDIA"), 0700))
        {
          ZYPP_THROW(Exception("Cannot create cache MEDIA directory: " + (cache_dir_r + "MEDIA").asString()));
        }
        else
        {
          filesystem::copy_dir(media_src, cache_dir_r + "MEDIA");
          MIL << "cached media directory" << std::endl;
        }
      }

      bool SuseTagsImpl::cacheExists()
      {
        MIL << "Checking if source cache exists in "<< _cache_dir << std::endl;
        bool exists = true;

        bool data_exists = PathInfo(_cache_dir + "DATA").isExist();
        exists = exists && data_exists;

        //bool description_exists = PathInfo(_cache_dir + "DESCRIPTION").isExist();
        //exists = exists && description_exists;

        bool media_exists = PathInfo(_cache_dir + "MEDIA").isExist();
        exists = exists && media_exists;

        bool media_file_exists = PathInfo(_cache_dir + "MEDIA/media.1/media").isExist();
        exists = exists && media_file_exists;

        MIL << "DATA " << (data_exists ? "exists" : "not found") << ", MEDIA " << (media_exists ? "exists" : "not found") << ", MEDIA/media.1/media " <<  (media_file_exists ? "exists" : "not found") << std::endl;
        return exists;
      }

      void SuseTagsImpl::factoryInit()
      {
        media::MediaManager media_mgr;

        std::string vendor;
        std::string media_id;
        bool cache = cacheExists();

        try {
          Pathname media_file = Pathname("media.1/media");

          if (cache)
          {
            media_file = _cache_dir + "MEDIA" + media_file;
          }
          else
          {
            media::MediaAccessId _media = _media_set->getMediaAccessId(1);
            media_mgr.provideFile (_media, media_file);
            media_file = media_mgr.localPath (_media, media_file);
          }

          std::ifstream pfile( media_file.asString().c_str() );

          if ( pfile.bad() ) {
            ERR << "Error parsing media.1/media from file" << media_file << endl;
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

	  // get media ID, but not attaching
          media::MediaAccessId _media = _media_set->getMediaAccessId(1, true);
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
        report->startData( url() );

	provideProducts ( source_r, _store );
	providePackages ( source_r, _store );
	provideSelections ( source_r, _store );
	providePatterns ( source_r, _store );

        report->finishData( url(), CreateSourceReport::NO_ERROR, "" );
      }

      const std::list<Pathname> SuseTagsImpl::publicKeys()
      {
        bool cache = cacheExists();
        std::list<std::string> files;
        std::list<Pathname> paths;

        MIL << "Reading public keys..." << std::endl;
        if (cache)
        {
          filesystem::readdir(files, _cache_dir + "PUBLICKEYS");
          for( std::list<std::string>::const_iterator it = files.begin(); it != files.end(); ++it)
            paths.push_back(Pathname(*it));

          MIL << "read " << files.size() << " keys from cache " << _cache_dir << std::endl;
        }
        else
        {
          std::list<std::string> allfiles;
          media::MediaManager media_mgr;
          media::MediaAccessId media_num = _media_set->getMediaAccessId(1);
          dirInfo(media_num, allfiles, "/");

          for( std::list<std::string>::const_iterator it = allfiles.begin(); it != allfiles.end(); ++it)
          {
            std::string filename = *it;
            if ( filename.substr(0, 10) == "gpg-pubkey" )
            {
              Pathname key_src = provideFile("/" + filename);
              paths.push_back(filename);
            }
          }
          MIL << "read " << paths.size() << " keys from media" << std::endl;
        }
        return paths;
      }

      ResStore SuseTagsImpl::provideResolvables(Source_Ref source_r, Resolvable::Kind kind)
      {
        callback::SendReport<CreateSourceReport> report;
        report->startData( url() );

	ResStore store;

	if ( kind == ResTraits<Product>::kind )
	    provideProducts ( source_r, store );
	else if ( kind == ResTraits<Package>::kind )
	    providePackages ( source_r, store );
	else if ( kind == ResTraits<Selection>::kind )
	    provideSelections ( source_r, store );
	else if ( kind == ResTraits<Pattern>::kind )
	    providePatterns ( source_r, store );

        report->finishData( url(), CreateSourceReport::NO_ERROR, "" );

	return store;
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
        return Pathname( _data_dir + Pathname(nvrad.arch.asString()) + "/");
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

      void SuseTagsImpl::provideProducts(Source_Ref source_r, ResStore &store)
      {
        Pathname p;
        bool cache = cacheExists();

        if ( cache )
        {
          DBG << "Cached metadata found. Reading from " << _cache_dir << endl;
          _content_file = _cache_dir + "DATA/content";
        }
        else
        {
          DBG << "Cached metadata not found in [" << _cache_dir << "]. Reading from " << _path << endl;
 
          _content_file = provideFile(_path + "content");
        }

        SourceFactory factory;

        try {
          DBG << "Going to parse content file " << _content_file << endl;
          
          ProductMetadataParser p;
          p.parse( _content_file, factory.createFrom(this) );
          Product::Ptr product = p.result;
          
          // data dir is the same, it is determined by the content file
          _data_dir = _path + p.prodImpl->_data_dir;
          
          // description dir also changes when using cache          
          if (cache)
            _descr_dir  = _cache_dir + "DATA/descr";
          else
            _descr_dir = _path + p.prodImpl->_description_dir;
          
          MIL << "Product: " << product->summary() << endl;
          store.insert( product );
          _prodImpl = p.prodImpl;
        }
        catch (Exception & excpt_r) {
          ERR << "cannot parse content file" << endl;
        }
      }

      void SuseTagsImpl::providePackages(Source_Ref source_r, ResStore &store)
      {
        bool cache = cacheExists();

        Pathname p = cache ? _descr_dir + "packages" : provideFile( _descr_dir + "packages");
        DBG << "Going to parse " << p << endl;
        PkgContent content( parsePackages( source_r, this, p ) );

#warning Should use correct locale and locale fallback list
        // note, this locale detection has nothing to do with translated text.
        // basically we are only loading the data we need. Instead of parsing all
        // package description we fill the TranslatedText properties only
        // with the detected locale.

	ZYpp::Ptr z = getZYpp();
        Locale lang( z->getTextLocale() );

        std::string packages_lang_prefix( "packages." );
	std::string packages_lang_name;

        // find the most apropiate file
        bool trymore = true;
        while ( (lang != Locale()) && trymore )
        {
          packages_lang_name = packages_lang_prefix + lang.code();
          MIL << "Going to try " << packages_lang_name << std::endl;
          try
          {
            p = cache ? _descr_dir + packages_lang_name : provideFile( _descr_dir + packages_lang_name);
            if ( PathInfo(p).isExist() )
            {
              MIL << packages_lang_name << " found" << std::endl;
              DBG << "Going to parse " << p << endl;
              parsePackagesLang( p, lang, content );
              trymore = false;
            }
            else
            {
              MIL << packages_lang_name << " not found" << endl;
            }
          }
          catch (Exception & excpt_r)
          {
            ZYPP_CAUGHT(excpt_r);
          }
          lang = lang.fallback();
        }

        PkgDiskUsage du;
        try
        {
          p = cache ? _descr_dir + "packages.DU" : provideFile( _descr_dir + "packages.DU");
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
          store.insert( pkg );
        }
        DBG << "SuseTagsImpl (fake) from " << p << ": "
            << content.size() << " packages" << endl;
      }

      void SuseTagsImpl::provideSelections(Source_Ref source_r, ResStore &store)
      {
        bool cache = cacheExists();

	Pathname p;

        bool file_found = true;

        // parse selections
        try {
          p = cache ? _descr_dir + "selections" : provideFile( _descr_dir + "selections");
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

              Pathname file = cache ? _descr_dir + selfile : provideFile( _descr_dir + selfile);
            MIL << "Selection file to parse " << file << endl;
            Selection::Ptr sel( parseSelection( source_r, file ) );

            if (sel)
            {
              DBG << "Selection:" << sel << endl;
              store.insert( sel );
              DBG << "Parsing of " << file << " done" << endl;
            }
            else
            {
              DBG << "Parsing of " << file << " failed" << endl;
            }


          }
        }
      }

      void SuseTagsImpl::providePatterns(Source_Ref source_r, ResStore &store)
      {
        bool cache = cacheExists();
	Pathname p;

        // parse patterns
        bool file_found = true;

        try {
          p = cache ? _descr_dir + "patterns" : provideFile( _descr_dir + "patterns");
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

            Pathname file = cache ? _descr_dir + patfile : provideFile( _descr_dir + patfile);
            MIL << "Pattern file to parse " << file << endl;

            Pattern::Ptr pat( parsePattern( source_r, file ) );

            if (pat)
            {
              DBG << "Pattern:" << pat << endl;
              _store.insert( pat );
              DBG << "Parsing of " << file << " done" << endl;
            }
            else
            {
              DBG << "Parsing of " << file << " failed" << endl;
            }
          }
        }
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : SuseTagsImpl::dumpOn
      //	METHOD TYPE : std::ostream &
      //
      std::ostream & SuseTagsImpl::dumpOn( std::ostream & str ) const
      {
        return SourceImpl::dumpOn( str );
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
