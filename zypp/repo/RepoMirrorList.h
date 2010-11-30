/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_REPO_MIRRORLIST_H_
#define ZYPP_REPO_MIRRORLIST_H_

#include <vector>
#include "zypp/Url.h"
#include "zypp/Pathname.h"

namespace zypp
{
  namespace repo
  {
    class RepoMirrorList
    {
      public:
        RepoMirrorList( const Url &url );
        RepoMirrorList( const Url &url, const Pathname &metadatapath );
        virtual ~RepoMirrorList();
        
        std::vector<Url> getUrls() const;

      private:
        std::vector<Url> urls;
        void setUrls( std::vector<Url> my_urls );
        std::vector<Url> parseXML( const Pathname &tmpfile ) const;
        std::vector<Url> parseTXT( const Pathname &tmpfile ) const;
    };

  } // ns repo
} // ns zypp

#endif

// vim: set ts=2 sts=2 sw=2 et ai:
