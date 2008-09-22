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

#include "zypp/Date.h"

#include "Downloader.h"
#include "zypp/repo/MediaInfoDownloader.h"
#include "zypp/base/UserRequestException.h"

using namespace std;

namespace zypp
{
namespace repo
{

Downloader::Downloader()
{
}
Downloader::Downloader(const RepoInfo & repoinfo) : _repoinfo(repoinfo)
{  
}
Downloader::~Downloader()
{
}

RepoStatus Downloader::status( MediaSetAccess &media )
{
  WAR << "Non implemented" << endl;
  return RepoStatus();
}

void Downloader::download( MediaSetAccess &media,
                           const Pathname &dest_dir,
                           const ProgressData::ReceiverFnc & progress )
{
  WAR << "Non implemented" << endl;
}

}// ns repo
} // ns zypp



