/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_SOURCE_SUSETAGS_DOWNLOADER
#define ZYPP_SOURCE_SUSETAGS_DOWNLOADER

#include "zypp/Url.h"
#include "zypp/Pathname.h"
#include "zypp/ProgressData.h"
#include "zypp/RepoStatus.h"
#include "zypp/MediaSetAccess.h"
#include "zypp/repo/Downloader.h"

namespace zypp
{
  namespace repo
  {
    namespace susetags
    {
  
      /**
       * \short Downloader for SUSETags (YaST2) repositories
       * Encapsulates all the knowledge of which files have
       * to be downloaded to the local disk.
       */
      class Downloader : public repo::Downloader
      {
       public:
        /**
         * \short Constructor
         *
         * \param path Path to the repostory from the media
         */
        Downloader( const Pathname &path );
        
        /**
         * \short Download metadata to a local directory
         *
         * \param media Media access to the repository url
         * \param dest_dir Local destination directory
         * \param progress progress receiver
         */
        void download( MediaSetAccess &media,
                       const Pathname &dest_dir,
                       const ProgressData::ReceiverFnc & progress = ProgressData::ReceiverFnc() );
        /**
         * \short Status of the remote repository
         */
        RepoStatus status( MediaSetAccess &media );
       private:
        Pathname _path;
      };

    } // ns susetags
  } // ns source
} // ns zypp

#endif
