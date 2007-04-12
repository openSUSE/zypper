
#include <fstream>
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Function.h"

#include "zypp/Date.h"


#include "zypp/parser/yum/RepomdFileReader.h"
#include "YUMDownloader.h"

using namespace std;
using namespace zypp::xml;

namespace zypp
{
namespace source
{
namespace yum
{

YUMDownloader::YUMDownloader( const Url &url, const Pathname &path )
  : _url(url), _path(path)
{
}

bool YUMDownloader::repomd_Callback( const OnMediaLocation &loc, const string &dtype )
{
  MIL << loc << " " << dtype << endl;
  fetcher.enqueue(loc);
  return true;
}

void YUMDownloader::download( const Pathname &dest_dir )
{
  MediaSetAccess media(_url, _path);
  fetcher.enqueue( OnMediaLocation().filename("/repodata/repomd.xml") );
  fetcher.start( dest_dir, media);
  fetcher.reset();

  //std::ifstream file((dest_dir + "/content").asString().c_str());
  //std::string buffer;
  //Pathname descr_dir;
  
  Reader reader( dest_dir + "/repodata/repomd.xml" );
  RepomdFileReader( dest_dir + "/repodata/repomd.xml", bind( &YUMDownloader::repomd_Callback, this, _1, _2));
  fetcher.start( dest_dir, media);
  //reader.foreachNode( bind( &YUMDownloader::consumeNode, this, _1 ) );
  // Note this code assumes DESCR comes before as META

//   while (file && !file.eof())
//   {
//     getline(file, buffer);
//     if ( buffer.substr( 0, 5 ) == "DESCR" )
//     {
//       std::vector<std::string> words;
//       if ( str::split( buffer, std::back_inserter(words) ) != 2 )
//       {
//         // error
//         ZYPP_THROW(Exception("bad DESCR line")); 
//       }
//       descr_dir = words[1];
//     }
//     else if ( buffer.substr( 0, 4 ) == "META" )
//     {
//       std::vector<std::string> words;
//       if ( str::split( buffer, std::back_inserter(words) ) != 4 )
//       {
//         // error
//         ZYPP_THROW(Exception("bad DESCR line"));
//       }
//       OnMediaLocation location;
//       location.filename( descr_dir + words[3]).checksum( CheckSum( words[1], words[2] ) );
//       fetcher.enqueue(location);
//     }
//   }
//   file.close();
//   fetcher.start( dest_dir );
}

}// ns yum
}// ns source 
} // ns zypp
