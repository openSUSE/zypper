
#include <fstream>
#include "zypp/base/String.h"
#include "zypp/OnMediaLocation.h"
#include "zypp/MediaSetAccess.h"
#include "zypp/Fetcher.h"
#include "zypp/repo/MediaInfoDownloader.h"

#include "zypp/repo/susetags/Downloader.h"
#include "zypp/base/UserRequestException.h"

using namespace std;

namespace zypp
{
namespace repo
{
namespace susetags
{

Downloader::Downloader(const Pathname &path )
    : _path(path)
{

}

RepoStatus Downloader::status( MediaSetAccess &media )
{
  Pathname content = media.provideFile( _path + "/content");
  return RepoStatus(content);
}

void Downloader::download( MediaSetAccess &media,
                           const Pathname &dest_dir,
                           const ProgressData::ReceiverFnc & progress )
{
  downloadMediaInfo( dest_dir, media );

  SignatureFileChecker sigchecker;

  Pathname sig = _path + "/content.asc";
  if ( media.doesFileExist(sig) )
  {
    this->enqueue( OnMediaLocation( sig, 1 ) );
    this->start( dest_dir, media );
    this->reset();

    sigchecker = SignatureFileChecker( dest_dir + sig );
  }

  Pathname key = _path + "/content.key";
  if ( media.doesFileExist(key) )
  {
    this->enqueue( OnMediaLocation( key, 1 ) );
    this->start( dest_dir, media );
    this->reset();
    sigchecker.addPublicKey(dest_dir + key);
  }


  this->enqueue( OnMediaLocation( _path + "/content", 1 ), sigchecker );
  this->start( dest_dir, media );
  this->reset();

  std::ifstream file((dest_dir +  _path + "/content").asString().c_str());
  std::string buffer;
  Pathname descr_dir;

  // FIXME Note this code assumes DESCR comes before as META
  string value;
  while (file && !file.eof())
  {
    getline(file, buffer);
    if ( buffer.substr( 0, 5 ) == "DESCR" )
    {
      std::vector<std::string> words;
      if ( str::split( buffer, std::back_inserter(words) ) != 2 )
      {
        // error
        ZYPP_THROW(Exception("bad DESCR line"));
      }
      descr_dir = words[1];
    }
    else if ( buffer.substr( 0, 4 ) == "META" )
    {
      std::vector<std::string> words;
      if ( str::split( buffer, std::back_inserter(words) ) != 4 )
      {
        // error
        ZYPP_THROW(Exception("bad META line"));
      }
      OnMediaLocation location( _path + descr_dir + words[3], 1 );
      location.setChecksum( CheckSum( words[1], words[2] ) );
      this->enqueueDigested(location);
    }
    else if (buffer.substr( 0, 3 ) == "KEY")
    {
      std::vector<std::string> words;
      if ( str::split( buffer, std::back_inserter(words) ) != 4 )
      {
        // error
        ZYPP_THROW(Exception("bad KEY line"));
      }
      OnMediaLocation location( _path + words[3], 1 );
      location.setChecksum( CheckSum( words[1], words[2] ) );
      this->enqueueDigested(location);
    }
  }
  file.close();
  this->start( dest_dir, media );
}

}// ns susetags
}// ns source
} // ns zypp
