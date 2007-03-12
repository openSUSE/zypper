#include <sys/time.h>

#include <iostream>
#include <fstream>

#include <list>
#include <set>

#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>
#include <zypp/media/MediaAccess.h>
#include <zypp/media/MediaManager.h>
#include <zypp/MediaSetAccess.h>
#include <zypp/source/SUSEMediaVerifier.h>
#include <zypp/nMediaLocation.h>
#include <zypp/Fetcher.h>

#include "zypp/Product.h"
#include "zypp/Package.h"


using namespace std;
using namespace zypp;
using namespace media;
//using namespace source;

class SUSETagsDownloader
{
  Url _url;
  Pathname _path;
  public:
  SUSETagsDownloader( const Url &url, const Pathname &path )
     : _url(url), _path(path)
  {
    
  }

  void download( const Pathname &dest_dir )
  {
    Fetcher fetcher(_url, _path);
    fetcher.enqueue( OnMediaLocation().filename("/content") );
    fetcher.start( dest_dir );
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
    fetcher.start( dest_dir );
  }
};


int main(int argc, char **argv)
{
    try
    {
      ZYpp::Ptr z = getZYpp();
      SUSETagsDownloader downloader(Url("dir:/home/duncan/suse/repo/stable-x86"), "/");
      downloader.download("/tmp/repo-cache");
    }
    catch ( const Exception &e )
    {
      cout << "ups! " << e.msg() << std::endl;
    }
    return 0;
}



