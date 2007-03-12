/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                     (C) 2002 SuSE AG |
\----------------------------------------------------------------------/

   File:       TaggedFile.h
   Purpose:    Declare Tag and TagSet as interface to the TaggedParser
   Author:     Ludwig Nussel <lnussel@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/

#ifndef TaggedFile_h
#define TaggedFile_h
#include <iostream>
#include <string>
#include <map>
#include <list>
#include <zypp/parser/taggedfile/TaggedParser.h>
#include <zypp/parser/taggedfile/TagRetrievalPos.h>

namespace TaggedFile
{

	/**
	 * START = explicit start tag, return ACCEPTED_FULL when
	 *	      this tag occurs a second time
	 *	   N.B.: in the current implementation, a start tag
	 *	   can't have a locale (START implies REJECTLOCALE)
	 * <br>
	 * ALLOWLOCALE = accept locale for this tag
	 * <br>
	 * FORCELOCALE = force locale for this tag
	 * <br>
	 * REJECTLOCALE = reject locale (default !)
	 */
	enum tagtype {
	    REJECTLOCALE, START, ALLOWLOCALE, FORCELOCALE
	};

	/**
	 * data type allowed for this tag
	 * SINGLE = single line of data, retrieve data<br>
	 * SINGLEPOS = single line of data, just retrieve positions<br>
	 * MULTI = multiple lines of data<br>
	 * MULTIOLD = multiple lines of data<br>
	 *		end tag is start tag reversed
	 */
	enum datatype {
	    SINGLE, SINGLEPOS, MULTI, MULTIOLD, MULTIYOU
	};

	/**
	 * final assign status
	 * ACCEPTED = single tag accepted
	 * ACCEPTED_FULL = new start tag found
	 * <br>
	 * REJECTED_EOF = at end of file
	 * <br>
	 * REJECTED_NOMATCH = no matching tag found
	 * <br>
	 * REJECTED_LOCALE = matching tag found but locale not allowed
	 * <br>
	 * REJECTED_NOLOCALE = matching tag found but locale required
	 * <br>
	 * REJECTED_FULL = repeating non-start tag found
	 * <br>
	 * REJECTED_NOENDTAG = missing end tag
	 * <br>
	 */
	enum assignstatus {
	    ACCEPTED,		// 0
	    ACCEPTED_FULL,	// 1
	    REJECTED_EOF,	// 2
	    REJECTED_NOMATCH,	// 3
	    REJECTED_LOCALE,	// 4
	    REJECTED_NOLOCALE,	// 5
	    REJECTED_FULL,	// 6
	    REJECTED_NOENDTAG	// 7
	};

	static const streamoff nopos = streamoff(-1);

/**
* A Tag has a starttag, probably and endtag as well as data and the start and end positions in the stream
*/

class Tag
{
    public:
	typedef std::map<std::string, TagRetrievalPos> posmaptype;

    private:
	/** name of the tag */
	std::string _name;

	/** name of the end tag for datatype MULTIOLD */
	std::string _end;

	/**
	 * start and end position of data in stream
	 * indexed by locale
	 */
	posmaptype _pos;

	/**
	 * the actual data for a SINGLE, REJECTLOCALE datatype.
	 * for all other datatypes, only _pos is retrieved
	 */
	std::string _data;

	/**
	 * the type of data for this tag
	 */
	datatype _datatype;

	/**
	 * the type of tag
	 */
	tagtype _tagtype;

    public:
	/** Constructor
	 * @param name Name of Tag
	 * @param type how to handle multiple assignments of the same tag
	 * */
	Tag (const std::string& name, datatype dtype, tagtype ttype = REJECTLOCALE);

	/**
	 * override old-style end tag
	 * (needed e.g. for update.inf parsing which reversed
	 * DefaultInstsrcFTP to PTFCrstsniTluafed
	 * which can _not_ be handle automagically :-}
	 */
	void setEndTag (std::string end) { _end = end; }

	/**
	 * assign data from stream to tag
	 *
	 * @param parser = TaggedParser to use
	 * @param locale = locale found at tag
	 * @param istr = stream to use
	 * @returns assignstatus
	 *
	 * if REJECTED_NOENDTAG is returned, stream and parser are in an
	 * undefined state
	 */
	assignstatus assign (const std::string& locale, TaggedParser& parser, std::istream& istr);

