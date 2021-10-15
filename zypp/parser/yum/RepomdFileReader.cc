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

#include <zypp/base/String.h>
#include <zypp/base/Logger.h>
#include <zypp/base/Regex.h>

#include <zypp/Pathname.h>
#include <zypp/Date.h>
#include <zypp/Url.h>
#include <zypp/CheckSum.h>
#include <zypp/parser/xml/Reader.h>

#include <zypp/parser/yum/RepomdFileReader.h>

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "parser::yum"

using std::endl;
using namespace zypp::xml;

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
    /** Ctro taking a ProcessResource callback */
    Impl(const Pathname &repomd_file, const ProcessResource & callback )
    : _callback( callback )
    {
      Reader reader( repomd_file );
      MIL << "Reading " << repomd_file << endl;
      reader.foreachNode( bind( &RepomdFileReader::Impl::consumeNode, this, _1 ) );
    }

    /**
     * Callback provided to the XML parser.
     */
    bool consumeNode( Reader & reader_r );


    /** repo keywords parsed on the fly */
    const std::set<std::string> & keywords() const
    { return _keywords; }

  private:
    /** Retrieve a checksum node. */
    CheckSum getChecksum( Reader & reader_r )
    { return CheckSum( reader_r->getAttribute("type").asString(), reader_r.nodeText().asString() ); }

    /** Retrieve a size node. */
    ByteCount getSize( Reader & reader_r )
    { return ByteCount( str::strtonum<ByteCount::SizeType>( reader_r.nodeText().asString() ) ); }


  private:
    /** Function for processing collected data. Passed-in through constructor. */
    ProcessResource _callback;

    /** The resource type string. */
    std::string _typeStr;

    /** Location of metadata file. */
    OnMediaLocation _location;

    std::set<std::string> _keywords;	///< repo keywords parsed on the fly
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
        return true;
      }

      // xpath: /repomd/data (+)
      if ( reader_r->name() == "data" )
      {
        _typeStr = reader_r->getAttribute("type").asString();
        return true;
      }

      // xpath: /repomd/location
      if ( reader_r->name() == "location" )
      {
        _location.setLocation( reader_r->getAttribute("href").asString(), 1 );
        // ignoring attribute xml:base
        return true;
      }

      // xpath: /repomd/checksum
      if ( reader_r->name() == "checksum" )
      {
        _location.setChecksum( getChecksum( reader_r ) );
        return true;
      }

      // xpath: /repomd/header-checksum
      if ( reader_r->name() == "header-checksum" )
      {
        _location.setHeaderChecksum( getChecksum( reader_r ) );
        return true;
      }

      // xpath: /repomd/timestamp
      if ( reader_r->name() == "timestamp" )
      {
        // ignore it
        return true;
      }

      // xpath: /repomd/size
      if ( reader_r->name() == "size" )
      {
        _location.setDownloadSize( getSize( reader_r ) );
        return true;
      }

      // xpath: /repomd/header-size
      if ( reader_r->name() == "header-size" )
      {
        _location.setHeaderSize( getSize( reader_r ) );
        return true;
      }

      // xpath: /tags/content
      if ( reader_r->name() == "content" )
      {
        const auto & tag = reader_r.nodeText();
        if ( tag.c_str() && *tag.c_str() )
          _keywords.insert( tag.asString() );	// remember keyword
        return true;
      }
    }

    else if ( reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT )
    {
      // xpath: /repomd/data
      if ( reader_r->name() == "data" )
      {
        if (_callback) {
          _callback( std::move(_location), _typeStr );
          _location = OnMediaLocation();
          _typeStr.clear();
        }
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

  RepomdFileReader::RepomdFileReader( const Pathname & repomd_file )
  : _pimpl( new Impl(repomd_file, ProcessResource()) )
  {}

  RepomdFileReader::~RepomdFileReader()
  {}

  const std::set<std::string> & RepomdFileReader::keywords() const
  { return _pimpl->keywords(); }

  std::vector<std::pair<std::string,std::string>> RepomdFileReader::keyhints() const
  {
    std::vector<std::pair<std::string,std::string>> ret;
    for ( const std::string & tag : keywords() ) {
      // Get keyhints on the fly:
      // gpg-pubkey-39db7c82-5847eb1f.asc?fpr=22C07BA534178CD02EFE22AAB88B2FD43DBDC284
      // Fingerprint is explicitely mentioned or id/fpr can be derived from the filename
      if ( tag.compare( 0,10,"gpg-pubkey" ) != 0 )
        continue;

      static const str::regex rx( "^(gpg-pubkey([^?]*))(\\?fpr=([[:xdigit:]]{8,}))?$" );
      str::smatch what;
      if ( str::regex_match( tag.c_str(), what, rx ) ) {
        std::string keyfile { what[1] };
        std::string keyident;
        if ( what.size(4) != std::string::npos ) {	// with fpr=
          keyident = what[4];
        }
        else {
          static const str::regex rx( /*gpg-pubkey*/"^-([[:xdigit:]]{8,})" );
          if ( str::regex_match( what[2], what, rx ) ) {
            keyident = what[1];
          }
          else {
            DBG << "Tag " << tag << " does not contain a keyident. ignore it." << endl;
            continue;
          }
        }
        ret.push_back( std::make_pair( std::move(keyfile), std::move(keyident) ) );
      }
    }
    return ret;
  }

    } // ns yum
  } // ns parser
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
