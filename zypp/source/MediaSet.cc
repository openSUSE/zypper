/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/MediaSet.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp/SourceFactory.h"
#include "zypp/source/MediaSet.h"
#include "zypp/ZYppCallbacks.h"

#include <fstream>

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////

    IMPL_PTR_TYPE(MediaSet);

    MediaSet::MediaSet(const Source_Ref & source_r)
    {
      _source = source_r;
    }
    MediaSet::~MediaSet()
    {
      MIL << "Called MediaSet destructor" << endl;
      release();
      MIL << "Closing all medias of source" << endl;
      media::MediaManager media_mgr;
      for (MediaMap::iterator it = medias.begin(); it != medias.end(); it++)
      {
	MIL << "Closing media " << it->second << endl;
	media_mgr.close(it->second);
      }
    }

    void MediaSet::redirect (media::MediaNr medianr, media::MediaAccessId media_id)
    {
      media::MediaManager media_mgr;
      MediaMap::iterator  it( medias.find(medianr));
      if( it != medias.end() && media_mgr.isOpen(it->second)) {
	try {
	  DBG << "Closing media access id " << it->second << endl;
	  media_mgr.close(it->second);
	}
	// paranoia ...
	catch (const Exception & excpt_r) {
	  ZYPP_CAUGHT(excpt_r);
	}
      }

      medias[medianr] = media_id;
    }

    void MediaSet::reattach(const Pathname &attach_point)
    {
      media::MediaManager media_mgr;
      media_mgr.setAttachPrefix(attach_point);
      for (MediaMap::iterator it = medias.begin(); it != medias.end(); it++)
      {
	Url url = media_mgr.url(it->second);
	std::string scheme = url.getScheme();
	if (scheme == "http" || scheme == "ftp" || scheme == "https" || scheme == "ftps")
	{
	  media_mgr.release(it->second);
	  media_mgr.attach(it->second);
	}
      }
    }

    void MediaSet::reset()
    {
      media::MediaManager media_mgr;
      for (MediaMap::iterator it = medias.begin(); it != medias.end(); it++)
      {
        if( media_mgr.isOpen(it->second)) {
	  try {
	    DBG << "Closing media access id " << it->second << endl;
	    media_mgr.close(it->second);
	  }
	  // paranoia ...
	  catch (const Exception & excpt_r) {
	    ZYPP_CAUGHT(excpt_r);
	  }
        }
      }
      medias = MediaMap();
    }

    void MediaSet::release()
    {
      MIL << "Releasing all medias of source" << endl;
      media::MediaManager media_mgr;
      for (MediaMap::iterator it = medias.begin(); it != medias.end(); it++)
      {
	if (media_mgr.isAttached(it->second))
	{
	  MIL << "Releasing media " << it->second << endl;
	  media_mgr.release(it->second, false);
	}
	else
	{
	  MIL << "Media " << it->second << " not attached" << endl;
	}
      }
    }

    media::MediaAccessId MediaSet::getMediaAccessId (media::MediaNr medianr, bool noattach)
    {
     media::MediaManager media_mgr;

     if (medias.find(medianr) != medias.end())
      {
	media::MediaAccessId id = medias[medianr];
	if (! noattach && ! media_mgr.isAttached(id))
	  media_mgr.attach(id);
	return id;
      }
      Url url = _source.url();
      url = rewriteUrl (url, medianr);
      media::MediaAccessId id = media_mgr.open(url, _source.path());
      try {
	MIL << "Adding media verifier" << endl;
	media_mgr.delVerifier(id);
	media_mgr.addVerifier(id, _source.verifier(medianr));
      }
      catch (const Exception & excpt_r)
      {
#warning FIXME: If media data is not set, verifier is not set. Should the media be refused instead?
	ZYPP_CAUGHT(excpt_r);
	WAR << "Verifier not found" << endl;
      }
      medias[medianr] = id;
      
      if (! noattach)
        media_mgr.attach(id);

      return id;
    }

    Url MediaSet::rewriteUrl (const Url & url_r, const media::MediaNr medianr)
    {
      std::string scheme = url_r.getScheme();
      if (scheme == "cd" || scheme == "dvd")
	return url_r;

      DBG << "Rewriting url " << url_r << endl;

      if( scheme == "iso")
      {
	std::string isofile = url_r.getQueryParam("iso");
	boost::regex e("^(.*(cd|dvd))([0-9]+)(\\.iso)$", boost::regex::icase);
	boost::smatch what;
	if(boost::regex_match(isofile, what, e, boost::match_extra))
	{
	  Url url( url_r);

          isofile = what[1] + str::numstring(medianr) + what[4];
	  url.setQueryParam("iso", isofile);

          DBG << "Url rewrite result: " << url << endl;
	  return url;
	}
      }
      else
      {
        std::string pathname = url_r.getPathName();
        boost::regex e("^(.*(cd|dvd))([0-9]+)(/?)$", boost::regex::icase);
        boost::smatch what;
        if(boost::regex_match(pathname, what, e, boost::match_extra))
        {
	  Url url( url_r);

	  pathname = what[1] + str::numstring(medianr) + what[4];
	  url.setPathName(pathname);

          DBG << "Url rewrite result: " << url << endl;

	  return url;
        }
      }
      return url_r;
    }

    std::ostream & MediaSet::dumpOn( std::ostream & str ) const
    { return str << "MediaSet"; }


    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
