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
#include "zypp/parser/yum/PatchesFileReader.h"


using namespace std;
using namespace zypp::xml;

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  PatchesFileReader::PatchesFileReader( const Pathname &repomd_file, ProcessResource callback )
      : _tag(tag_NONE), _callback(callback)
  {
    Reader reader( repomd_file );
    MIL << "Reading " << repomd_file << endl;
    reader.foreachNode( bind( &PatchesFileReader::consumeNode, this, _1 ) );
  }
    
  bool PatchesFileReader::consumeNode( Reader & reader_r )
  {
    //MIL << reader_r->name() << endl;
    std::string data_type;
    if ( reader_r->nodeType() == XML_READER_TYPE_ELEMENT )
    {
      if ( reader_r->name() == "patches" )
      {
        _tag = tag_Patches;
        return true;
      }
      if ( reader_r->name() == "patch" )
      {
        _tag = tag_Patch;
        _id = reader_r->getAttribute("id").asString();
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
      if ( reader_r->name() == "patch" )
        _callback( _location, _id );
      return true;
    }
    return true;
  }


    } // ns yum
  } // ns parser
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