	/** clears only data, not behavior nor tag names
	 * */
	void clear()
	{
	    _pos.clear();
	    _data.erase();
	}

	/**
	 * Name()
	 * return name of this tag
	 */
	const std::string& Name() const
	{
	    return _name;
	}

	/**
	 * return single line data of current tag
	 */
	const std::string& Data() const
	{
	    return _data;
	}

	/**
	 * return start position of data in stream
	 */
	const TagRetrievalPos Pos (const std::string& locale = "") const;

	/**
	 * return complete positionmap
	 */
	const posmaptype PosMap () const { return _pos; }

	/**
	 * return start position of data in stream
	 */
	std::streamoff posDataStart (const std::string& locale = "") const { return Pos(locale).begin(); }

	/**
	 * return end position of data in stream
	 */
	std::streamoff posDataEnd (const std::string& locale = "") const { return Pos(locale).end(); }

	friend std::ostream & operator<<( std::ostream & str, const TaggedFile::Tag & obj );
};

/** TagSet manages all Tags contained in a file. It redirects
 * assignments to the proper Tag
 * */
class TagSet
{
    private:
	/** file contains multiple sets or single set */
	bool _allow_multiple_sets;

	/** allow unknown tags */
	bool _allow_unknown_tags;

	/** language dependant tags, needed for setting the encoding */	
	typedef std::map<std::string, Tag *> tagmaptype;

	/** map of tags managed by this tagset */
	tagmaptype _tags;

	/**
	 * index <-> string mapping for access-by-index
	 * since this is faster and easier when handling the
	 * complete tagset
	 */
	typedef std::vector<Tag*> tagvectortype;
	tagvectortype _tagv;

	/** assign number to Tag
	 *
	 * @param idx number
	 * @param t Tag*
	 * */
	void setTagByIndex (int idx, Tag* t)
	{
	    if (idx < 0)
		return;
	    if (_tagv.size() <= (unsigned int)idx)
	    {
		_tagv.resize ((unsigned int)idx+1);
	    }
	    _tagv[(unsigned int)idx] = t;
	}

	/**
	 * re-use previous tag
	 * (used in assignSet() to re-use last parser state from
	 *  previous ACCEPTED_FULL)
	 */
	bool _reuse_previous_tag;

	/**
	 * lookup single Tag responsible for parsing starttag in map and
	 * call its assign function
	 *
	 * @param starttag Tag to assign
	 * @param istr stream to parse
	 * */
	assignstatus assign (const std::string& starttag, const std::string& startlocale, TaggedParser& parser, std::istream& istr);

    public:
    	TagSet();
	virtual ~TagSet();

	/**
	 * allow multiple sets
	 */
	void setAllowMultipleSets (bool flag) { _allow_multiple_sets = flag; }

	/**
	 * allow unknown tags
	 */
	void setAllowUnknownTags (bool flag) { _allow_unknown_tags = flag; }

	/** add Tag to TagSet
	 *
	 * @param name		name of tag
	 * @param idx		index for getTagByIndex, -1 for n/a
	 * @param dtype		datatype for tag
	 * @param ttype		tagtype for tag
	 * */
	void addTag (const std::string& name, int idx, datatype dtype, tagtype ttype = REJECTLOCALE)
	{
	    Tag *tag = new Tag (name, dtype, ttype);
	    _tags[name] = tag;
	    setTagByIndex (idx, tag);
	}

	/**
	 * assign complete TagSet from parser and stream
	 *
	 * @param parser parser to use
	 * @param istr stream to parse
	 */
	assignstatus assignSet (TaggedParser& parser, std::istream& istr);

	/** get Tag by number instead of string
	 *
	 * @param idx Tag number
	 * @return pointer to tag or NULL if idx doesn't exist
	 * */
	Tag* getTagByIndex (unsigned int idx)
	{
	    if (idx < _tagv.size ())
		return _tagv[idx];
	    else
		return NULL;
	}

	friend std::ostream & operator<<( std::ostream & str, const TaggedFile::TagSet & obj );
};

}  // namespace TaggedFile
 
#endif // TaggedFile_h

// vim:sw=4
