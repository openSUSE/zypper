/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/PatchesFileReader.cc
 * Implementation of patches.xml file reader.
 */
#include <iostream>

#include "zypp/base/String.h"
#include "zypp/base/Logger.h"

#include "zypp/Date.h"
#include "zypp/CheckSum.h"
#include "zypp/OnMediaLocation.h"

#include "zypp/parser/xml/Reader.h"
#include "zypp/parser/yum/PatchesFileReader.h"


using namespace std;
using namespace zypp::xml;

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  enum Tag
  {
    tag_NONE,
    tag_Patches,
    tag_Patch,
    tag_Location,
    tag_CheckSum,
    tag_Timestamp,
    tag_OpenCheckSum
  };


  ///////////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : PatchesFileReader::Impl
  //
  class PatchesFileReader::Impl : private base::NonCopyable
  {
  public:
   /**
    * CTOR
    */
    Impl(const Pathname &patches_file, const ProcessResource & callback);

    /**
    * Callback provided to the XML parser. Don't use it.
    */
    bool consumeNode( Reader & reader_r );

  private:
    OnMediaLocation _location;
    Tag _tag;
    std::string _id;
    ProcessResource _callback;
    CheckSum _checksum;
    std::string _checksum_type;
    Date _timestamp;
  };
  ///////////////////////////////////////////////////////////////////////


  PatchesFileReader::Impl::Impl(const Pathname & patches_file,
                                const ProcessResource & callback)
    : _tag(tag_NONE), _callback(callback)
  {
    Reader reader( patches_file );
    MIL << "Reading " << patches_file << endl;
    reader.foreachNode(bind( &PatchesFileReader::Impl::consumeNode, this, _1 ));
  }

  // --------------------------------------------------------------------------

  bool PatchesFileReader::Impl::consumeNode( Reader & reader_r )
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
        _location.setLocation( reader_r->getAttribute("href").asString(), 1 );
        return true;
      }
      if ( reader_r->name() == "checksum" )
      {
        _tag = tag_CheckSum;
        string checksum_type = reader_r->getAttribute("type").asString() ;
        string checksum_vaue = reader_r.nodeText().asString();
        _location.setChecksum( CheckSum( checksum_type, checksum_vaue ) );
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


  ///////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : PatchesFileReader
  //
  ///////////////////////////////////////////////////////////////////

  PatchesFileReader::PatchesFileReader(const Pathname & patches_file,
                                       const ProcessResource & callback)
    : _pimpl(new Impl(patches_file, callback))
  {}

  PatchesFileReader::~PatchesFileReader()
  {}


    } // ns yum
  } // ns parser
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
