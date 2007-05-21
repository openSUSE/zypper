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
#include "YUMDownloader.h"

using namespace std;
using namespace zypp::xml;
using namespace zypp::parser::yum;

namespace zypp
{
namespace source
{
namespace yum
{

YUMDownloader::YUMDownloader( const Url &url, const Pathname &path )
  : _url(url), _path(path), _media(url, path)
{
}

bool YUMDownloader::patches_Callback( const OnMediaLocation &loc, const string &id )
{
  MIL << id << " : " << loc << endl;
  _fetcher.enqueueDigested(loc);
  return true;
}


bool YUMDownloader::repomd_Callback( const OnMediaLocation &loc, const YUMResourceType &dtype )
{
  MIL << dtype << " : " << loc << endl;
  _fetcher.enqueueDigested(loc);
  
  // We got a patches file we need to read, to add patches listed
  // there, so we transfer what we have in the queue, and 
  // queue the patches in the patches callback
  if ( dtype == YUMResourceType::PATCHES )
  {
    _fetcher.start( _dest_dir, _media);
    // now the patches.xml file must exists
    PatchesFileReader( _dest_dir + loc.filename(), bind( &YUMDownloader::patches_Callback, this, _1, _2));
  }
  return true;
}

void YUMDownloader::download( const Pathname &dest_dir )
{
  Pathname repomdpath =  "/repodata/repomd.xml";
  Pathname keypath =  "/repodata/repomd.xml.key";
  Pathname sigpath =  "/repodata/repomd.xml.asc";
  
  
  _dest_dir = dest_dir;
  if ( _media.doesFileExist(keypath) )
    _fetcher.enqueue( OnMediaLocation().filename(keypath) );

  if ( _media.doesFileExist(sigpath) )
     _fetcher.enqueue( OnMediaLocation().filename(sigpath) );
  
  _fetcher.start( dest_dir, _media );
  
  Fetcher::SignatureFileChecker sigchecker;
  
  if ( PathInfo( dest_dir + sigpath ).isExist() )
    sigchecker = Fetcher::SignatureFileChecker(dest_dir + sigpath);
  
  if ( PathInfo( dest_dir + keypath ).isExist() )
    sigchecker.addPublicKey(dest_dir + keypath );
  
  _fetcher.enqueue( OnMediaLocation().filename(repomdpath), sigchecker );
  _fetcher.start( dest_dir, _media);
  
  
  _fetcher.reset();

  Reader reader( dest_dir + "/repodata/repomd.xml" );
  RepomdFileReader( dest_dir + "/repodata/repomd.xml", bind( &YUMDownloader::repomd_Callback, this, _1, _2));

  // ready, go!
  _fetcher.start( dest_dir, _media);
}

}// ns yum
}// ns source 
} // ns zypp



