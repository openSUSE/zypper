/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repo/RepoMirrorList.cc
 *
*/

#include <iostream>
#include <vector>
#include <time.h>
#include "zypp/repo/RepoMirrorList.h"
#include "zypp/media/MetaLinkParser.h"
#include "zypp/MediaSetAccess.h"
#include "zypp/base/LogTools.h"
#include "zypp/ZConfig.h"
#include "zypp/PathInfo.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////

    RepoMirrorList::RepoMirrorList( const Url &url, const Pathname &metadatapath )
    {
      std::vector<Url> my_urls;
      Pathname tmpfile, cachefile;

      if ( url.asString().find("/metalink") != string::npos )
        cachefile = metadatapath / "mirrorlist.xml";
      else
        cachefile = metadatapath / "mirrorlist.txt";
        //cachefile = ZConfig::instance().repoMetadataPath() / Pathname(escaped_alias) / "mirrorlist.txt";

      zypp::filesystem::PathInfo cacheinfo (cachefile);

      if ( !cacheinfo.isFile() || cacheinfo.mtime() < time(NULL) - (long) ZConfig::instance().repo_refresh_delay() * 60 )
      {
        Pathname filepath (url.getPathName());
        Url abs_url (url);

        DBG << "Getting MirrorList from URL: " << abs_url << endl;

        abs_url.setPathName("");
        abs_url.setQueryParam("mediahandler", "curl");

        MediaSetAccess access (abs_url);
        tmpfile = access.provideFile(filepath);

        // Create directory, if not existing
        zypp::filesystem::assert_dir(metadatapath);

        DBG << "Copy MirrorList file to " << cachefile << endl;
        zypp::filesystem::copy(tmpfile, cachefile);
      }

      if ( url.asString().find("/metalink") != string::npos )
      {
        my_urls = parseXML(cachefile);
      }
      else
      {
        my_urls = parseTXT(cachefile);
      }

      setUrls( my_urls );
      if( urls.empty() )
      {
        DBG << "Removing Cachefile as it contains no URLs" << endl;
        zypp::filesystem::unlink(cachefile);
      }
    }

    RepoMirrorList::RepoMirrorList( const Url &url )
    {
      std::vector<Url> my_urls;
      Pathname tmpfile;

      Pathname filepath (url.getPathName());
      Url abs_url (url);

      DBG << "Getting MirrorList from URL: " << abs_url << endl;

      abs_url.setPathName("");
      abs_url.setQueryParam("mediahandler", "curl");

      MediaSetAccess access (abs_url);
      tmpfile = access.provideFile(filepath);

      if ( url.asString().find("/metalink") != string::npos )
      {
        my_urls = parseXML(tmpfile);
      }
      else
      {
        my_urls = parseTXT(tmpfile);
      }

      setUrls( my_urls );
    }

    void RepoMirrorList::setUrls( std::vector<Url> my_urls )
    {
      int valid_urls = 0;
      for (std::vector<Url>::iterator it = my_urls.begin() ; it != my_urls.end() and valid_urls < 4 ; ++it)
      {
        if ( it->getScheme() != "rsync" )
        {
          size_t delpos = it->getPathName().find("repodata/repomd.xml");
          if( delpos != string::npos )
          {
            it->setPathName( it->getPathName().erase(delpos)  );
          }
          urls.push_back(*it);
          ++valid_urls;
        }
      }
    }

    std::vector<Url> RepoMirrorList::parseXML( const Pathname &tmpfile ) const
    {
      InputStream tmpfstream (tmpfile);
      media::MetaLinkParser metalink;
      metalink.parse(tmpfstream);
      return metalink.getUrls();
    }

    std::vector<Url> RepoMirrorList::parseTXT( const Pathname &tmpfile ) const
    {
      InputStream tmpfstream (tmpfile);
      std::vector<Url> my_urls;
      string tmpurl;
      while (getline(tmpfstream.stream(), tmpurl))
      {
        my_urls.push_back(Url(tmpurl));
      }
      return my_urls;
    }
    
    std::vector<Url> RepoMirrorList::getUrls() const
    {
      return urls;
    }

    RepoMirrorList::~RepoMirrorList()
    {}

   /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
