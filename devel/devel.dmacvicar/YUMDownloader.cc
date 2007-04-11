
#include <fstream>
#include "zypp/base/String.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Function.h"

#include "zypp/Date.h"

#include "YUMDownloader.h"

using namespace std;
using namespace zypp::xml;

namespace zypp
{
namespace source
{
namespace yum
{

class RepomdFileReader
{
public:
  typedef function<bool( const OnMediaLocation &, const string & )> ProcessResource;
  
  enum Tag
  {
    tag_NONE,
    tag_Repomd,
    tag_Data,
    tag_Location,
    tag_CheckSum,
    tag_Timestamp,
    tag_OpenCheckSum
  };
  
  RepomdFileReader( const Pathname &repomd_file, ProcessResource callback )
    : _tag(tag_NONE), _callback(callback)
  {
    Reader reader( repomd_file );
    MIL << "Reading " << repomd_file << endl;
    reader.foreachNode( bind( &RepomdFileReader::consumeNode, this, _1 ) );
  }
  
  bool consumeNode( Reader & reader_r )
  {
    //MIL << reader_r->name() << endl;
    std::string data_type;
    if ( reader_r->nodeType() == XML_READER_TYPE_ELEMENT )
    {
      if ( reader_r->name() == "repomd" )
      {
        _tag = tag_Repomd;
        return true;
      }
      if ( reader_r->name() == "data" )
      {
        _tag = tag_Data;
        _type = reader_r->getAttribute("type").asString();
        return true;
      }
      if ( reader_r->name() == "location" )
      {
        _tag = tag_Location;
        _location.filename( reader_r->getAttribute("href").asString() );
        return true;
      }
      if ( reader_r->name() == "checksum" )
      {
        _tag = tag_CheckSum;
        string checksum_type = reader_r->getAttribute("type").asString() ;
        string checksum_vaue = reader_r.nodeText().asString();
        _location.checksum( CheckSum( checksum_type, checksum_vaue ) );
        return true;
      }
      if ( reader_r->name() == "timestamp" )
      {
        // ignore it
        return true;
      }
    }
    else if ( reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT )
    {
      //MIL << "end element" << endl;
      if ( reader_r->name() == "data" )
        _callback( _location, _type );
      return true;
    }
    return true;
  }
  
  private:
    OnMediaLocation _location;
    Tag _tag;
    std::string _type;
    ProcessResource _callback;
    CheckSum _checksum;
    std::string _checksum_type;
    Date _timestamp;
};
 
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
