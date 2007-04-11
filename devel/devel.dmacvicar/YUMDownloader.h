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
#include "zypp/parser/xml/Reader.h"

namespace zypp
{
  namespace source
  {
    namespace yum
    {
  
      class YUMDownloader
      {
       public:
        YUMDownloader( const Url &url, const Pathname &path );
        void download( const Pathname &dest_dir );
        bool repomd_Callback( const OnMediaLocation &loc, const std::string &dtype );
       private:
        Url _url;
        Pathname _path;
        Fetcher fetcher;
      };

    } // ns yum
  } // ns source
} // ns zypp

#endif
