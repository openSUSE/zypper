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
#include "zypp/base/Logger.h"

#include "zypp/SourceFactory.h"
#include "zypp/source/SourceImpl.h"
#include "zypp/ZYppCallbacks.h"

#include <fstream>

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////

    IMPL_PTR_TYPE(SourceImpl);


    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : SourceImpl::SourceImpl
    //	METHOD TYPE : Ctor
    //
    SourceImpl::SourceImpl()
    : _enabled(true)
    , _priority (0)
    , _priority_unsubscribed (0)
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
                                  const Pathname cache_dir_r )
    {
      _media_set = new MediaSet( selfSourceRef() );
      _url = media_mgr.url( media_r );
      _media_set->redirect( 1, media_r );
      _path      = path_r;
      _alias     = alias_r;
      _cache_dir = cache_dir_r;
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
    {}

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

    void SourceImpl::dirInfo(const unsigned media_nr,
                             std::list<std::string> &retlist,
                             const Pathname         &path_r,
                             bool                    dots ) const
    {
      DBG << "reading " << path_r << " file list" << endl;
      media::MediaAccessId _media = _media_set->getMediaAccessId( media_nr );
      media_mgr.dirInfo( _media, retlist, path_r, dots );
    }

    const Pathname SourceImpl::provideFile(const Pathname & file_r,
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

	    DBG << "Media couldn't provide file, releasing." << endl;
	    try {
		media_mgr.release (_media, false);
	    }
	    catch (const Exception & excpt_r)
	    {
		ZYPP_CAUGHT(excpt_r);
		MIL << "Failed to release media " << _media << endl;
	    }

	    user  = checkonly ? media::MediaChangeReport::ABORT :
	      report->requestMedia (
		source_factory.createFrom( this ),
		media_nr,
		media::MediaChangeReport::WRONG, // FIXME: proper error
		excp.asUserString()
	      );

	    DBG << "ProvideFile exception caught, callback answer: " << user << endl;

	    if( user == media::MediaChangeReport::ABORT )
	    {
	      DBG << "Aborting" << endl;
	      ZYPP_RETHROW ( excp );
	    }
	    else if ( user == media::MediaChangeReport::EJECT )
	    {
	      DBG << "Eject: try to release" << endl;

	      media_mgr.release (_media, true);
	      // FIXME: this will not work, probably
	    }
	    else if ( user == media::MediaChangeReport::RETRY  ||
	    user == media::MediaChangeReport::CHANGE_URL )
	    {
	      // retry
	      DBG << "Going to attach again" << endl;

	      media_mgr.attach( _media );

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

    /** Provide a directory to local filesystem */
    const Pathname SourceImpl::provideDir(const Pathname & path_r,
					  const unsigned media_nr,
					  const bool recursive)
    {
      DBG << "provideDir(" << path_r << ", " << media_nr << (recursive?", recursive":"") << ")" << endl;
      media::MediaAccessId _media = _media_set->getMediaAccessId( media_nr );
      if (recursive)
	media_mgr.provideDirTree( _media, path_r );
      else
	media_mgr.provideDir( _media, path_r );
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
	storeMetadata( _cache_dir );
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

    const Pathname & SourceImpl::cacheDir (void)
    { return _cache_dir; }

    Url SourceImpl::url (void) const
    { return _url; }

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
