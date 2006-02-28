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

    void MediaSet::redirect (media::MediaNr medianr, media::MediaAccessId media_id)
    {
      medias[medianr] = media_id;
    }

    void MediaSet::reset()
    {
      medias = MediaMap();
    }

    void MediaSet::release()
    {
      media::MediaManager media_mgr;
      for (MediaMap::iterator it = medias.begin(); it != medias.end(); it++)
      {
	if (media_mgr.isAttached(it->second))
	{
	  media_mgr.release(it->second, false);
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

      std::string pathname = url_r.getPathName();
      boost::regex e("^(.*)[0-9]+(/?)$");
      boost::smatch what;
      if(boost::regex_match(pathname, what, e, boost::match_extra))
      {
	std::string base = what[1];
	pathname = base + str::numstring(medianr) + what[2];
	Url url = url_r;
	url.setPathName (pathname);

        DBG << "Url rewrite result: " << url << endl;

	return url;
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
