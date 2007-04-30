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

#include "zypp/Url.h"
#include "zypp/Pathname.h"
#include "zypp/Fetcher.h"
#include "zypp/OnMediaLocation.h"
#include "zypp/MediaSetAccess.h"
#include "zypp/parser/xml/Reader.h"
#include "zypp/source/yum/YUMResourceType.h"

namespace zypp
{
  namespace source
  {
    namespace yum
    {
      /**
      * This class allows to retrieve a YUM repository
      * to a local directory
      *
      * \code
      * YUMDownloader yum(url, path);
      * yum.download("localdir");
      * \endcode
      */
      class YUMDownloader
      {
       public:
       /**
        * Create the download object for a repository
        * located in \a url with path \a path
        */
        YUMDownloader( const Url &url, const Pathname &path );
       /**
        * starts the download to local directory \a dest_dir
        */
        void download( const Pathname &dest_dir );
        
       protected:
        bool repomd_Callback( const OnMediaLocation &loc, const YUMResourceType &dtype );
        bool patches_Callback( const OnMediaLocation &loc, const std::string &id );
       private:
        Url _url;
        Pathname _path;
        Fetcher _fetcher;
        Pathname _dest_dir;
        std::list<OnMediaLocation> _patches_files;
        MediaSetAccess _media;
      };

    } // ns yum
  } // ns source
} // ns zypp

#endif
