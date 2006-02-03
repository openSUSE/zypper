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

#include "zypp/source/SourceImpl.h"

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
    SourceImpl::SourceImpl(media::MediaId & media_r,
                           const Pathname & path_r,
			   const std::string & alias_r)
    : _media(media_r)
    , _path(path_r)
    , _enabled(true)
    , _alias (alias_r)
    , _priority (0)
    , _priority_unsubscribed (0)
    , _res_store_initialized(false)
    {
      try {
	Pathname media_file = "/media.1/media";

	media_mgr.provideFile (_media, 1, media_file);
	media_file = media_mgr.localPath(_media, media_file);

#warning check the stream status
	std::ifstream str(media_file.asString().c_str());
	std::string media_vendor;
	std::string media_id;
	getline(str, media_vendor);
	getline(str, media_id);

	media_mgr.delVerifier(_media);
	
	MIL << "Adding proper media verifier" << endl;
	
	media_mgr.addVerifier(_media, media::MediaVerifierRef(new Verifier(media_vendor, media_id)));
      }
      catch (const Exception & excpt_r)
      {
#warning FIXME: If media data is not set, verifier is not set. Should the media be refused instead?
	ZYPP_CAUGHT(excpt_r);
	WAR << "Verifier not found" << endl;
      }
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : SourceImpl::~SourceImpl
    //	METHOD TYPE : Dtor
    //
    SourceImpl::~SourceImpl()
    {}

    const ResStore & SourceImpl::resolvables(Source_Ref source_r) const
    {
      if ( !_res_store_initialized )
      {
        // cast away const to allow late init
	const_cast<SourceImpl*>(this)->createResolvables(source_r);
	const_cast<SourceImpl*>(this)->_res_store_initialized = true;
      }
      return _store;
     }

    const Pathname SourceImpl::provideFile(const Pathname & file_r,
					   const unsigned media_nr, 
					   bool cached, 
					   bool checkonly )
    {
      media_mgr.provideFile (_media, media_nr, file_r, cached, checkonly);
      return media_mgr.localPath(_media, file_r);
    }

    /** Provide a directory to local filesystem */
    const Pathname SourceImpl::provideDir(const Pathname & path_r,
					  const unsigned media_nr,
					  const bool recursive)
    {
      if (recursive)
	media_mgr.provideDirTree(_media, media_nr, path_r);
      else
	media_mgr.provideDir(_media, media_nr, path_r);
      return media_mgr.localPath(_media, path_r);
    }

    void SourceImpl::createResolvables(Source_Ref source_r)
    {}


    std::string SourceImpl::zmdname (void) const
    { return "zmdname"; }

    std::string SourceImpl::zmddescription (void) const
    { return "zmddescription"; }

    unsigned SourceImpl::priority (void) const
    { return _priority; }

    unsigned SourceImpl::priority_unsubscribed (void) const
    { return _priority_unsubscribed; }

    Url SourceImpl::url (void) const
    { return media_mgr.url(_media); }

    const Pathname & SourceImpl::path (void) const
    { return _path; }

    std::ostream & SourceImpl::dumpOn( std::ostream & str ) const
    { return str << (_alias.empty() ? "SourceImpl" : _alias); }

    SourceImpl::Verifier::Verifier(const std::string & vendor_r, const std::string & id_r)
    : _media_vendor(vendor_r)
    , _media_id(id_r)
    {}

    bool SourceImpl::Verifier::isDesiredMedia(const media::MediaAccessRef &ref, media::MediaNr mediaNr)
    {
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
