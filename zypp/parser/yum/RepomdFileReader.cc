/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/RepomdFileReader.cc
 * Implementation of repomd.xml file reader.
 */
#include <iostream>

#include "zypp/base/String.h"
#include "zypp/base/Logger.h"

#include "zypp/Pathname.h"
#include "zypp/Date.h"
#include "zypp/CheckSum.h"
#include "zypp/parser/xml/Reader.h"

#include "zypp/parser/yum/RepomdFileReader.h"

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "parser::yum"

using namespace std;
using namespace zypp::xml;
using zypp::repo::yum::ResourceType;

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  ///////////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : RepomdFileReader::Impl
  //
  class RepomdFileReader::Impl : private base::NonCopyable
  {
  public:

    /**
     * Enumeration of repomd.xml tags.
     * \see _tag
     */
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

  public:
    /** Ctro taking a ProcessResource2 callback */
    Impl(const Pathname &repomd_file, const ProcessResource2 & callback )
    : _tag( tag_NONE )
    , _type( ResourceType::NONE_e )
    , _callback( callback )
    {
      Reader reader( repomd_file );
      MIL << "Reading " << repomd_file << endl;
      reader.foreachNode( bind( &RepomdFileReader::Impl::consumeNode, this, _1 ) );
    }
   /** \overload Redirect an old ProcessResource callback */
    Impl(const Pathname &repomd_file, const ProcessResource & callback)
    : Impl( repomd_file, ProcessResource2( bind( callback, _1, _2 ) ) )
    {}

    /**
     * Callback provided to the XML parser.
     */
    bool consumeNode( Reader & reader_r );


  private:
    /** Function for processing collected data. Passed-in through constructor. */
    ProcessResource2 _callback;

    /** Used to remember currently processed tag */
    Tag _tag;

    /** Type of metadata file (string) */
    std::string _typeStr;

    /** Type of metadata file as enum of well known repoinded.xml entries. */
    repo::yum::ResourceType _type;

    /** Location of metadata file. */
    OnMediaLocation _location;
  };
  ///////////////////////////////////////////////////////////////////////

  /*
   * xpath and multiplicity of processed nodes are included in the code
   * for convenience:
   *
   * // xpath: <xpath> (?|*|+)
   *
   * if multiplicity is ommited, then the node has multiplicity 'one'.
   */

  // --------------------------------------------------------------------------

  bool RepomdFileReader::Impl::consumeNode( Reader & reader_r )
  {
    if ( reader_r->nodeType() == XML_READER_TYPE_ELEMENT )
    {
      // xpath: /repomd
      if ( reader_r->name() == "repomd" )
      {
        _tag = tag_Repomd;
        return true;
      }

      // xpath: /repomd/data (+)
      if ( reader_r->name() == "data" )
      {
        _tag = tag_Data;
	_typeStr = reader_r->getAttribute("type").asString();
        _type = ResourceType(_typeStr);
        return true;
      }

      // xpath: /repomd/location
      if ( reader_r->name() == "location" )
      {
        _tag = tag_Location;
        _location.setLocation( reader_r->getAttribute("href").asString(), 1 );
        // ignoring attribute xml:base
        return true;
      }

      // xpath: /repomd/checksum
      if ( reader_r->name() == "checksum" )
      {
        _tag = tag_CheckSum;
        string checksum_type = reader_r->getAttribute("type").asString() ;
        string checksum_vaue = reader_r.nodeText().asString();
        _location.setChecksum( CheckSum( checksum_type, checksum_vaue ) );
        return true;
      }

      // xpath: /repomd/timestamp
      if ( reader_r->name() == "timestamp" )
      {
        // ignore it
        return true;
      }

      //! \todo xpath: /repomd/open-checksum (?)
    }

    else if ( reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT )
    {
      // xpath: /repomd/data
      if ( reader_r->name() == "data" )
      {
        if (_callback)
          _callback( _location, _type, _typeStr );

        return true;
      }
    }

    return true;
  }


  ///////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : RepomdFileReader
  //
  ///////////////////////////////////////////////////////////////////

  RepomdFileReader::RepomdFileReader( const Pathname & repomd_file, const ProcessResource & callback )
  : _pimpl( new Impl(repomd_file, callback) )
  {}

  RepomdFileReader::RepomdFileReader( const Pathname & repomd_file, const ProcessResource2 & callback )
  : _pimpl( new Impl(repomd_file, callback) )
  {}

  RepomdFileReader::~RepomdFileReader()
  {}


    } // ns yum
  } // ns parser
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
