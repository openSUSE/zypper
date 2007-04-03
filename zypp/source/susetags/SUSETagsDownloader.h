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

namespace zypp
{
  namespace source
  {
    namespace susetags
    {
  
      class SUSETagsDownloader
      {
       public:
        SUSETagsDownloader( const Url &url, const Pathname &path );
        void download( const Pathname &dest_dir );
       private:
        Url _url;
        Pathname _path;
      };

    } // ns susetags
  } // ns source
} // ns zypp

#endif
