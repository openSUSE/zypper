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

#include <boost/bind.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"

#include "zypp/PathInfo.h"
#include "zypp/Digest.h"
#include "zypp/CheckSum.h"
#include "zypp/KeyRing.h"

#include "zypp/parser/ParserProgress.h"
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


      struct NullParseProgress
      {
        NullParseProgress( Pathname /*file*/ )
        //: _file(file)
        {}
        void operator()( int /*p*/ )
        {}
        //Pathname _file;
      };

      struct SourceEventHandler
      {
        SourceEventHandler( callback::SendReport<SourceReport> &report ) : _report(report)
        {}
        
        void operator()( int p )
        {
          _report->progress(p);
        }
        
        callback::SendReport<SourceReport> &_report;
      };
      
      
      bool SuseTagsProber::operator()()
      {
        MIL << "Probing for YaST source..." << std::endl;
        bool result = false;
        media::MediaManager mm;
        result = mm.doesFileExist(_media_id, _path + Pathname("/content"));

        if ( result )
        {
          MIL << "YaST source detected..." << std::endl;
          return true;
        }

         MIL << "Not a YaST source..." << std::endl;
         return false;
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : SuseTagsImpl::SuseTagsImpl
      //	METHOD TYPE : Ctor
      //
      SuseTagsImpl::SuseTagsImpl()
      {}

      const Pathname SuseTagsImpl::metadataRoot() const
      {
        return _cache_dir.empty() ? tmpMetadataDir() : _cache_dir;
      }

      const Pathname SuseTagsImpl::contentFile() const
      {
        return metadataRoot() + "/DATA" + "content";
      }

      const Pathname SuseTagsImpl::contentFileSignature() const
      {
        return metadataRoot() + "/DATA" + "content.asc";
      }

      const Pathname SuseTagsImpl::contentFileKey() const
      {
        return metadataRoot() + "/DATA" + "/content.key";
      }

      const Pathname SuseTagsImpl::mediaFile() const
      {
        return metadataRoot() + "MEDIA/media.1/media";
      }

      const Pathname SuseTagsImpl::descrDir() const
      {
        return metadataRoot() + "DATA/descr";
      }

      const Pathname SuseTagsImpl::dataDir() const
      {
        return _data_dir;
      }

      const Pathname SuseTagsImpl::mediaDescrDir() const
      {
        return _media_descr_dir;
      }

      bool SuseTagsImpl::downloadNeeded(const Pathname & localdir)
      {
        Pathname new_media_file;
        try {
          new_media_file = tryToProvideFile("media.1/media");
        }
        catch( const Exception &e )
        {
          MIL << "media file used to determine if source changed not found. Assuming refresh needed." << std::endl;
          return true;
        }

        // before really download all the data and init the cache, check
        // if the source has really changed, otherwise, make it quick
        Pathname cached_media_file = localdir + "/MEDIA/media.1/media";
        if ( cacheExists() )
        {
          CheckSum old_media_file_checksum( "SHA1", filesystem::sha1sum(cached_media_file));
          CheckSum new_media_file_checksum( "SHA1", filesystem::sha1sum(new_media_file));
          if ( (new_media_file_checksum == old_media_file_checksum) && (!new_media_file_checksum.empty()) && (! old_media_file_checksum.empty()))
          {
            MIL << "susetags source " << alias() << " has not changed. Refresh completed. SHA1 of media.1/media file is " << old_media_file_checksum.checksum() << std::endl;
            return false;
          }
        }
        MIL << "susetags source " << alias() << " has changed. Refresh needed." << std::endl;
        return true;
      }

      void SuseTagsImpl::readMediaFile(const Pathname &p)
      {
        media::MediaManager media_mgr;

        std::ifstream pfile( p.asString().c_str() );
        if ( pfile.bad() ) {
          ZYPP_THROW(Exception("Error parsing media.1/media") );
        }
        _media_vendor = str::getline( pfile, str::TRIM );
        if ( pfile.fail() ) {
          ZYPP_THROW(Exception("Error parsing media.1/media") );
        }
        _media_id = str::getline( pfile, str::TRIM );
        if ( pfile.fail() ) {
          ZYPP_THROW(Exception("Error parsing media.1/media") );
        }
        std::string media_count_str = str::getline( pfile, str::TRIM );
        if ( pfile.fail() ) {
          ZYPP_THROW(Exception("Error parsing media.1/media") );
        }
        _media_count = str::strtonum<unsigned>( media_count_str );

        try {
          MIL << "Adding susetags media verifier: " << endl;
          MIL << "Vendor: " << _media_vendor << endl;
          MIL << "Media ID: " << _media_id << endl;

          // get media ID, but not attaching
          media::MediaAccessId _media = _media_set->getMediaAccessId(1, true);
          media_mgr.delVerifier(_media);
          media_mgr.addVerifier(_media, media::MediaVerifierRef(
              new SourceImpl::Verifier ( _media_vendor, _media_id) ));
        }
        catch (const Exception & excpt_r)
        {
#warning FIXME: If media data is not set, verifier is not set. Should the media
          ZYPP_CAUGHT(excpt_r);
          WAR << "Verifier not found" << endl;
        }
      }

      TmpDir SuseTagsImpl::downloadMetadata()
      {
        resetMediaVerifier();

        TmpDir tmpdir;
        MIL << "Downloading metadata to " << tmpdir.path() << std::endl;

        Pathname local_dir = tmpdir.path();

        // (#163196)
        // before we used _descr_dir, which is is wrong if we
        // store metadata already running from cache
        // because it points to a local file and not
        // to the media. So use the original media descr_dir.
        Pathname media_src;
        Pathname descr_src;
        Pathname content_src;

        // init the cache structure
        if (0 != assert_dir(local_dir + "DATA", 0755))
          ZYPP_THROW(Exception("Cannot create /DATA directory in download dir." + local_dir.asString()));
        if (0 != assert_dir(local_dir + "MEDIA", 0755))
          ZYPP_THROW(Exception("Cannot create /MEDIA directory in download dir." + local_dir.asString()));
        if (0 != assert_dir(local_dir + "PUBLICKEYS", 0755))
          ZYPP_THROW(Exception("Cannot create /PUBLICKEYS directory in download dir." + local_dir.asString()));

        try {
          media_src = provideDirTree("media.1");
        }
        catch(Exception &e) {
          ZYPP_THROW(Exception("Can't provide " + _path.asString() + "/media.1 from " + url().asString() ));
        }

        if ( filesystem::copy_dir(media_src, local_dir + "MEDIA") != 0 )
          ZYPP_THROW(Exception("Unable to copy media directory to " + (local_dir + "/MEDIA").asString()));

        MIL << "cached media directory" << std::endl;

        // media is provided, now we can install a media verifier.
        readMediaFile(local_dir + "/MEDIA/media.1/media");

        try {
          content_src = provideFile( _path + "content");
        }
        catch(Exception &e) {
          ZYPP_THROW(Exception("Can't provide " + _path.asString() + "/content from " + url().asString() ));
        }

        if ( filesystem::copy(content_src, local_dir + "DATA/content") != 0)
          ZYPP_THROW(Exception("Unable to copy the content file to " + (local_dir + "DATA/content").asString()));

        // get the list of cache keys
        std::list<std::string> files;
        dirInfo( 1, files, _path);

        // cache all the public keys
        for( std::list<std::string>::const_iterator it = files.begin(); it != files.end(); ++it)
        {
          ZYpp::Ptr z = getZYpp();

          std::string filename = *it;
          if ( filename.substr(0, 10) == "gpg-pubkey" )
          {
            Pathname key_src;
            try {
              key_src = provideFile(_path + filename);
            }
            catch(Exception &e) {
              ZYPP_THROW(Exception("Can't provide " + (_path + filename).asString() + " from " + url().asString() ));
            }

            MIL << "Trying to cache " << key_src << std::endl;

            if ( filesystem::copy(key_src, local_dir + "PUBLICKEYS/" + filename) != 0 )
              ZYPP_THROW(Exception("Unable to copy key " + key_src.asString() + " to " + (local_dir + "PUBLICKEYS").asString()));

            MIL << "cached " << filename << std::endl;
            z->keyRing()->importKey(local_dir + "PUBLICKEYS/" + filename, false);
          }
          else if( (filename == "content.asc") || (filename == "content.key"))
          {
            Pathname src_data;
            try {
              src_data = provideFile(_path + filename);
            }
            catch(Exception &e) {
              ZYPP_THROW(Exception("Can't provide " + filename + " from " + url().asString() + ". File was listed as available."));
            }

            if ( filesystem::copy( src_data, local_dir + "DATA/" + filename) != 0 )
              ZYPP_THROW(Exception("Unable to copy " + filename + " to " + (local_dir + "/DATA").asString()));

            if ( filename == "content.key" )
              z->keyRing()->importKey(local_dir + "DATA/content.key", false);

            MIL << "cached " << filename << std::endl;
          }
        }

        // verify it is a valid content file
        ZYpp::Ptr z = getZYpp();
        MIL << "SuseTags source: checking 'content' file vailidity using digital signature.." << endl;
        // verify the content file
        bool valid = z->keyRing()->verifyFileSignatureWorkflow( local_dir + "/DATA/content", (path() + "content").asString() + " (" + url().asString() + ")", local_dir + "/DATA/content.asc");

        // the source is not valid and the user did not want to continue
        if (!valid)
          ZYPP_THROW (Exception( "Error. Source signature does not validate and user does not want to continue. "));

        // now we have the content file copied, we need to init data and descrdir from the product
        readContentFile(local_dir + "/DATA/content");

        // make sure a local descr dir exists
        if ( assert_dir( local_dir + "/DATA/descr") != 0 )
          ZYPP_THROW (Exception( "Error. Can't create local descr directory. "));

        // we can get the list of files in description dir in 2 ways
        // from the checksum list, or ls'ing the dir via directory.yast
        if ( ! _prodImpl->_descr_files_checksums.empty() )
        {
          // iterate through all available checksums
          for ( std::map<std::string, CheckSum>::const_iterator it = _prodImpl->_descr_files_checksums.begin(); it != _prodImpl->_descr_files_checksums.end(); ++it)
          {
            std::string key = it->first;
            getPossiblyCachedMetadataFile( mediaDescrDir() + key, local_dir + "/DATA/descr" + key, _cache_dir + "/DATA/descr" + key,  _prodImpl->_descr_files_checksums[key] );
          }
        }
        else
        {
          // in case we dont have list of valid files in content file, we just glob for them
          std::list<std::string> descr_dir_file_list;
          try {
            dirInfo( 1, descr_dir_file_list, mediaDescrDir());
          }
          catch(Exception &e) {
            ZYPP_THROW(Exception("Can't list description directory content from " + url().asString() ));
          }

          for( std::list<std::string>::const_iterator it = descr_dir_file_list.begin(); it != descr_dir_file_list.end(); ++it)
          {
            std::string filename = *it;
            getPossiblyCachedMetadataFile( mediaDescrDir() + filename, local_dir + "/DATA/descr" + filename, _cache_dir + "/DATA/descr" + filename, CheckSum() );
          }
        }

        return tmpdir;
      }

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
        if ( !_cache_dir.empty() )
        {
          saveMetadataTo(cache_dir_r);
        }
        else
        {
          // no previous cache, use the data read temporarely
          copyLocalMetadata(tmpMetadataDir(), cache_dir_r);
        }

        MIL << "Metadata saved in " << cache_dir_r << ". Setting as cache." << std::endl;
        _cache_dir = cache_dir_r;
      }

      void SuseTagsImpl::saveMetadataTo(const Pathname & dir_r)
      {
        TmpDir download_tmp_dir;

        bool need_to_refresh = true;
        try {
          need_to_refresh = downloadNeeded(dir_r);
        }
        catch(Exception &e) {
          ZYPP_THROW(Exception("Can't check if source has changed or not. Aborting refresh."));
        }

        if ( need_to_refresh )
        {
          MIL << "SuseTags source " << alias() << " has changed since last download. Re-reading metadata into " << dir_r << endl;
        }
        else
        {
          MIL << "SUSEtags source " << alias() << " has not changed. Refresh completed. timestamp of media file is  the same." << std::endl;
          return;
        }

        try {
          download_tmp_dir = downloadMetadata();
        }
        catch(Exception &e) {
          ZYPP_THROW(Exception("Downloading metadata failed (is a susetags source?) or user did not accept remote source. Aborting refresh."));
        }

        copyLocalMetadata(download_tmp_dir, dir_r);

        // download_tmp_dir go out of scope now but it is ok as we already copied the content.
      }

      bool SuseTagsImpl::cacheExists() const
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
        bool cache = cacheExists();
        if ( cache )
        {
          MIL << "Cached metadata found in [" << _cache_dir << "]." << endl;
          // we need to read the content file to init data dir and descr dir
          // in the media.
          readMediaFile(mediaFile());
          readContentFile(contentFile());

          if ( autorefresh() )
            storeMetadata(_cache_dir);
        }
        else
        {
          if ( _cache_dir.empty() || !PathInfo(_cache_dir).isExist() )
          {
            MIL << "Cache dir not set. Downloading to temp dir: " << tmpMetadataDir() << std::endl;
            // as we have no local dir set we use a tmp one, but we use a member variable because
            // it cant go out of scope while the source exists.
            saveMetadataTo(tmpMetadataDir());
          }
          else
          {
            MIL << "Cached metadata not found in [" << _cache_dir << "]. Will download." << std::endl;
            saveMetadataTo(_cache_dir);
          }
        }
        MIL << "SUSETags source initialized." << std::endl;
        MIL << "   Url      : " << url() << std::endl;
        MIL << "   Path     : " << path() << std::endl;
        MIL << "   Data     : " << dataDir() << std::endl;
        MIL << "   Metadata : " << metadataRoot() << (_cache_dir.empty() ? " [TMP]" : " [CACHE]") << std::endl;
        MIL << "   N-Media  : " << numberOfMedia() << std::endl;
      }

      void SuseTagsImpl::createResolvables(Source_Ref source_r)
      {
        provideProducts ( source_r, _store );
        providePackages ( source_r, _store );
        provideSelections ( source_r, _store );
        providePatterns ( source_r, _store );
      }

      const std::list<Pathname> SuseTagsImpl::publicKeys()
      {
        std::list<std::string> files;
        std::list<Pathname> paths;

        MIL << "Reading public keys..." << std::endl;
        filesystem::readdir(files, metadataRoot() + "PUBLICKEYS");
        for( std::list<std::string>::const_iterator it = files.begin(); it != files.end(); ++it)
          paths.push_back(Pathname(*it));

        MIL << "read " << files.size() << " keys from cache " << metadataRoot() << std::endl;
        return paths;
      }

      ResStore SuseTagsImpl::createResolvablesByKind(Source_Ref source_r, Resolvable::Kind kind)
      {
        ResStore store;

        if ( kind == ResTraits<Product>::kind )
          provideProducts ( source_r, store );
        else if ( kind == ResTraits<Package>::kind )
          providePackages ( source_r, store );
        else if ( kind == ResTraits<Selection>::kind )
          provideSelections ( source_r, store );
        else if ( kind == ResTraits<Pattern>::kind )
          providePatterns ( source_r, store );
        return store;
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : SuseTagsImpl::~SuseTagsImpl
      //	METHOD TYPE : Dtor
      //
      SuseTagsImpl::~SuseTagsImpl()
      {}

      Pathname SuseTagsImpl::sourceDir( const std::string & dir )
      {
        return Pathname( _data_dir + Pathname( dir ) + "/");
      }

      media::MediaVerifierRef SuseTagsImpl::verifier(media::MediaNr media_nr)
      {
	return media::MediaVerifierRef(
    	    new SourceImpl::Verifier (_media_vendor, _media_id, media_nr));
      }

      unsigned SuseTagsImpl::numberOfMedia(void) const
      { return _media_count; }

      std::string SuseTagsImpl::vendor (void) const
      { return _prodImpl->vendor(); }

      std::string SuseTagsImpl::unique_id (void) const
      { return _media_id; }

      Date SuseTagsImpl::timestamp() const
      {
        return PathInfo(contentFile()).mtime();
      }

      void SuseTagsImpl::readContentFile(const Pathname &content_file)
      {
        SourceFactory factory;
        try {
          DBG << "Going to parse content file " << content_file << endl;

          ProductMetadataParser p;
          p.parse( content_file, factory.createFrom(this) );
          _product = p.result;

          // data dir is the same, it is determined by the content file
          _data_dir = _path + p.prodImpl->_data_dir;
          // description dir also changes when using cache
          _media_descr_dir = _path + p.prodImpl->_description_dir;

          MIL << "Read product: " << _product->summary() << endl;

          _prodImpl = p.prodImpl;
	  _autorefresh = p.volatile_content && media::MediaAccess::canBeVolatile( _url );
        }
        catch (Exception & excpt_r) {
          ZYPP_THROW (Exception( "cannot parse content file."));
        }
      }

      void SuseTagsImpl::provideProducts(Source_Ref source_r, ResStore &store)
      {
        MIL << "Adding product: " << _product->summary() << " to the store" << endl;
        store.insert( _product );
      }

      void SuseTagsImpl::providePackages(Source_Ref source_r, ResStore &store)
      {
        Pathname p = descrDir() + "packages";

        //verifyFile( p, "packages");

        DBG << "Going to parse " << p << endl;

        parser::ParserProgress::Ptr progress;
        //progress.reset( new parser::ParserProgress(npp) );
        
        callback::SendReport<SourceReport> report;
        SourceEventHandler npp(report);
        
        progress.reset( new parser::ParserProgress( npp ) );
        report->start( selfSourceRef(), "Parsing packages file" );
        PkgContent content( parsePackages( progress, source_r, this, p ) );
        report->finish( selfSourceRef(), "Parsing packages file", source::SourceReport::NO_ERROR, "" );
        
#warning Should use correct locale and locale fallback list
        // note, this locale detection has nothing to do with translated text.
        // basically we are only loading the data we need. Instead of parsing all
        // package description we fill the TranslatedText properties only
        // with the detected locale.

        ZYpp::Ptr z = getZYpp();
        Locale lang( z->getTextLocale() );

        std::string packages_lang_prefix( "packages." );
        std::string packages_lang_name;

        // get the list of available packages.X trsnlation files
        std::list<std::string> all_files;
        filesystem::readdir(all_files, descrDir());

        std::list<std::string> _pkg_translations;
        for( std::list<std::string>::const_iterator it = all_files.begin(); it != all_files.end(); ++it)
        {
          if ( ((*it).substr(0, 9) == "packages." ) && ((*it) != "packages.DU" ))
          {
            MIL << *it << " available as package data translation." << std::endl;
            _pkg_translations.push_back(*it);
          }
        }

        // find the most apropiate file
        bool trymore = true;
        while ( (lang != Locale()) && trymore )
        {
          packages_lang_name = packages_lang_prefix + lang.code();
          MIL << "Translation candidate: " << lang.code() << std::endl;
          try
          {
            // only provide it if it exists
            if ( find( _pkg_translations.begin(), _pkg_translations.end(), packages_lang_name ) != _pkg_translations.end() )
            {
              p = descrDir() + packages_lang_name;
              if ( PathInfo(p).isExist() )
              {
                MIL << packages_lang_name << " found" << std::endl;
                DBG << "Going to parse " << p << endl;
                //verifyFile( p, packages_lang_name);
                parser::ParserProgress::Ptr progress;
                NullParseProgress npp(p);
                progress.reset( new parser::ParserProgress(npp) );
                parsePackagesLang( progress, this, p, lang, content );
                trymore = false;
              }
              else
              {
                ERR << packages_lang_name << " can't be provided, even if it should" << endl;
              }
            }
            else
            {
              MIL << "Skipping translation candidate " << packages_lang_name << " (not present in media)" << endl;
            }
          }
          catch (Exception & excpt_r)
          {
            ZYPP_CAUGHT(excpt_r);
          }
          lang = lang.fallback();
        }

        MIL << _package_data.size() << " packages holding real data" << std::endl;
        MIL << content.size() << " packages parsed" << std::endl;

        int counter =0;
        for ( std::map<NVRA, DefaultFalseBool>::const_iterator it = _is_shared.begin(); it != _is_shared.end(); ++it)
        {
          if( it->second)
            counter++;
        }

        MIL << counter << " packages sharing data" << std::endl;


        PkgDiskUsage du;
        try
        {
          p = descrDir() +  + "packages.DU";
          //verifyFile( p, "packages.DU");
          parser::ParserProgress::Ptr progress;
          NullParseProgress npp(p);
          progress.reset( new parser::ParserProgress(npp) );
          du = parsePackagesDiskUsage(progress, p);
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

          //MIL << "Package " << pkg->summary() << std::endl;
          //MIL << "        " << pkg->description() << std::endl;
          //MIL << "----------------------------------" << std::endl;

        }
        DBG << "SuseTagsImpl (fake) from " << p << ": "
            << content.size() << " packages" << endl;
      }

      void SuseTagsImpl::provideSelections(Source_Ref source_r, ResStore &store)
      {
        Pathname p;

        bool file_found = true;

        // parse selections
        p = descrDir() + "selections";
        if ( ! PathInfo(p).isExist() )
        {
          MIL << p << " not found." << endl;
          file_found = false;
        }

        if (file_found)
        {
          //verifyFile( p, "selections");
          std::ifstream sels (p.asString().c_str());

          while (sels && !sels.eof())
          {
            std::string selfile;
            getline(sels,selfile);

            if (selfile.empty() ) continue;
              DBG << "Going to parse selection " << selfile << endl;

            Pathname file = descrDir() + selfile;
            //verifyFile( file, selfile);

            MIL << "Selection file to parse " << file << endl;
            parser::ParserProgress::Ptr progress;
            NullParseProgress npp(file);
            progress.reset( new parser::ParserProgress(npp) );
            Selection::Ptr sel( parseSelection( progress, source_r, file ) );

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
        Pathname p;

        // parse patterns
        bool file_found = true;

        p = descrDir() + "patterns";
        if ( ! PathInfo(p).isExist() )
        {
          MIL << p << " not found." << endl;
          file_found = false;
        }

        if ( file_found )
        {
          //verifyFile( p, "patterns");
          std::ifstream pats (p.asString().c_str());

          while (pats && !pats.eof())
          {
            std::string patfile;
            getline(pats,patfile);

            if (patfile.empty() ) continue;

            DBG << "Going to parse pattern " << patfile << endl;

            Pathname file = descrDir() + patfile;

            MIL << "Pattern file to parse " << file << endl;
            parser::ParserProgress::Ptr progress;
            NullParseProgress npp(file);
            progress.reset( new parser::ParserProgress(npp) );
            Pattern::Ptr pat( parsePattern( progress, source_r, file ) );

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
