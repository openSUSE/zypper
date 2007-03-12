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

   File:	TaggedFile.cc

   Author:     Michael Andres <ma@suse.de>
   Maintainer: Michael Andres <ma@suse.de>

/-*/
#include <ctype.h> // for toupper

#include <zypp/parser/taggedfile/TaggedFile.h>
#include <zypp/parser/taggedfile/TagRetrievalPos.h>
#include <zypp/base/Logger.h>
#include <iostream>

using namespace zypp;

namespace TaggedFile
{

///////////////////////////////////////////////////////////////////////
// Tag
//-------------------------------------------------------------------

/******************************************************************
**
**
**	FUNCTION NAME : Tag::Tag ()
**	FUNCTION TYPE : constructor
**
**	DESCRIPTION set _name, _datatype, and _tagtype
**		it datatype == MULTIOLD, also send end
*/

Tag::Tag (const std::string& name, datatype dtype, tagtype ttype)
    : _name(name)
    , _datatype(dtype)
    , _tagtype(ttype)
{
    if (_datatype == MULTIOLD || _datatype == MULTIYOU)
    {
	unsigned int namepos = _name.size();
	if (namepos == 0)
	    return;
	_end.reserve (namepos);
	_end += toupper (_name[--namepos]);
	for (;;)
	{
	    namepos--;

	    if (namepos == 0)
	    {
		_end += tolower (_name[namepos]);
		break;
	    }
	    _end += _name[namepos];
	}
	while (namepos > 0);
    }
}


/******************************************************************
**
**
**	FUNCTION NAME : Tag::assign ()
**	FUNCTION TYPE : assignstatus
**
**	DESCRIPTION assign a value/position to _this_ Tag
//
// in order to find _this_, the initial lookupTag must have already
// taken place. So for single-line tags, we just have to check
// the locale and assing start and end positions.
//
// For multi-line tags. this functions searches the matching end tag
// and assigns start and end positions to the proper locale slot.
*/

assignstatus
Tag::assign (const std::string& locale, TaggedParser& parser, std::istream& istr)
{
//std::cerr << "Tag::assign(" << _name << "." << locale << ")@" << parser.lineNumber() << std::endl;

    if (locale.empty())
    {
	if (_tagtype == FORCELOCALE)
	    return REJECTED_NOLOCALE;
    }
    else	// locale not empty
    {
	if (_tagtype == REJECTLOCALE)
	    return REJECTED_LOCALE;
    }

    // this tag _again_ ?

    posmaptype::iterator it = _pos.find (locale);

    if (it != _pos.end())		// we already had this one
    {
	if (_tagtype == START)
	    return ACCEPTED_FULL;	// ok for start
	else
	    return REJECTED_FULL;	// not ok for others
    }

    switch (_datatype)
    {
	case SINGLE:			// retrieve data
	    _data = parser.data();
	break;
	case SINGLEPOS:			// skip data
	break;
	case MULTI:			// for multi-line data, look for the matching end tag
	    if (parser.lookupEndTag (istr, _name, locale) != TaggedParser::END)
	    {
		ERR << "Endtag not found" << std::endl;
		return REJECTED_NOENDTAG;
	    }
	break;
	case MULTIOLD:
	    if (parser.lookupEndTag (istr, _end, locale) != TaggedParser::OLDMULTI)
	    {
		ERR << "Endtag not found" << std::endl;
		return REJECTED_NOENDTAG;
	    }
	break;
	case MULTIYOU:
	    if (parser.lookupEndTag (istr, _end, locale, true) != TaggedParser::OLDMULTI)
	    {
		ERR << "Endtag not found" << std::endl;
		return REJECTED_NOENDTAG;
	    }
	break;
    }
    // remember positions
    _pos[locale] = TagRetrievalPos (parser.dataStartPos (), parser.dataEndPos ());

    return ACCEPTED;
}

/******************************************************************
**
**
**	FUNCTION NAME : Pos
**	FUNCTION TYPE : TagRetrievalPos
**
**	DESCRIPTION : return position matching locale
**		      or TagRetrievalPos(nopos,nopos) if locale not defined
*/
const TagRetrievalPos
Tag::Pos (const std::string& locale) const
{
    posmaptype::const_iterator it = _pos.find (locale);
    if (it == _pos.end())
	return TagRetrievalPos();
    return it->second;
}


/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : ostream &
**
**	DESCRIPTION : print Tag to stream
*/
std::ostream & operator<<( std::ostream & str, const TaggedFile::Tag & obj )
{
    str << "Tag: '" << obj._name << "' ";
    switch (obj._tagtype) {
	case TaggedFile::REJECTLOCALE: str << "REJECTLOCALE"; break;
	case TaggedFile::START: str << "START"; break;
	case TaggedFile::ALLOWLOCALE: str << "ALLOWLOCALE"; break;
	case TaggedFile::FORCELOCALE: str << "FORCELOCALE"; break;
    }
    str << ", ";
    switch (obj._datatype) {
	case TaggedFile::SINGLE: str << "SINGLE"; break;
	case TaggedFile::SINGLEPOS: str << "SINGLEPOS"; break;
	case TaggedFile::MULTI: str << "MULTI"; break;
	case TaggedFile::MULTIOLD: str << "MULTIOLD"; break;
	case TaggedFile::MULTIYOU: str << "MULTIYOU"; break;
    }
    str << std::endl;
    return str;
}

///////////////////////////////////////////////////////////////////////
// TagSet
//-------------------------------------------------------------------

TagSet::TagSet()
    : _allow_multiple_sets (false)
    , _allow_unknown_tags (true)
    , _reuse_previous_tag (false)
{
}

TagSet::~TagSet()
{
    // delete tags from map, allocated in addTag()
    tagmaptype::iterator it;
    for (it = _tags.begin(); it != _tags.end(); ++it)
    {
	delete it->second;
    }
}

/******************************************************************
**
**
**	FUNCTION NAME : TagSet::assign ()
**	FUNCTION TYPE : assignstatus
**
**	DESCRIPTION assign a single value/position to a single Tag
*/
assignstatus
TagSet::assign (const std::string& starttag, const std::string& startlocale, TaggedParser& parser, std::istream& istr)
{
    // find given Tag in map

    tagmaptype::iterator t = _tags.find (starttag);

    if (t == _tags.end())
    {
	if (_allow_unknown_tags)
	    return ACCEPTED;
	return REJECTED_NOMATCH;
    }

//std::cerr << "TagSet::assign(" << starttag << "." << startlocale << ")@" << parser.lineNumber() << std::endl;
    // assign to found tag
    return t->second->assign (startlocale, parser, istr);
}


/******************************************************************
**
**
**	FUNCTION NAME : TagSet::assignSet ()
**	FUNCTION TYPE : assignstatus
**
**	DESCRIPTION assign a complete TagSet from a stream
*/
assignstatus
TagSet::assignSet (TaggedParser& parser, std::istream& istr)
{
//std::cerr << "TagSet::assignSet(-------------------)@" << parser.lineNumber() << std::endl;

    // reset tag set

    tagmaptype::iterator it;
    for (it = _tags.begin(); it != _tags.end(); ++it)
    {
	it->second->clear();
    }

    int count = -1;

    // fill tag set

    assignstatus ret = ACCEPTED;
    do
    {
	// only call parser if it isn't already set from a previous
	// ACCEPTED_FULL

	if (!_reuse_previous_tag)
	{
	    TaggedParser::TagType tagtype = parser.lookupTag (istr);

	    if (tagtype == TaggedParser::NONE)	// no tag found
	    {
		if (istr.eof ())
		{
		    if (count > 0)
			ret = ACCEPTED_FULL;
		    else
			ret = REJECTED_EOF;
		}
		else if (_allow_unknown_tags)
		{
		    continue;
		}
		else
		{
		    ret = REJECTED_NOMATCH;
		}
		break;
	    }
	    else
		ret = ACCEPTED;
	}
	else
	{
	    _reuse_previous_tag = false;
	}

	if (ret == ACCEPTED)
	{
	    ret = assign (parser.currentTag(), parser.currentLocale(), parser, istr);
	    ++count;
	}
    }
    while (ret == ACCEPTED);

    if ((ret == ACCEPTED_FULL)
	&& _allow_multiple_sets)
    {
	// re use last scanned tag on next entry
	_reuse_previous_tag = true;
    }
    return ret;
}



/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : ostream &
**
**	DESCRIPTION : print every contained tag
*/
std::ostream & operator<<( std::ostream & str, const TagSet & obj )
{
    str << "TagSet {" << std::endl;
    TaggedFile::TagSet::tagmaptype::const_iterator it;
    for (it = obj._tags.begin(); it != obj._tags.end(); ++it)
    {
	str << *(it->second);
    }
    str << "}" << std::endl;
    return str;
}


}


// vim:sw=4
