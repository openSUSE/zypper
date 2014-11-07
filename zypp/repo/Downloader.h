/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_REPO_DOWNLOADER
#define ZYPP_REPO_DOWNLOADER

#include "zypp/Url.h"
#include "zypp/Pathname.h"
#include "zypp/ProgressData.h"
#include "zypp/RepoStatus.h"
#include "zypp/MediaSetAccess.h"
#include "zypp/Fetcher.h"
#include "zypp/RepoInfo.h"

namespace zypp
{
  namespace repo
  {
    /**
      * \short Downloader base class
      *
      * a Downloader encapsulates all the knowledge of
      * which files have to be downloaded to the local disk.
      *
      */
    class Downloader : public Fetcher
    {
      public:
      /**
        * \short Constructor
        */
      Downloader();
      /** C-tor associating the downloader with a RepoInfo */
      Downloader(const RepoInfo & info);
      virtual ~Downloader();

      /**
        * \short Download metadata to a local directory
        *
        * \param media Media access to the repository url
        * \param dest_dir Local destination directory
        * \param progress progress receiver
        */
      virtual void download( MediaSetAccess &media,
                              const Pathname &dest_dir,
                              const ProgressData::ReceiverFnc & progress = ProgressData::ReceiverFnc() );
      /**
        * \short Status of the remote repository
        */
      virtual RepoStatus status( MediaSetAccess &media );

      const RepoInfo & repoInfo() const { return _repoinfo; }

      protected:
	/** Common workflow downloading a (signed) master index file */
	void defaultDownloadMasterIndex( MediaSetAccess & media_r, const Pathname & destdir_r, const Pathname & masterIndex_r );

      private:
        RepoInfo _repoinfo;
    };
  } // ns repo
} // ns zypp

#endif
