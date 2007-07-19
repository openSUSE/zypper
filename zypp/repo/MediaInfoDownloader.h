/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_REPO_MEDIAINFO_DOWNLOADER
#define ZYPP_REPO_MEDIAINFO_DOWNLOADER

#include "zypp/Url.h"
#include "zypp/Pathname.h"
#include "zypp/Fetcher.h"
#include "zypp/OnMediaLocation.h"
#include "zypp/MediaSetAccess.h"
#include "zypp/ProgressData.h"

namespace zypp
{
  namespace repo
  {
   
    /**
     * \short Downloads the media info (/media.1) to a local directory
     * \param dest_dir Destination directory
     * \param media \ref MediaSetAccess object to some media
     * \param progress Progress callback function
     *
     * \throws Exception on error
     */
    void downloadMediaInfo( const Pathname &dest_dir,
                            MediaSetAccess &media,
                            const ProgressData::ReceiverFnc & progress = ProgressData::ReceiverFnc() );
  } // ns repo
} // ns zypp

#endif
