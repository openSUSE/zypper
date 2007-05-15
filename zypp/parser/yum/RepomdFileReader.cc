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
#include "zypp/parser/yum/RepomdFileReader.h"


using namespace std;
using namespace zypp::xml;
using zypp::source::yum::YUMResourceType;

namespace zypp { namespace parser { namespace yum {

RepomdFileReader::RepomdFileReader( const Pathname &repomd_file, ProcessResource callback )
    : _tag(tag_NONE), _type(YUMResourceType::NONE_e), _callback(callback) 
{
  Reader reader( repomd_file );
  MIL << "Reading " << repomd_file << endl;
  reader.foreachNode( bind( &RepomdFileReader::consumeNode, this, _1 ) );
}
  
bool RepomdFileReader::consumeNode( Reader & reader_r )
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
      _type = YUMResourceType(reader_r->getAttribute("type").asString());
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

} } } //ns zypp::source::yum

