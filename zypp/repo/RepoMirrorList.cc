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
#include "zypp/repo/RepoMirrorList.h"
#include "zypp/media/MetaLinkParser.h"
#include "zypp/MediaSetAccess.h"
#include "zypp/base/LogTools.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////

    RepoMirrorList::RepoMirrorList( const Url &url )
    {
      Pathname filepath (url.getPathName());
      Url abs_url (url);
      std::vector<Url> my_urls;

      DBG << "Getting MirrorList from URL: " << abs_url << endl;

      abs_url.setPathName("");
      abs_url.setQueryParam("mediahandler", "curl");

      MediaSetAccess access (abs_url);
      Pathname tmpfile = access.provideFile(filepath);

      InputStream tmpfstream (tmpfile);

      if ( url.asString().find("/metalink") != string::npos )
      {
        media::MetaLinkParser metalink;
        metalink.parse(tmpfstream);
        my_urls = metalink.getUrls();
      }
      else
      {
        string tmpurl;
        while (getline(tmpfstream.stream(), tmpurl))
        {
          my_urls.push_back(Url(tmpurl));
        }
      }

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
