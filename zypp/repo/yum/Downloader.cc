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

#include "zypp/parser/yum/RepomdFileReader.h"
#include "zypp/parser/yum/PatchesFileReader.h"
#include "Downloader.h"
#include "zypp/repo/MediaInfoDownloader.h"
#include "zypp/base/UserRequestException.h"
#include "zypp/parser/xml/Reader.h"

using namespace std;
using namespace zypp::xml;
using namespace zypp::parser::yum;

namespace zypp
{
namespace repo
{
namespace yum
{

Downloader::Downloader( const RepoInfo &repoinfo , const Pathname &delta_dir)
  : repo::Downloader(repoinfo), _delta_dir(delta_dir), _media_ptr(0L)
{
}


RepoStatus Downloader::status( MediaSetAccess &media )
{
  Pathname repomd = media.provideFile( repoInfo().path() + "/repodata/repomd.xml");
  return RepoStatus(repomd);
}

static OnMediaLocation
loc_with_path_prefix(const OnMediaLocation & loc,
                     const Pathname & prefix)
{
  if (prefix.empty() || prefix == "/")
    return loc;

  OnMediaLocation loc_with_path(loc);
  loc_with_path.changeFilename(prefix / loc.filename());
  return loc_with_path;
}

// search old repository file file to run the delta algorithm on
static Pathname search_deltafile( const Pathname &dir, const Pathname &file )
{
  Pathname deltafile;
  if (!PathInfo(dir).isDir())
    return deltafile;
  string base = file.basename();
  size_t hypoff = base.find("-");
  if (hypoff != string::npos)
    base.replace(0, hypoff + 1, "");
  size_t basesize = base.size();
  std::list<Pathname> retlist;
  if (!filesystem::readdir(retlist, dir, false))
  {
    for_( it, retlist.begin(), retlist.end() )
    {
      string fn = it->asString();
      if (fn.size() >= basesize && fn.substr(fn.size() - basesize, basesize) == base)
	deltafile = *it;
    }
  }
  return deltafile;
}

bool Downloader::patches_Callback( const OnMediaLocation &loc,
                                   const string &id )
{
  OnMediaLocation loc_with_path(loc_with_path_prefix(loc, repoInfo().path()));
  MIL << id << " : " << loc_with_path << endl;
  this->enqueueDigested(loc_with_path,  FileChecker(), search_deltafile(_delta_dir + "repodata", loc.filename()));
  return true;
}

bool Downloader::repomd_Callback( const OnMediaLocation &loc,
                                  const ResourceType &dtype )
{
  OnMediaLocation loc_with_path(loc_with_path_prefix(loc, repoInfo().path()));
  MIL << dtype << " : " << loc_with_path << endl;

  //! \todo do this through a ZConfig call so that it is always in sync with parser
  // skip other
  if ( dtype == ResourceType::OTHER )
  {
    MIL << "Skipping other.xml" << endl;
    return true;
  }
  // skip filelists
  if ( dtype == ResourceType::FILELISTS )
  {
    MIL << "Skipping filelists.xml.gz" << endl;
    return true;
  }

  this->enqueueDigested(loc_with_path, FileChecker(), search_deltafile(_delta_dir + "repodata", loc.filename()));

  // We got a patches file we need to read, to add patches listed
  // there, so we transfer what we have in the queue, and
  // queue the patches in the patches callback
  if ( dtype == ResourceType::PATCHES )
  {
    this->start( _dest_dir, *_media_ptr );
    // now the patches.xml file must exists
    PatchesFileReader( _dest_dir + repoInfo().path() + loc.filename(),
                       bind( &Downloader::patches_Callback, this, _1, _2));
  }

  return true;
}

/** \todo: Downloading/sigcheck of master index shoudl be common in base class */
void Downloader::download( MediaSetAccess &media,
                           const Pathname &dest_dir,
                           const ProgressData::ReceiverFnc & progressrcv )
{
  Pathname masterIndex( repoInfo().path() / "/repodata/repomd.xml" );
  defaultDownloadMasterIndex( media, dest_dir, masterIndex );

  _media_ptr = (&media);
  _dest_dir = dest_dir;
  RepomdFileReader( dest_dir / masterIndex, bind( &Downloader::repomd_Callback, this, _1, _2));

  // ready, go!
  start( dest_dir, media );
}

}// ns yum
}// ns source
} // ns zypp



