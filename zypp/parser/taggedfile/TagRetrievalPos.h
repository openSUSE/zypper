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

  File:       TagRetrievalPos.h

  Author:     Klaus Kaempf <kkaempf@suse.de>
  Maintainer: Klaus Kaempf <kkaempf@suse.de>

  Purpose: Keep data and provide functions for on-demand retrieval
	   of cache values

/-*/
#ifndef TagRetrievalPos_h
#define TagRetrievalPos_h

#include <iostream>
#include <string>
#include <list>

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : TagRetrievalPos
//
class TagRetrievalPos {
    private:
	std::streamoff _begin;
	std::streamoff _end;

	// position stream and calculate expected characters
	int positionStream (std::istream & stream_fr) const;

	static const unsigned bufferLen_i;
	static char           buffer_ac[];

    public:
	static const std::streamoff nopos;
	TagRetrievalPos () : _begin(nopos), _end (nopos) {}
	TagRetrievalPos (std::streamoff begin, std::streamoff end) : _begin(begin), _end (end) {}
	~TagRetrievalPos() {}

	/**
	 * test if empty
	 */
	bool empty () const { return _end == nopos; }

	/**
	 * access functions
	 */
	const std::streamoff begin() const { return _begin; }
	const std::streamoff end() const { return _end; }

	/**
	 * set position
	 */
	void set (std::streamoff begin, std::streamoff end) {
	  if ( begin < end ) {
	    _begin = begin;
	    _end = end;
	  }
	}

	/**
	 * retrieve single-line data
	 */
	bool retrieveData (std::istream& input, std::string& data_r) const;

	/**
	 * retrieve multi-line data
	 */
	bool retrieveData (std::istream& input, std::list<std::string>& data_r) const;
};

#endif // TagRetrievalPos_h
