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

  File:       TagRetrievalPos.cc

  Author:     Klaus Kaempf <kkaempf@suse.de>
  Maintainer: Klaus Kaempf <kkaempf@suse.de>

  Purpose: Realize data access at position

/-*/

#include <iostream>

#include <zypp/base/Logger.h>
#include <zypp/parser/taggedfile/TagRetrievalPos.h>
#include <zypp/base/String.h>

using namespace std;
using namespace zypp;

// ------------------------------------------------------------------

const unsigned TagRetrievalPos::bufferLen_i = 1024;
char           TagRetrievalPos::buffer_ac[bufferLen_i];
const streamoff TagRetrievalPos::nopos = streamoff(-1);

// ------------------------------------------------------------------

/**
 * position stream to _begin
 *
 * return _end - _begin ( == expected data size)
 */
int
TagRetrievalPos::positionStream (istream & stream_fr) const
{
    if (empty())
	return 0;

    if (_begin == nopos
	|| _end == nopos
	|| _end < _begin)
    {
	ERR << "positions make no sense "<< stream_fr.rdstate() << "(" << _begin << ", " << _end << ")" << endl;;
   	return -1; // positions make no sense
    }

    stream_fr.clear();
    stream_fr.seekg( _begin );

    if ( !stream_fr.good() )
    {
	ERR << "seekg failed "<< stream_fr.rdstate() << "(" << _begin << ", " << _end << ")" << endl;;
	return -1; // illegal startData position
    }

    return _end - _begin;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : TagRetrievalPos::retrieveData	single-line
//	METHOD TYPE : bool
//
//	DESCRIPTION :	retrieve data from istream between two offsets
//			into a single string
//
bool
TagRetrievalPos::retrieveData (istream & stream_fr, string & data_tr ) const
{
    data_tr.erase();

    // position stream an calculate
    int expect_ii = positionStream (stream_fr);
    if ( expect_ii < 0)
	return false;		// error
    if ( expect_ii == 0)
	return true;		// startData position is valid, but we don't expect any data

    for ( unsigned toread_ii = min ((unsigned)expect_ii, bufferLen_i);
	toread_ii;
	expect_ii -= toread_ii, toread_ii = min( (unsigned)expect_ii, bufferLen_i ) )
    {
	stream_fr.read( buffer_ac, toread_ii );
	if ( stream_fr.gcount() != (int)toread_ii )
	{
	    data_tr.erase();
	    ERR << "data missing "<< stream_fr.rdstate() << "(" << _begin << ", " << _end << ")" << endl;;
	    return false; // not as many data available as expected
	}
	data_tr += string( buffer_ac, toread_ii );
    }

    // strip a trailing NL
    if ( data_tr[data_tr.size()-1] == '\n' )
    {
	data_tr.erase( data_tr.size()-1 );
    }

    return true;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : TagRetrievalPos::retrieveData	multi-line
//	METHOD TYPE : bool
//
//	DESCRIPTION :	retrieve data from istream between two offsets
//			into a list of strings
//
bool
TagRetrievalPos::retrieveData (istream & stream_fr, list<string>& data_Vtr ) const
{
    data_Vtr.clear();

    // position stream an calculate
    int expect_ii = positionStream (stream_fr);
    if ( expect_ii < 0)
	return false;		// error
    if ( expect_ii == 0)
	return true;		// startData position is valid, but we don't expect any data

    while ( streamoff( stream_fr.tellg() ) < _end)
    {
        string ln = str::getline( stream_fr );
	if ( ! ( stream_fr.fail() || stream_fr.bad() ) )
	{
	    data_Vtr.push_back( ln );
	}
	else
	{
	    break;
	}
    }

    return true;
}

