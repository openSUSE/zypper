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
	RepoMirrorList( const Url & url_r, const Pathname & metadatapath_r, bool mirrorListForceMetalink_r );

        RepoMirrorList( const Url & url_r )
	: RepoMirrorList( url_r, Pathname(), false )
	{}

        const std::vector<Url> & getUrls() const
        { return _urls; }

        std::vector<Url> & getUrls()
        { return _urls; }

      private:
        std::vector<Url> _urls;
    };
  } // ns repo
} // ns zypp

#endif

// vim: set ts=2 sts=2 sw=2 et ai:
