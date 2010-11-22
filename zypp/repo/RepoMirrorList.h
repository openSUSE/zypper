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

namespace zypp
{
  namespace repo
  {
    class RepoMirrorList
    {
      public:
        RepoMirrorList( const Url &url );
        virtual ~RepoMirrorList();
        
        std::vector<Url> getUrls() const;

      private:
        std::vector<Url> urls;
    };

  } // ns repo
} // ns zypp

#endif

// vim: set ts=2 sts=2 sw=2 et ai:
