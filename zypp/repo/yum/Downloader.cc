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
#include "zypp/base/UserRequestException.h"

using namespace std;
using namespace zypp::xml;
using namespace zypp::parser::yum;

namespace zypp
{
namespace repo
{
namespace yum
{

Downloader::Downloader( const Url &url, const Pathname &path )
  : _url(url), _path(path), _media(url, path)
{
}

bool Downloader::patches_Callback( const OnMediaLocation &loc, const string &id )
{
  MIL << id << " : " << loc << endl;
  _fetcher.enqueueDigested(loc);
  return true;
}


bool Downloader::repomd_Callback( const OnMediaLocation &loc, const ResourceType &dtype )
{
  MIL << dtype << " : " << loc << endl;
  
  // skip other
  if ( dtype == ResourceType::OTHER )
  {
    MIL << "Skipping other.xml" << endl;
    return true;
  }
  
  _fetcher.enqueueDigested(loc);
  
  // We got a patches file we need to read, to add patches listed
  // there, so we transfer what we have in the queue, and 
  // queue the patches in the patches callback
  if ( dtype == ResourceType::PATCHES )
  {
    _fetcher.start( _dest_dir, _media);
    // now the patches.xml file must exists
    PatchesFileReader( _dest_dir + loc.filename(), bind( &Downloader::patches_Callback, this, _1, _2));
  }
    
  return true;
}

void Downloader::download( const Pathname &dest_dir,
                           const ProgressData::ReceiverFnc & progressrcv )
{
  Pathname repomdpath =  "/repodata/repomd.xml";
  Pathname keypath =  "/repodata/repomd.xml.key";
  Pathname sigpath =  "/repodata/repomd.xml.asc";
  
  ProgressData progress;
  progress.sendTo(progressrcv);
  progress.toMin();

  _dest_dir = dest_dir;
  if ( _media.doesFileExist(keypath) )
    _fetcher.enqueue( OnMediaLocation().filename(keypath) );

  if ( _media.doesFileExist(sigpath) )
     _fetcher.enqueue( OnMediaLocation().filename(sigpath) );
  
  _fetcher.start( dest_dir, _media );
  
  if ( ! progress.tick() )
    ZYPP_THROW(AbortRequestException());

  SignatureFileChecker sigchecker;
  
  if ( PathInfo( dest_dir + sigpath ).isExist() )
    sigchecker = SignatureFileChecker(dest_dir + sigpath);
  
  if ( PathInfo( dest_dir + keypath ).isExist() )
    sigchecker.addPublicKey(dest_dir + keypath );
  
  _fetcher.enqueue( OnMediaLocation().filename(repomdpath), sigchecker );
  _fetcher.start( dest_dir, _media);
  
  if ( ! progress.tick() )
        ZYPP_THROW(AbortRequestException());

  _fetcher.reset();

  Reader reader( dest_dir + "/repodata/repomd.xml" );
  RepomdFileReader( dest_dir + "/repodata/repomd.xml", bind( &Downloader::repomd_Callback, this, _1, _2));

  // ready, go!
  _fetcher.start( dest_dir, _media);
  progress.toMax();
}

}// ns yum
}// ns source 
} // ns zypp



