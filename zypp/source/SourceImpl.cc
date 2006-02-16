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

    media::MediaManager media_mgr;

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
      _media_set = new MediaSet(selfSourceRef());
      _url = media_mgr.url(media_r);
      _media_set->redirect(1, media_r);
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
    {}

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

    const Pathname SourceImpl::provideFile(const Pathname & file_r,
					   const unsigned media_nr,
					   bool cached,
					   bool checkonly )
    {

      callback::SendReport<media::MediaChangeReport> report;

      SourceFactory source_factory;

      media::MediaAccessId _media;
      do {
        try {
	  DBG << "Going to try provide file " << file_r << " from " << media_nr << endl;
	  _media = _media_set->getMediaAccessId(media_nr); // in case of redirect
	  media_mgr.provideFile (_media, file_r, cached, checkonly);
	  break;
        }
	catch ( Exception & excp )
        {
	    media::MediaChangeReport::Action user;
  	  do {
	    user  = checkonly ? media::MediaChangeReport::ABORT :
	      report->requestMedia (
		source_factory.createFrom(this),
		media_nr,
		media::MediaChangeReport::WRONG, // FIXME: proper error
		excp.msg()
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
	      DBG << "Going to release and attach again" << endl;
	    
	      media_mgr.release (_media, false);
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

      return media_mgr.localPath(_media, file_r);
    }

    /** Provide a directory to local filesystem */
    const Pathname SourceImpl::provideDir(const Pathname & path_r,
					  const unsigned media_nr,
					  const bool recursive)
    {
      media::MediaAccessId _media = _media_set->getMediaAccessId(media_nr);
      if (recursive)
	media_mgr.provideDirTree(_media, path_r);
      else
	media_mgr.provideDir(_media, path_r);
      return media_mgr.localPath(_media, path_r);
    }

    void SourceImpl::changeMedia(const media::MediaId & media_r, const Pathname & path_r)
    {
      _url = media_mgr.url(media_r);
      _media_set->reset();
      _media_set->redirect(1, media_r);
      _path = path_r;
    }

    void SourceImpl::createResolvables(Source_Ref source_r)
    {}

    void SourceImpl::storeMetadata(const Pathname & cache_dir_r)
    {}

    void SourceImpl::redirect(unsigned media_nr, const Url & new_url)
    {
      media::MediaAccessId id = media_mgr.open(new_url);
      _media_set->redirect(media_nr, id);
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

    Url SourceImpl::url (void) const
    { return _url; }

    const Pathname & SourceImpl::path (void) const
    { return _path; }

    unsigned SourceImpl::numberOfMedia(void) const
    { return 1; }

    std::string SourceImpl::vendor (void) const
    { return ""; }

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
    { return str << (_alias.empty() ? "SourceImpl" : _alias); }

    SourceImpl::Verifier::Verifier(const std::string & vendor_r, const std::string & id_r)
    : _media_vendor(vendor_r)
    , _media_id(id_r)
    {}

    bool SourceImpl::Verifier::isDesiredMedia(const media::MediaAccessRef &ref)
    {
      media::MediaNr mediaNr = 1; // FIXME!!
      if (_media_vendor.empty() || _media_id.empty())
	return true;
#warning TODO define what does missing media_id/media_vendor mean

      Pathname media_file = "/media." + str::numstring(mediaNr) + "/media";
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
