/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/susetags/ContentFileReader.cc
 *
*/
#include <iostream>
#include <sstream>

#include "zypp/base/LogTools.h"
#include "zypp/base/String.h"
#include "zypp/base/IOStream.h"
#include "zypp/base/UserRequestException.h"
#include "zypp/parser/ParseException.h"

#include "zypp/parser/susetags/ContentFileReader.h"
#include "zypp/parser/susetags/RepoIndex.h"

using std::endl;
#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "parser::susetags"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace susetags
    { /////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : ContentFileReader::Impl
      //
      /** ContentFileReader implementation. */
      struct ContentFileReader::Impl
      {
	public:
	  Impl( const ContentFileReader & parent_r )
	  : _parent( parent_r )
	  {}

	  RepoIndex & repoindex()
	  {
	    if ( !_repoindex )
	      _repoindex = new RepoIndex;
	    return *_repoindex;
	  }

	  bool hasRepoIndex() const
	  { return _repoindex != nullptr; }

	  RepoIndex_Ptr handoutRepoIndex()
	  {
	    RepoIndex_Ptr ret;
	    ret.swap( _repoindex );
	    _repoindex = nullptr;
	    return ret;
	  }

	public:
	  bool setFileCheckSum( std::map<std::string, CheckSum> & map_r, const std::string & value ) const
	  {
	    bool error = false;
	    std::vector<std::string> words;
	    if ( str::split( value, std::back_inserter( words ) ) == 3 )
	    {
	      map_r[words[2]] = CheckSum( words[0], words[1] );
	    }
	    else
	    {
	      error = true;
	    }
	    return error;
	  }

	public:
	  std::string _inputname;

	private:
	  const ContentFileReader & _parent;
	  RepoIndex_Ptr      _repoindex;
      };
      ///////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	CLASS NAME : ContentFileReader
      //
      ///////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : ContentFileReader::ContentFileReader
      //	METHOD TYPE : Ctor
      //
      ContentFileReader::ContentFileReader()
      {}

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : ContentFileReader::~ContentFileReader
      //	METHOD TYPE : Dtor
      //
      ContentFileReader::~ContentFileReader()
      {}

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : ContentFileReader::beginParse
      //	METHOD TYPE : void
      //
      void ContentFileReader::beginParse()
      {
	_pimpl.reset( new Impl(*this) );
        // actually mandatory, but in case they were forgotten...
        _pimpl->repoindex().descrdir = "suse/setup/descr";
        _pimpl->repoindex().datadir = "suse";
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : ContentFileReader::endParse
      //	METHOD TYPE : void
      //
      void ContentFileReader::endParse()
      {
	// consume oldData
	if ( _pimpl->hasRepoIndex() )
	{
	  if ( _repoIndexConsumer )
	    _repoIndexConsumer( _pimpl->handoutRepoIndex() );
	}

	MIL << "[Content]" << endl;
	_pimpl.reset();
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : ContentFileReader::userRequestedAbort
      //	METHOD TYPE : void
      //
      void ContentFileReader::userRequestedAbort( unsigned lineNo_r )
      {
	ZYPP_THROW( AbortRequestException( errPrefix( lineNo_r ) ) );
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : ContentFileReader::errPrefix
      //	METHOD TYPE : std::string
      //
      std::string ContentFileReader::errPrefix( unsigned lineNo_r,
	                                        const std::string & msg_r,
						const std::string & line_r ) const
      {
	return str::form( "%s:%u:%s | %s",
			  _pimpl->_inputname.c_str(),
			  lineNo_r,
			  line_r.c_str(),
			  msg_r.c_str() );
      }

      ///////////////////////////////////////////////////////////////////
      //
      //	METHOD NAME : ContentFileReader::parse
      //	METHOD TYPE : void
      //
      void ContentFileReader::parse( const InputStream & input_r,
				     const ProgressData::ReceiverFnc & fnc_r )
      {
	MIL << "Start parsing content repoindex" << input_r << endl;
	if ( ! input_r.stream() )
	{
	  std::ostringstream s;
	  s << "Can't read bad stream: " << input_r;
	  ZYPP_THROW( ParseException( s.str() ) );
	}
	beginParse();
	_pimpl->_inputname = input_r.name();

	ProgressData ticks( makeProgressData( input_r ) );
	ticks.sendTo( fnc_r );
	if ( ! ticks.toMin() )
	  userRequestedAbort( 0 );

	iostr::EachLine line( input_r );
	for( ; line; line.next() )
	{
	  // strip 1st word from line to separate tag and value.
	  std::string value( *line );
	  std::string key( str::stripFirstWord( value, /*ltrim_first*/true ) );

	  if ( key.empty() || *key.c_str() == '#' ) // empty or comment line
	  {
	    continue;
	  }

	  // strip modifier if exists
	  std::string modifier;
	  std::string::size_type pos = key.rfind( '.' );
	  if ( pos != std::string::npos )
	  {
	    modifier = key.substr( pos+1 );
	    key.erase( pos );
	  }

	  //
	  // ReppoIndex related data:
	  //
	  else if ( key == "DESCRDIR" )
	  {
	    _pimpl->repoindex().descrdir = value;
	  }
	  else if ( key == "DATADIR" )
	  {
	    _pimpl->repoindex().datadir = value;
	  }
	  else if ( key == "KEY" )
	  {
	    if ( _pimpl->setFileCheckSum( _pimpl->repoindex().signingKeys, value ) )
	    {
	      ZYPP_THROW( ParseException( errPrefix( line.lineNo(), "Expected [KEY algorithm checksum filename]", *line ) ) );
	    }
	  }
	  else if ( key == "META" )
	  {
	    if ( _pimpl->setFileCheckSum( _pimpl->repoindex().metaFileChecksums, value ) )
	    {
	      ZYPP_THROW( ParseException( errPrefix( line.lineNo(), "Expected [algorithm checksum filename]", *line ) ) );
	    }
	  }
          else if ( key == "HASH" )
	  {
	    if ( _pimpl->setFileCheckSum( _pimpl->repoindex().mediaFileChecksums, value ) )
	    {
	      ZYPP_THROW( ParseException( errPrefix( line.lineNo(), "Expected [algorithm checksum filename]", *line ) ) );
	    }
	  }
          else
	  {
            DBG << errPrefix( line.lineNo(), "ignored", *line ) << endl;
          }


	  if ( ! ticks.set( input_r.stream().tellg() ) )
	    userRequestedAbort( line.lineNo() );
	}

	//
	// post processing
	//
	if ( ! ticks.toMax() )
	  userRequestedAbort( line.lineNo() );

	endParse();
	MIL << "Done parsing " << input_r << endl;
      }

      /////////////////////////////////////////////////////////////////
    } // namespace susetags
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
