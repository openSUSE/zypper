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

Downloader::Downloader( const RepoInfo &info )
  : _info(info), _media_ptr(0L)
{
}

Downloader::Downloader(const Pathname &path )
{
    RepoInfo info;
    info.setPath(path);
    _info = info;
}

RepoStatus Downloader::status( MediaSetAccess &media )
{
  Pathname repomd = media.provideFile( _info.path() + "/repodata/repomd.xml");
  return RepoStatus(repomd);
}

static OnMediaLocation
loc_with_path_prefix(const OnMediaLocation & loc,
                     const Pathname & prefix)
{
  if (prefix.empty() || prefix == "/")
    return loc;

  OnMediaLocation loc_with_path(loc);
  loc_with_path.setFilename(prefix / loc.filename());
  return loc_with_path;
}


bool Downloader::patches_Callback( const OnMediaLocation &loc,
                                   const string &id )
{
  OnMediaLocation loc_with_path(loc_with_path_prefix(loc, _info.path()));
  MIL << id << " : " << loc_with_path << endl;
  this->enqueueDigested(loc_with_path);
  return true;
}


bool Downloader::repomd_Callback( const OnMediaLocation &loc,
                                  const ResourceType &dtype )
{
  OnMediaLocation loc_with_path(loc_with_path_prefix(loc, _info.path()));
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

  this->enqueueDigested(loc_with_path);

  // We got a patches file we need to read, to add patches listed
  // there, so we transfer what we have in the queue, and
  // queue the patches in the patches callback
  if ( dtype == ResourceType::PATCHES )
  {
    this->start( _dest_dir, *_media_ptr );
    // now the patches.xml file must exists
    PatchesFileReader( _dest_dir + _info.path() + loc.filename(),
                       bind( &Downloader::patches_Callback, this, _1, _2));
  }

  return true;
}

void Downloader::download( MediaSetAccess &media,
                           const Pathname &dest_dir,
                           const ProgressData::ReceiverFnc & progressrcv )
{
  Pathname repomdpath =  _info.path() + "/repodata/repomd.xml";
  Pathname keypath =  _info.path() + "/repodata/repomd.xml.key";
  Pathname sigpath =  _info.path() + "/repodata/repomd.xml.asc";

  _media_ptr = (&media);

  ProgressData progress;
  progress.sendTo(progressrcv);
  progress.toMin();

  //downloadMediaInfo( dest_dir, _media );

  _dest_dir = dest_dir;

  SignatureFileChecker sigchecker(_info.name());

  if ( _media_ptr->doesFileExist(sigpath) )
  {
      this->enqueue( OnMediaLocation(sigpath,1).setOptional(true) );
     this->start( dest_dir, *_media_ptr);
     this->reset();
     sigchecker = SignatureFileChecker(dest_dir + sigpath, _info.name());
  }


  if ( _media_ptr->doesFileExist(keypath) )
  {
      this->enqueue( OnMediaLocation(keypath,1).setOptional(true) );
    this->start( dest_dir, *_media_ptr);
    this->reset();
    sigchecker.addPublicKey(dest_dir + keypath);
  }


  this->start( dest_dir, *_media_ptr );

  if ( ! progress.tick() )
    ZYPP_THROW(AbortRequestException());

  if ( ! _info.gpgCheck() )
  {
    WAR << "Signature checking disabled in config of repository " << _info.alias() << endl;
  }
  this->enqueue( OnMediaLocation(repomdpath,1),
                 _info.gpgCheck() ? FileChecker(sigchecker) : FileChecker(NullFileChecker()) );
  this->start( dest_dir, *_media_ptr);

  if ( ! progress.tick() )
        ZYPP_THROW(AbortRequestException());

  this->reset();

  Reader reader( dest_dir + _info.path() + "/repodata/repomd.xml" );
  RepomdFileReader( dest_dir + _info.path() + "/repodata/repomd.xml", bind( &Downloader::repomd_Callback, this, _1, _2));

  // ready, go!
  this->start( dest_dir, *_media_ptr);
  progress.toMax();
}

}// ns yum
}// ns source
} // ns zypp



