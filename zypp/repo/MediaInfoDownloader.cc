/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <fstream>
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Function.h"

#include "MediaInfoDownloader.h"
#include "zypp/base/UserRequestException.h"

using namespace std;

namespace zypp
{
namespace repo
{

void downloadMediaInfo( const Pathname &dest_dir,
                        MediaSetAccess &media,
                        const ProgressData::ReceiverFnc & progressrcv )
{
  Fetcher fetcher;
  fetcher.enqueue( OnMediaLocation("/media.1/media") );
  fetcher.start( dest_dir, media, progressrcv );
  // ready, go!
  fetcher.reset();
}

}// ns repo 
} // ns zypp



