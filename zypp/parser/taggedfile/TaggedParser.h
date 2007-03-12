/*---------------------------------------------------------------------\
 |                                                                      |
 |                      __   __    ____ _____ ____                      |
 |                      \ \ / /_ _/ ___|_   _|___ \                     |
 |                       \ V / _` \___ \ | |   __) |                    |
 |                        | | (_| |___) || |  / __/                     |
 |                        |_|\__,_|____/ |_| |_____|                    |
 |                                                                      |
 |                               core system                            |
 |                                                        (C) SuSE GmbH |
 \----------------------------------------------------------------------/

 File:       TaggedParser.h

 Author:     Michael Andres <ma@suse.de>
 Maintainer: Klaus Kaempf <kkaempf@suse.de>

 Parser for tagged file format as used in SuSE/UnitedLinux
 media data

 /-*/
#ifndef TaggedParser_h
#define TaggedParser_h

#include <iosfwd>
#include <string>
#include <list>
#include <vector>
#include <map>

using std::vector;
using std::map;
using std::streamoff;
using std::istream;

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : TaggedParser
//
//
//	DESCRIPTION :
//		parses file in "tagged" format
//		a tag starts at the beginning of a line with
//		'=' (single line tag, tag_type SINGLE),
//		'+' (start of multi-line tag, tag_type START), or
//		'-' (end of multi line tag, tag_type END)
//		followed by an arbitrary string and a colon (':')
//
//		The tag parser 'lookupTag()' searches through an open
//		stream for such a tag
//
//		It also recognizes all other "<tag>:<blank>" lines as
//		tag_type OLDSTYLE. However, since this style requires
//		a full-line scan (opposed to a initial char check only),
//		the lookupTag() and lookupEndTag() have an extra oldstyle
//		parameter for this.
//
//
class TaggedParser {
  public:
    enum tag_type {
	NONE=0,		// no tag
	SINGLE,		// single value
	START,		// start of multi value
	END,		// end of multi value
	OLDSINGLE,	// tag has no prefix but a value
	OLDMULTI	// tag has no prefix and no value
    };
    typedef tag_type TagType;

  private:

    static const unsigned bufferLen_i;
    static char           buffer_ac[];

    std::string currentLine;

  private:

    streamoff _tagPos;		// position of tag
    streamoff _startPos;	// start postition of data
    streamoff _endPos;		// end position of data

    int _bufferPos;		// position of data in buffer
    unsigned int _bufferLen;	// length of data in buffer (unsigned for string::npos comparison)

    int _lineNumber;

    std::string _currentTag;
    std::string _currentLocale;
    std::string _currentData;		// substr of currentLine, set by data()

    bool _oldstyle;

    // set from start of line to start of tag
    int _offset;

  private:

    void _reset();
    void _datareset();

    // read line from stream
    static streamoff readLine (istream & stream_fr, std::string & cline_tr );

    // check line for tag
    TagType tagOnLine (const std::string & cline_tr, std::string & tag_tr,
			std::string::size_type & delim_ir, std::string & lang_tr);

  public:

    TaggedParser();
    virtual ~TaggedParser();

    void asOldstyle (bool oldstyle) { _oldstyle = oldstyle; _offset = (oldstyle?0:1); }
    static const streamoff nopos;

    int lineNumber () const { return _lineNumber; }

    streamoff tagPos() const { return _tagPos; }
    const std::string & currentTag() const { return _currentTag; }
    const std::string & currentLocale() const { return _currentLocale; }

  public:

    streamoff      dataStartPos () const { return _startPos; }
    streamoff      dataEndPos ()   const { return _endPos; }
    unsigned       dataLength ()   const { return _endPos - _startPos; }

    // valid after parseData()
    const std::string& data() { return (_currentData = currentLine.substr (_bufferPos, _bufferLen)); }

  public:

    /**
     * lookup a tag
     * parse until stag_tr, leave empty to parse until any tag. On succes
     * posStartTag() and posEndTag() can be used
     * Usually used to lookup a single or a start tag
     */
    TagType lookupTag( istream & stream_fr, const std::string & stag_tr = "", const std::string & slang_tr = "");

    /**
     * lookup end tag
     * set start and end retrieval positions
     */ 
    TagType lookupEndTag ( istream & stream_fr, const std::string & etag_tr,
                           const std::string & elang_tr = "", bool reverseLocale = false );

    // helper functions
    static std::string data2string( const std::list<std::string> & data_Vtr );
    static vector<std::string> split2words( const std::string & line_tr, const std::string & sepchars_tr = " \t\n" );
};

#endif // TaggedParser_h

// vim:sw=2
