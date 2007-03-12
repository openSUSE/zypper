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

  File:       TagCacheRetrieval.h

  Author:     Michael Andres <ma@suse.de>
  Maintainer: Michael Andres <ma@suse.de>

  Purpose: Keep data and provide functions for on-demand retrieval
	   of cache values

/-*/
#ifndef TagCacheRetrieval_h
#define TagCacheRetrieval_h

#include <iosfwd>
#include <string>
#include <fstream>

#include <zypp/parser/taggedfile/TaggedParser.h>
#include <zypp/parser/taggedfile/TagRetrievalPos.h>

#include <zypp/parser/taggedfile/TagCacheRetrievalPtr.h>
#include <zypp/Pathname.h>

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : TagCacheRetrieval
//
class TagCacheRetrieval : public CountedRep {
    REP_BODY(TagCacheRetrieval);
    private:
	// the name of the file
	std::string _name;

	// hint to keep stream open
	bool _keep_open;

	// the stream to read data from
	std::ifstream _stream;

    public:

	TagCacheRetrieval (const zypp::Pathname& filename);
	~TagCacheRetrieval();

	void startRetrieval();
	void stopRetrieval();

	/**
	 * access to stream and parser
	 * these are non-const because the caller might clobber the values
	 */
	std::string& getName (void) { return _name; }

	/**
	 * access to values
	 */
	bool retrieveData (const TagRetrievalPos& pos, std::list<std::string> &data_r);
	bool retrieveData (const TagRetrievalPos& pos, std::string &data_r);
};

///////////////////////////////////////////////////////////////////

#endif // TagCacheRetrieval_h
