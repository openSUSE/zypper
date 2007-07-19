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

Downloader::Downloader( const Pathname &path )
  : _path(path), _media_ptr(0L)
{
}

RepoStatus Downloader::status( MediaSetAccess &media )
{
  Pathname repomd = media.provideFile( _path + "/repodata/repomd.xml");
  return RepoStatus(repomd);
}

bool Downloader::patches_Callback( const OnMediaLocation &loc,
                                   const string &id )
{
  MIL << id << " : " << loc << endl;
  this->enqueueDigested(loc);
  return true;
}


bool Downloader::repomd_Callback( const OnMediaLocation &loc,
                                  const ResourceType &dtype )
{
  MIL << dtype << " : " << loc << endl;

  // skip other
  if ( dtype == ResourceType::OTHER )
  {
    MIL << "Skipping other.xml" << endl;
    return true;
  }

  this->enqueueDigested(loc);

  // We got a patches file we need to read, to add patches listed
  // there, so we transfer what we have in the queue, and
  // queue the patches in the patches callback
  if ( dtype == ResourceType::PATCHES )
  {
    this->start( _dest_dir, *_media_ptr );
    // now the patches.xml file must exists
    PatchesFileReader( _dest_dir + _path + loc.filename(),
                       bind( &Downloader::patches_Callback, this, _1, _2));
  }

  return true;
}

void Downloader::download( MediaSetAccess &media,
                           const Pathname &dest_dir,
                           const ProgressData::ReceiverFnc & progressrcv )
{
  Pathname repomdpath =  _path + "/repodata/repomd.xml";
  Pathname keypath =  _path + "/repodata/repomd.xml.key";
  Pathname sigpath =  _path + "/repodata/repomd.xml.asc";

  _media_ptr = (&media);
  
  ProgressData progress;
  progress.sendTo(progressrcv);
  progress.toMin();

  //downloadMediaInfo( dest_dir, _media );
  
  _dest_dir = dest_dir;
  
  if ( _media_ptr->doesFileExist(keypath) )
    this->enqueue( OnMediaLocation(keypath,1) );

  if ( _media_ptr->doesFileExist(sigpath) )
     this->enqueue( OnMediaLocation(sigpath,1) );

  this->start( dest_dir, *_media_ptr );

  if ( ! progress.tick() )
    ZYPP_THROW(AbortRequestException());

  SignatureFileChecker sigchecker;

  if ( PathInfo( dest_dir + sigpath ).isExist() )
    sigchecker = SignatureFileChecker(dest_dir + sigpath);

  if ( PathInfo( dest_dir + keypath ).isExist() )
    sigchecker.addPublicKey(dest_dir + keypath );

  this->enqueue( OnMediaLocation(repomdpath,1), sigchecker );
  this->start( dest_dir, *_media_ptr);

  if ( ! progress.tick() )
        ZYPP_THROW(AbortRequestException());

  this->reset();

  Reader reader( dest_dir + _path + "/repodata/repomd.xml" );
  RepomdFileReader( dest_dir + _path + "/repodata/repomd.xml", bind( &Downloader::repomd_Callback, this, _1, _2));

  // ready, go!
  this->start( dest_dir, *_media_ptr);
  progress.toMax();
}

}// ns yum
}// ns source
} // ns zypp



