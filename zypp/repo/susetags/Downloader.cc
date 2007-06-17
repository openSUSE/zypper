
#include <fstream>
#include "zypp/base/String.h"
#include "zypp/OnMediaLocation.h"
#include "zypp/MediaSetAccess.h"
#include "zypp/Fetcher.h"

#include "zypp/repo/susetags/Downloader.h"
#include "zypp/base/UserRequestException.h"

using namespace std;

namespace zypp
{
namespace repo
{
namespace susetags
{
  
Downloader::Downloader( const Url &url, const Pathname &path )
    : _url(url), _path(path)
{
  
}

void Downloader::download( const Pathname &dest_dir,
                           const ProgressData::ReceiverFnc & progress )
{
  MediaSetAccess media(_url, _path);
  Fetcher fetcher;
  fetcher.enqueue( OnMediaLocation().filename("/content") );
  fetcher.start( dest_dir, media );
  fetcher.reset();

  std::ifstream file((dest_dir + "/content").asString().c_str());
  std::string buffer;
  Pathname descr_dir;

  // Note this code assumes DESCR comes before as META

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
        ZYPP_THROW(Exception("bad DESCR line"));
      }
      OnMediaLocation location;
      location.filename( descr_dir + words[3]).checksum( CheckSum( words[1], words[2] ) );
      fetcher.enqueue(location);
    }
  }
  file.close();
  fetcher.start( dest_dir, media );
}

}// ns susetags
}// ns source 
} // ns zypp
