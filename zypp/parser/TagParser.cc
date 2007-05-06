/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/TagParser.cc
 *
*/
#include <iostream>
#include <sstream>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/IOStream.h"

#include "zypp/parser/TagParser.h"
#include "zypp/ProgressData.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : TagParser::Tag
    //
    ///////////////////////////////////////////////////////////////////

    std::string TagParser::Tag::asString() const
    {
      std::string ret( name );
      if ( ! modifier.empty() )
	ret += modifier;
      return ret += ":";
    }

    std::ostream & operator<<( std::ostream & str, const TagParser::Tag & obj )
    {
      str << "@" << obj.lineNo << "{" << obj.name;
      if ( ! obj.modifier.empty() )
	str << '.' << obj.modifier;
      return str << ":}(" << obj.dataStart << "|" << obj.dataLength << ")";
    }

    std::ostream & operator<<( std::ostream & str, const TagParser::SingleTag & obj )
    {
      str << "=" << static_cast<const TagParser::Tag &>( obj );
      return str << "\"" << obj.value << "\"";
    }

    std::ostream & operator<<( std::ostream & str, const TagParser::MultiTag & obj )
    {
      str << "+" << static_cast<const TagParser::Tag &>( obj );
      return str << "[" << obj.value.size() << "]";
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : TagParser
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : TagParser::TagParser
    //	METHOD TYPE : Ctor
    //
    TagParser::TagParser()
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : TagParser::~TagParser
    //	METHOD TYPE : Dtor
    //
    TagParser::~TagParser()
    {}

    void TagParser::beginParse()
    {}
    void TagParser::consume( const SingleTagPtr & tag_r )
    {}
    void TagParser::consume( const MultiTagPtr & tag_r )
    {}
    void TagParser::endParse()
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : TagParser::errPrefix
    //	METHOD TYPE : std::string
    //
    std::string TagParser::errPrefix( unsigned lineNo_r,
				      const std::string & msg_r ) const
    {
      return str::form( "%s:%u:%s: %s",
			_inputname.c_str(),
			lineNo_r,
			"-",
			msg_r.c_str() );
    }

    std::string TagParser::errPrefix( const SingleTagPtr & tag_r,
				      const std::string & msg_r ) const
    {
      return str::form( "%s:%u:=%s %s",
			_inputname.c_str(),
			tag_r->lineNo,
			tag_r->asString().c_str(),
			msg_r.c_str() );
    }

    std::string TagParser::errPrefix( const MultiTagPtr & tag_r,
				      const std::string & msg_r ) const
    {
      return str::form( "%s:%u:+%s %s",
			_inputname.c_str(),
			tag_r->lineNo,
			tag_r->asString().c_str(),
			msg_r.c_str() );
    }

    ///////////////////////////////////////////////////////////////////
    namespace
    { /////////////////////////////////////////////////////////////////

      /** Parse a tag <tt>NAME[.EXT]:</tt>from \a begin_r.
       * If a tag was found, update \a tag_r and advance
       * \a begin_r to point behind the \c :.
       */
      inline bool helperParseStartTag( TagParser::Tag & tag_r, const char *& begin_r )
      {
	const char * tsep = 0;  // find ':'
	const char * esep = 0;  // remember last '.'
	for ( const char * ch = begin_r; *ch; ++ch )
	{
	  switch ( *ch )
	  {
	    case '.':
	      esep = ch; // remember
	      break;
	    case ':':
	      tsep = ch;
	      ch = 0;    // done: found ':'
	      break;
	    case ' ':
	    case '\t':
	    case '\n':
	    case '\r':
	      ch = 0; // fail: no whitespace allowed in tag
	      break;
	  }
	  if ( ! ch )
	    break;
	}

	if ( ! tsep )
	  return false; // no tag found

	// Update name and modifier
	if ( esep )
	{
	  std::string( begin_r, esep-begin_r ).swap( tag_r.name );
	  ++esep;
	  std::string( esep, tsep-esep ).swap( tag_r.modifier );
	}
	else
	{
	  std::string( begin_r, tsep-begin_r ).swap( tag_r.name );
	}

	begin_r = tsep+1; // behind ':'
	return true;
      }

      /////////////////////////////////////////////////////////////////
    } // namespace
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : TagParser::parse
    //	METHOD TYPE : void
    //
    void TagParser::parse( const InputStream & input_r )
    {
      MIL << "Start parsing " << input_r << endl;
      _inputname = input_r.name();
      beginParse();

      ProgressData ticks( makeProgressData( input_r ) );
      ticks.toMin();

      iostr::EachLine line( input_r );
      for( ; line; line.next() )
      {
	const char * cp = (*line).c_str();
	switch ( *cp )
	{
	  ///////////////////////////////////////////////////////////////////
	  case '=': // get single line data
	  {
	    SingleTagPtr tagP( new SingleTag( line.lineNo(), line.lineStart() ) );
	    SingleTag & tag( *tagP.get() );

	    const char * cp = (*line).c_str() + 1;
	    if ( helperParseStartTag( tag, cp ) )
	    {
	      while ( *cp == ' ' || *cp == '\t' )
		++cp;

	      tag.dataStart = tag.tagStart + cp - (*line).c_str();

	      if ( *cp ) // not at string end
	      {
		const char * ep = (*line).c_str() + (*line).size();
		do {
		  --ep;
		} while ( *ep == ' ' || *ep == '\t' );
		tag.dataLength = ep+1-cp;
		std::string( cp, tag.dataLength ).swap( tag.value );
	      }

	      consume( tagP );
	    }
	    else
	    {
	      ZYPP_THROW( ParseException( errPrefix( line.lineNo(), "Orphan data: " + (*line) ) ) );
	    }
	  }
	  break;

	  ///////////////////////////////////////////////////////////////////
	  case '+': // get mulit line data
	  {
	    MultiTagPtr tagP( new MultiTag(line.lineNo(), line.lineStart() ) );
	    MultiTag & tag( *tagP.get() );

	    const char * cp = (*line).c_str() + 1;
	    if ( helperParseStartTag( tag, cp ) )
	    {
	      std::string endTag( "-" );
	      endTag += tag.name;
	      if ( ! tag.modifier.empty() )
	      {
		endTag += ".";
		endTag += tag.modifier;
	      }
	      endTag += ":";

	      line.next();
	      tag.dataStart = line.lineStart();

	      for( ; line; line.next() )
	      {
		if ( str::hasPrefix( *line, endTag ) )
		{
		  tag.dataLength = line.lineStart() - tag.dataStart;
		  break;
		}
		else
		{
		  tag.value.push_back( *line );
		}
	      }

	      if ( ! line )
	      {
		ZYPP_THROW( ParseException( errPrefix( tagP, "Reached EOF while looking for end tag") ) );
	      }

	      consume( tagP );
	    }
	    else
	    {
	      ZYPP_THROW( ParseException( errPrefix( line.lineNo(), "Orphan data: " + (*line) ) ) );
	    }
	  }
	  break;

	  ///////////////////////////////////////////////////////////////////
	  default: // empty or comment
	  {
	    for ( const char * cp = (*line).c_str(); *cp; ++cp )
	    {
	      switch( *cp )
	      {
		case ' ':
		case '\t':
		case '\r':
		case '\n':
		  break;

		default:
		  if ( *cp != '#' )
		  {
		    ZYPP_THROW( ParseException( errPrefix( line.lineNo(), "Orphan data: " + (*line) ) ) );
		  }
		  cp = 0;
		  break;
	      }

	      if ( ! cp )
	      {
		break;
	      }
	    }
	  }
	  break;
	}

	ticks.set( input_r.stream().tellg() );
      }

      ticks.toMax();

      endParse();
      _inputname.clear();
      MIL << "Done parsing " << input_r << endl;
    }

    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
