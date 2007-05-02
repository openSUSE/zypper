/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/parser/TagParser.h
 *
*/
#ifndef ZYPP_PARSER_TAGPARSER_H
#define ZYPP_PARSER_TAGPARSER_H

#include <iosfwd>
#include <string>
#include <list>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/InputStream.h"

#include "zypp/parser/tagfile/ParseException.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace parser
  { /////////////////////////////////////////////////////////////////

    using tagfile::ParseException;

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : TagParser
    //
    /** Basic SUSEtags parser.
     * Will replace parser/tagfile/ and  parser/taggedfile/ stuff.
    */
    class TagParser : private base::NonCopyable
    {
    public:

      struct Tag
      {
	Tag( unsigned lineNo_r = 0, std::streamoff tagStart_r = -1 )
	  : lineNo( lineNo_r ), tagStart( tagStart_r ), dataStart( -1 ), dataLength( 0 )
	{}
	/** String <tt>"NAME[.MODIFIER]:"</tt>. */
	std::string asString() const;

	unsigned       lineNo;
	std::streamoff tagStart;
	std::streamoff dataStart;
	std::streamoff dataLength;

	std::string    name;
	std::string    modifier;
      };

      struct SingleTag : Tag
      {
	SingleTag( unsigned lineNo_r = 0, std::streamoff tagStart_r = -1 )
	  : Tag( lineNo_r, tagStart_r )
	{}
	std::string value;
      };

      struct MultiTag : Tag
      {
	MultiTag( unsigned lineNo_r = 0, std::streamoff tagStart_r = -1 )
	  : Tag( lineNo_r, tagStart_r )
	{}
	std::list<std::string> value;
      };

      typedef shared_ptr<SingleTag> SingleTagPtr;
      typedef shared_ptr<MultiTag> MultiTagPtr;

    public:
      /** Default ctor */
      TagParser();
      /** Dtor */
      virtual ~TagParser();
      /** Parse the stream.
       * \throw ParseExcetion on errors. Invoke \ref consume
       * for each tag. \ref consume might throw other exceptions
       * as well.
      */
      virtual void parse( const InputStream & imput_r );

    protected:
      /** Called when start parsing. */
      virtual void beginParse();
      /** Called when a single-tag is found. */
      virtual void consume( const SingleTagPtr & tag_r );
      /** Called when a multi-tag is found. */
      virtual void consume( const MultiTagPtr & tag_r );
      /** Called when the parse is done. */
      virtual void endParse();

    protected:
      /** Prefix exception message with line and tag information. */
      std::string errPrefix( unsigned lineNo_r,
			     const std::string & msg_r = std::string() ) const;
      /** Prefix exception message with line and tag information. */
      std::string errPrefix( const SingleTagPtr & tag_r,
			     const std::string & msg_r = std::string() ) const;
      /** Prefix exception message with line and tag information. */
      std::string errPrefix( const MultiTagPtr & tag_r,
			     const std::string & msg_r = std::string() ) const;
      /** Name of the current InputStream. */
      const std::string & inputname() const
      { return _inputname; }

    private:
      std::string _inputname;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates TagParser::Tag Stream output. */
    std::ostream & operator<<( std::ostream & str, const TagParser::Tag & obj );

    /** \relates TagParser::SingleTag Stream output. */
    std::ostream & operator<<( std::ostream & str, const TagParser::SingleTag & obj );

    /** \relates TagParser::MultiTag Stream output. */
    std::ostream & operator<<( std::ostream & str, const TagParser::MultiTag & obj );

    template<class _D>
    inline std::ostream & operator<<( std::ostream & str, const shared_ptr<_D> & obj )
    {
      if ( obj )
	return str << *obj;
      return str << std::string("NULL");
    }

    /////////////////////////////////////////////////////////////////
  } // namespace parser
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PARSER_TAGPARSER_H
