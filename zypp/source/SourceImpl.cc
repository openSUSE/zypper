/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/SourceImpl.cc
 *
*/
#include <iostream>
#include <fstream>
#include "zypp/base/Logger.h"
#include "zypp/Digest.h"
#include "zypp/SourceFactory.h"
#include "zypp/source/SourceImpl.h"
#include "zypp/ZYppCallbacks.h"
#include "zypp/SourceManager.h"
#include "zypp/ZYppFactory.h"
#include <fstream>

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////

    IMPL_PTR_TYPE(SourceImpl);


    class DownloadProgressPackageReceiver
	: public callback::ReceiveReport<media::DownloadProgressReport>
    {
	callback::SendReport <DownloadResolvableReport> & _report;
	Resolvable::constPtr _resolvable;

      public:

	DownloadProgressPackageReceiver (
	    callback::SendReport <DownloadResolvableReport> & report_r,
	    Resolvable::constPtr resolvable_r
	)
	: _report (report_r)
	, _resolvable (resolvable_r)
	{}

	virtual ~DownloadProgressPackageReceiver () {}

	virtual void reportbegin() {}

	virtual void reportend() {}

        /**
         * Inform about progress
         * Return true on abort
         */
        virtual bool progress( int percent, Url )
	{
	    return _report->progress( percent, _resolvable );
	}
    };


    class DownloadProgressFileReceiver
	: public callback::ReceiveReport<media::DownloadProgressReport>
    {
	callback::SendReport <DownloadFileReport> & _report;

      public:

	DownloadProgressFileReceiver (
	    callback::SendReport <DownloadFileReport> & report_r
	)
	: _report (report_r)
	{}

	virtual ~DownloadProgressFileReceiver () {}

	virtual void reportbegin() {}

	virtual void reportend() {}

        /**
         * Inform about progress
         * Return true on abort
         */
        virtual bool progress( int percent, Url url )
	{
	    return _report->progress( percent, url );
	}
    };

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : SourceImpl::SourceImpl
    //	METHOD TYPE : Ctor
    //
    SourceImpl::SourceImpl()
    : _enabled(true)
    , _autorefresh(true)
    , _priority (0)
    , _priority_unsubscribed (0)
    , _subscribed(false)
    , _base_source(false)
    , _tmp_metadata_dir(getZYpp()->tmpPath())
    , _res_store_initialized(false)
    {
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : SourceImpl::factoryCtor
    //	METHOD TYPE : void
    //
    void SourceImpl::factoryCtor( const media::MediaId & media_r,
                                  const Pathname & path_r,
                                  const std::string & alias_r,
                                  const Pathname cache_dir_r,
				  const bool base_source)
    {
      _media_set = new MediaSet( selfSourceRef() );
      _url = media_mgr.url( media_r );
      _media_set->redirect( 1, media_r );
      _path      = path_r;
      _alias     = alias_r;
      _cache_dir = cache_dir_r;
      _subscribed = true;
      _base_source = base_source;

      // for sources which are neither CD nor DVD we enable autorefresh by default
      _autorefresh = media::MediaAccess::canBeVolatile( _url );

      try
        {
          factoryInit();
        }
      catch ( Exception & excpt )
        {
          _store.clear();
          ZYPP_RETHROW( excpt );
        }
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : SourceImpl::factoryInit
    //	METHOD TYPE : void
    //
    void SourceImpl::factoryInit()
    {
      ZYPP_THROW( Exception( "FactoryInit not implemented!" ) );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : SourceImpl::~SourceImpl
    //	METHOD TYPE : Dtor
    //
    SourceImpl::~SourceImpl()
    {
      if (_media_set) {
	media::MediaAccessId _media = _media_set->getMediaAccessId( 1 );
	media_mgr.release (_media, false);
      }
    }

    const ResStore & SourceImpl::resolvables() const
    {
      if ( !_res_store_initialized )
      {
        // cast away const to allow late init
        Source_Ref self( const_cast<SourceImpl*>(this)->selfSourceRef() );
	const_cast<SourceImpl*>(this)->createResolvables(self);
	const_cast<SourceImpl*>(this)->_res_store_initialized = true;
      }
      return _store;
     }

    const ResStore SourceImpl::resolvables(zypp::Resolvable::Kind kind) const
    {
      Source_Ref self( const_cast<SourceImpl*>(this)->selfSourceRef() );
      return const_cast<SourceImpl*>(this)->provideResolvables(self, kind);
    }

    Date SourceImpl::timestamp() const
    {
      return Date::now();
    }

    void SourceImpl::dirInfo(const unsigned media_nr,
                             std::list<std::string> &retlist,
                             const Pathname         &path_r,
                             bool                    dots ) const
    {
      DBG << "reading " << path_r << " file list" << endl;
      media::MediaAccessId _media = _media_set->getMediaAccessId( media_nr );
      media_mgr.dirInfo( _media, retlist, path_r, dots );
    }

    const Pathname SourceImpl::providePackage( Package::constPtr package )
    {
      bool retry = true;
      bool digest_ok = false;
      Pathname file;
      callback::SendReport<source::DownloadResolvableReport> report;
      DownloadProgressPackageReceiver download_report( report, package );

      while (retry)
      {
        report->start( package, package->source().url() );

	callback::TempConnect<media::DownloadProgressReport> tmp_download( download_report );

        file = provideJustFile( package->location(), package->sourceMediaNr());

        report->finish( package, source::DownloadResolvableReport::NO_ERROR, "" );

        CheckSum checksum = package->checksum();
        std::string calculated_digest;

        // check digest
        try
        {
          std::ifstream is(file.asString().c_str(), std::ifstream::binary);
          calculated_digest = Digest::digest(checksum.type(), is);
          is.close();
        }
        catch (std::exception &e)
        {
          ERR << "Can't open " << file << " for integrity check." << std::endl;
        }

        if ( checksum.checksum() == calculated_digest )
        {
          MIL << package->location() << " ok. [" << calculated_digest << "]" << std::endl;
          digest_ok = true;
          retry = false;
        }

        if (!digest_ok)
        {
          std::string  package_str = package->name() + "-" + package->edition().asString();
          source::DownloadResolvableReport::Action useraction = report->problem(package, source::DownloadResolvableReport::INVALID, "Package " + package_str + " fails integrity check. Do you want to retry downloading it, or abort installation?");

          if( useraction == source::DownloadResolvableReport::ABORT )
          {
            ZYPP_THROW(Exception("Package " + package->location().asString() + " fails integrity check. Expected: [" + checksum.checksum() + "] Read: [" + calculated_digest + "] (" + checksum.type() + ")"));
          }
          else if ( useraction == source::DownloadResolvableReport::RETRY )
          {
            retry = true;
          }
        }
      }
      return file;
    }

    void SourceImpl::resetMediaVerifier()
    {
      try
      {
        media::MediaManager media_mgr;
        MIL << "Reseting media verifier" << std::endl;

        // don't try to attach media
        media::MediaAccessId _media = _media_set->getMediaAccessId(1, true);
        media_mgr.delVerifier(_media);
        media_mgr.addVerifier(_media, media::MediaVerifierRef(new media::NoVerifier()));
      }
      catch (const Exception & excpt_r)
      {
          ZYPP_CAUGHT(excpt_r);
          WAR << "Media Verifier not found." << endl;
      }
    }

    const Pathname SourceImpl::provideFile(const Pathname & file_r,
					   const unsigned media_nr,
					   bool cached,
					   bool checkonly )
    {
      callback::SendReport<source::DownloadFileReport> report;
      DownloadProgressFileReceiver download_report( report );

      SourceFactory source_factory;

      Url file_url( url().asString() + file_r.asString() );

      report->start( source_factory.createFrom(this), file_url );

      callback::TempConnect<media::DownloadProgressReport> tmp_download( download_report );

      Pathname file = provideJustFile( file_r, media_nr, cached, checkonly );

      report->finish( file_url, source::DownloadFileReport::NO_ERROR, "" );

      return file;
    }

    const Pathname SourceImpl::tryToProvideFile( const Pathname & file, const unsigned media_nr )
    {
      media::MediaAccessId _media = _media_set->getMediaAccessId( media_nr);
      media_mgr.provideFile (_media, file, false, false);
      return media_mgr.localPath( _media, file );
    }

    void SourceImpl::copyLocalMetadata(const Pathname &src, const Pathname &dst) const
    {
      // refuse to use stupid paths as cache dir
      if (dst == Pathname("/") )
        ZYPP_THROW(Exception("I refuse to use / as local dir"));

      if (0 != assert_dir(dst, 0755))
        ZYPP_THROW(Exception("Cannot create local directory" + dst.asString()));

      MIL << "Cleaning up local dir" << std::endl;
      filesystem::clean_dir(dst);
      MIL << "Copying " << src << " content to local : " << dst << std::endl;

      if ( copy_dir_content( src, dst) != 0)
      {
        filesystem::clean_dir(dst);
        ZYPP_THROW(Exception( "Can't copy downloaded data to local dir. local dir cleaned."));
      }
    }

    const Pathname SourceImpl::provideJustFile(const Pathname & file_r,
					   const unsigned media_nr,
					   bool cached,
					   bool checkonly )
    {
      callback::SendReport<media::MediaChangeReport> report;

      SourceFactory source_factory;

      // get the mediaId, but don't try to attach it here
      media::MediaAccessId _media = _media_set->getMediaAccessId( media_nr, true );
      do {
        try {
	  DBG << "Going to try provide file " << file_r << " from " << media_nr << endl;

	  // try to attach the media
	  _media = _media_set->getMediaAccessId( media_nr ); // in case of redirect
	  media_mgr.provideFile (_media, file_r, cached, checkonly);
	  break;
        }
	catch ( Exception & excp )
        {
	  ZYPP_CAUGHT(excp);
	    media::MediaChangeReport::Action user;

  	  do {

	    DBG << "Media couldn't provide file " << file_r << " , releasing." << endl;
	    try {
		media_mgr.release (_media, false);
	    }
	    catch (const Exception & excpt_r)
	    {
		ZYPP_CAUGHT(excpt_r);
		MIL << "Failed to release media " << _media << endl;
	    }
	    MIL << "Releasing all medias of all sources" << endl;
	    try {
		zypp::SourceManager::sourceManager()->releaseAllSources();
	    }
	    catch (const zypp::Exception& excpt_r)
	    {
		ZYPP_CAUGHT(excpt_r);
		ERR << "Failed to release all sources" << endl;
	    }

	    // set up the reason
	    media::MediaChangeReport::Error reason
		= media::MediaChangeReport::INVALID;

	    if( typeid(excp) == typeid( media::MediaFileNotFoundException )  ||
	      typeid(excp) == typeid( media::MediaNotAFileException ) )
	    {
		reason = media::MediaChangeReport::NOT_FOUND;
	    } else if( typeid(excp) == typeid( media::MediaNotDesiredException)  ||
	      typeid(excp) == typeid( media::MediaNotAttachedException) )
	    {
		reason = media::MediaChangeReport::WRONG;
	    }

	    user  = checkonly ? media::MediaChangeReport::ABORT :
	      report->requestMedia (
		source_factory.createFrom( this ),
		media_nr,
		reason,
		excp.asUserString()
	      );

	    DBG << "ProvideFile exception caught, callback answer: " << user << endl;

	    if( user == media::MediaChangeReport::ABORT )
	    {
	      DBG << "Aborting" << endl;
	      ZYPP_RETHROW ( excp );
	    }
	    else if ( user == media::MediaChangeReport::IGNORE )
	    {
	      DBG << "Skipping" << endl;
	      ZYPP_THROW ( SkipRequestedException("User-requested skipping of a file") );
 	    }
	    else if ( user == media::MediaChangeReport::EJECT )
	    {
	      DBG << "Eject: try to release" << endl;
	      try {
		zypp::SourceManager::sourceManager()->releaseAllSources();
	      }
	      catch (const zypp::Exception& excpt_r)
	      {
		ZYPP_CAUGHT(excpt_r);
		ERR << "Failed to release all sources" << endl;
	      }
	      media_mgr.release (_media, true); // one more release needed for eject
	      // FIXME: this will not work, probably
	    }
	    else if ( user == media::MediaChangeReport::RETRY  ||
	    user == media::MediaChangeReport::CHANGE_URL )
	    {
	      // retry
	      DBG << "Going to try again" << endl;

	      // not attaching, media set will do that for us
	      // this could generate uncaught exception (#158620)

	      break;
	    }
	    else {
	      DBG << "Don't know, let's ABORT" << endl;

    	      ZYPP_RETHROW ( excp );
	    }
          } while( user == media::MediaChangeReport::EJECT );
        }

	// retry or change URL
      } while( true );

      return media_mgr.localPath( _media, file_r );
    }

    const Pathname SourceImpl::provideDirTree(const Pathname & path_r, const unsigned media_nr)
    {
      callback::SendReport<media::MediaChangeReport> report;
      SourceFactory source_factory;
      // get the mediaId, but don't try to attach it here
      media::MediaAccessId _media = _media_set->getMediaAccessId( media_nr, true );
      do
      {
        try
        {
          DBG << "Going to try provide tree " << path_r << " from " << media_nr << endl;
          // try to attach the media
          _media = _media_set->getMediaAccessId( media_nr ); // in case of redirect
          media_mgr.provideDirTree (_media, path_r);
          break;
        }
        catch ( Exception & excp )
        {
          ZYPP_CAUGHT(excp);
          media::MediaChangeReport::Action user;

          do
          {
            DBG << "Media couldn't provide tree " << path_r << " , releasing." << endl;
            try
            {
              media_mgr.release (_media, false);
            }
            catch (const Exception & excpt_r)
            {
              ZYPP_CAUGHT(excpt_r);
              MIL << "Failed to release media " << _media << endl;
            }
            MIL << "Releasing all medias of all sources" << endl;
            try
            {
              zypp::SourceManager::sourceManager()->releaseAllSources();
            }
            catch (const zypp::Exception& excpt_r)
            {
              ZYPP_CAUGHT(excpt_r);
              ERR << "Failed to release all sources" << endl;
            }
            // set up the reason
            media::MediaChangeReport::Error reason = media::MediaChangeReport::INVALID;

            if( typeid(excp) == typeid( media::MediaFileNotFoundException )  || typeid(excp) == typeid( media::MediaNotAFileException ) )
            {
              reason = media::MediaChangeReport::NOT_FOUND;
            }
            else if( typeid(excp) == typeid( media::MediaNotDesiredException)  || typeid(excp) == typeid( media::MediaNotAttachedException) )
            {
              reason = media::MediaChangeReport::WRONG;
            }
            user  = report->requestMedia ( source_factory.createFrom( this ), media_nr, reason, excp.asUserString() );

            DBG << "ProvideFile exception caught, callback answer: " << user << endl;

            if( user == media::MediaChangeReport::ABORT )
            {
              DBG << "Aborting" << endl;
              ZYPP_RETHROW ( excp );
            }
            else if ( user == media::MediaChangeReport::EJECT )
            {
              DBG << "Eject: try to release" << endl;
              try {
                zypp::SourceManager::sourceManager()->releaseAllSources();
              }
              catch (const zypp::Exception& excpt_r)
              {
                ZYPP_CAUGHT(excpt_r);
                ERR << "Failed to release all sources" << endl;
              }
              media_mgr.release (_media, true); // one more release needed for eject
              // FIXME: this will not work, probably
            }
            else if ( user == media::MediaChangeReport::RETRY  ||
                      user == media::MediaChangeReport::CHANGE_URL )
            {
              // retry
              DBG << "Going to try again" << endl;

              // not attaching, media set will do that for us
              // this could generate uncaught exception (#158620)

              break;
            }
            else
            {
              DBG << "Don't know, let's ABORT" << endl;

              ZYPP_RETHROW ( excp );
            }
          } while( user == media::MediaChangeReport::EJECT );
        }
      // retry or change URL
      } while( true );
      return media_mgr.localPath( _media, path_r );
    }

    const void SourceImpl::releaseFile(const Pathname & file_r,
				       const unsigned media_nr)
    {
      DBG << "releaseFile(" << file_r << ", " << media_nr << ")" << endl;
      media::MediaAccessId _media = _media_set->getMediaAccessId( media_nr );
      media_mgr.releaseFile(_media, file_r);
    }

    const void SourceImpl::releaseDir(const Pathname & path_r,
				      const unsigned media_nr,
				      const bool recursive)
    {
      DBG << "releaseDir(" << path_r << ", " << media_nr << (recursive?", recursive":"") << ")" << endl;
      media::MediaAccessId _media = _media_set->getMediaAccessId( media_nr );
      if (recursive)
	media_mgr.releasePath(_media, path_r);
      else
	media_mgr.releaseDir(_media, path_r);
    }

    void SourceImpl::changeMedia( const media::MediaId & media_r, const Pathname & path_r )
    {
      DBG << "changeMedia(" << path_r << ")" << endl;
      _url = media_mgr.url( media_r );
      _media_set->reset();
      _media_set->redirect( 1, media_r );
      _path = path_r;
    }

    void SourceImpl::enable()
    {
//      if (autorefresh())
//	refresh();
      _enabled = true;
    }

    void SourceImpl::createResolvables(Source_Ref source_r)
    {}

    ResStore SourceImpl::provideResolvables(Source_Ref source_r, zypp::Resolvable::Kind kind)
    {
	WAR << "provideResolvables not implemented by the source" << endl;
	return ResStore();
    }

    void SourceImpl::storeMetadata(const Pathname & cache_dir_r)
    {}

    void SourceImpl::refresh()
    {
	// TODO: will this work in chroot?
	// TODO: better download somewhere else and then copy over
	try{
  	  storeMetadata( _cache_dir );
	}
	catch( const zypp::Exception & excpt )
	{
	  ERR << "Unable to refresh the source cache" << endl;
	  if( ! _cache_dir.empty() && _cache_dir != "/" )
	    filesystem::clean_dir( _cache_dir );

	  ZYPP_RETHROW( excpt );
	}
    }

    void SourceImpl::redirect(unsigned media_nr, const Url & new_url)
    {
      DBG << "redirect(" << new_url << ")" << endl;
      media::MediaAccessId id = media_mgr.open( new_url );
      _media_set->redirect( media_nr, id );
    }
    void SourceImpl::reattach(const Pathname &attach_point)
    {
      DBG << "reattach(" << attach_point << ")" << endl;
      _media_set->reattach( attach_point );
    }

    void SourceImpl::release()
    {
      if (_media_set)
	_media_set->release();
    }

    media::MediaVerifierRef SourceImpl::verifier(unsigned media_nr)
    { return media::MediaVerifierRef(new media::NoVerifier()); }

    /////////////////////////////////////////////////////////////////
    // attribute accessors

    std::string SourceImpl::type (void) const
    { return "undefined"; }

    std::string SourceImpl::id (void) const
    { return _id; }

    void SourceImpl::setId (const std::string id_r)
    { _id = id_r; }

    unsigned SourceImpl::priority (void) const
    { return _priority; }

    void SourceImpl::setPriority (unsigned p)
    { _priority = p; }

    unsigned SourceImpl::priorityUnsubscribed (void) const
    { return _priority_unsubscribed; }

    void SourceImpl::setPriorityUnsubscribed (unsigned p)
    { _priority_unsubscribed = p; }

    bool SourceImpl::subscribed (void) const
    { return _subscribed; }

    void SourceImpl::setSubscribed (bool s)
    { _subscribed = s; }

    const Pathname & SourceImpl::cacheDir (void)
    { return _cache_dir; }

    Url SourceImpl::url (void) const
    { return _url; }

    void SourceImpl::setUrl( const Url & url )
    { _url = url; }

    bool SourceImpl::remote() const
    {
      bool downloads = false;
      try {
        downloads = media::MediaManager::downloads(_url);
      }
      catch(const zypp::Exception &e)
      {
        // should not happen, but ...
        ZYPP_CAUGHT(e);
      }
      return downloads;
    }

    const Pathname & SourceImpl::path (void) const
    { return _path; }

    unsigned SourceImpl::numberOfMedia(void) const
    { return 1; }

    std::string SourceImpl::vendor (void) const
    { return ""; }

    const std::list<Pathname> SourceImpl::publicKeys()
    { return std::list<Pathname>(); }

    std::string SourceImpl::unique_id (void) const
    { return ""; }

    /////////////////////////////////////////////////////////////////
    /**
     * ZMD specific stuff
     */

    std::string SourceImpl::zmdName (void) const
    { return "zmdname"; }

    void SourceImpl::setZmdName (const std::string name_r)
    { return; }

    std::string SourceImpl::zmdDescription (void) const
    { return "zmddescription"; }

    void SourceImpl::setZmdDescription (const std::string desc_r)
    { return; }

    /////////////////////////////////////////////////////////////////

    std::ostream & SourceImpl::dumpOn( std::ostream & str ) const
    {
      str << "Source[" << numericId() << "|" << type();
      if ( !_alias.empty() )
        str << "|" << _alias;
      str << "]";

      str << "{"
          << _url << "(" << _path << ")";
      if ( ! _cache_dir.empty() )
        str << "; cache " << _cache_dir;
      str << "}";

      return str;
    }

    SourceImpl::Verifier::Verifier(const std::string & vendor_r, const std::string & id_r, const media::MediaNr media_nr)
    : _media_vendor(vendor_r)
    , _media_id(id_r)
    , _media_nr(media_nr)
    {}

    bool SourceImpl::Verifier::isDesiredMedia(const media::MediaAccessRef &ref)
    {
      if (_media_vendor.empty() || _media_id.empty())
	return true;
#warning TODO define what does missing media_id/media_vendor mean

      Pathname media_file = "/media." + str::numstring(_media_nr) + "/media";
      ref->provideFile (media_file);
      media_file = ref->localPath(media_file);
      std::ifstream str(media_file.asString().c_str());
      std::string vendor;
      std::string id;

#warning check the stream status
      getline(str, vendor);
      getline(str, id);

      return (vendor == _media_vendor && id == _media_id );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
