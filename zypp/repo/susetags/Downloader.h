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
#include "zypp/RepoInfo.h"
#include "zypp/RepoStatus.h"
#include "zypp/MediaSetAccess.h"
#include "zypp/repo/Downloader.h"
#include "zypp/parser/susetags/ContentFileReader.h"

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
         * \short Constructor from the repository information
         *
         * The repository information allows more context to be given
         * to the user when something fails.
         *
         * \param info Repository information
         */
        Downloader( const RepoInfo &info, const Pathname &delta_dir = Pathname() );

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
        
        /**
         * Content file parser consumer
         */
        void consumeIndex( const parser::susetags::RepoIndex_Ptr & data_r );

      private:
        parser::susetags::RepoIndex_Ptr _repoindex;
	Pathname _delta_dir;
      };

    } // ns susetags
  } // ns source
} // ns zypp

#endif
