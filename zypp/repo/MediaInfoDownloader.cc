/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <fstream>
#include <zypp/base/String.h>
#include <zypp/base/Logger.h>
#include <zypp/base/Function.h>

#include "MediaInfoDownloader.h"
#include <zypp/base/UserRequestException.h>


namespace zypp
{
namespace repo
{

void downloadMediaInfo( const Pathname &dest_dir,
                        MediaSetAccess &media,
                        const ProgressData::ReceiverFnc & progressrcv )
{
  Fetcher fetcher;

  //hardcode the max filesize to 20MB, to prevent unlimited data downloads but this limit will
  //never be reached in a sane setup
  fetcher.enqueue( OnMediaLocation("/media.1/media").setOptional(true).setDownloadSize( ByteCount(20, ByteCount::MB ) ) );
  fetcher.start( dest_dir, media, progressrcv );
  // ready, go!
  fetcher.reset();
}

}// ns repo
} // ns zypp



