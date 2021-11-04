/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_SOURCE_YUM_DOWNLOADER
#define ZYPP_SOURCE_YUM_DOWNLOADER

#include <zypp/Url.h>
#include <zypp/Pathname.h>
#include <zypp/Fetcher.h>
#include <zypp-core/OnMediaLocation>
#include <zypp/MediaSetAccess.h>
#include <zypp/ProgressData.h>
#include <zypp/RepoInfo.h>
#include <zypp/RepoStatus.h>
#include <zypp/repo/Downloader.h>

namespace zypp
{
  namespace repo
  {
    namespace yum
    {
     /**
      * \short Downloader for YUM (rpm-nmd) repositories
      * Encapsulates all the knowledge of which files have
      * to be downloaded to the local disk.
      *
      * \code
      * MediaSetAccess media(url);
      * Downloader yum(path);
      * yum.download( media, "localdir");
      * \endcode
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
        Downloader( const RepoInfo & info_r, const Pathname & deltaDir_r = Pathname() );

        /**
         * \short Download metadata to a local directory
         *
         * \param media Media access to the repository url
         * \param destDir Local destination directory
         * \param progress progress receiver
         */
        void download( MediaSetAccess & media_r,
                       const Pathname & destDir_r,
                       const ProgressData::ReceiverFnc & progress_r = ProgressData::ReceiverFnc() ) override;

        /**
         * \short Status of the remote repository
         */
        RepoStatus status( MediaSetAccess & media_r ) override;

      private:
        class Impl;
        friend class Impl;
        Pathname _deltaDir;
      };

    } // ns yum
  } // ns source
} // ns zypp

#endif
