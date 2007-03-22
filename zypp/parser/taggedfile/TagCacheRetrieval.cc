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

  File:       TagCacheRetrieval.cc

  Author:     Klaus Kaempf <kkaempf@suse.de>
  Maintainer: Klaus Kaempf <kkaempf@suse.de>

  Purpose: Realize PMCacheRetrieval

/-*/

#include <iostream>

#include <zypp/base/Logger.h>
#include <zypp/parser/taggedfile/TagCacheRetrieval.h>

using namespace std;
using namespace zypp;

///////////////////////////////////////////////////////////////////
//
//      CLASS NAME : TagCacheRetrieval
//
///////////////////////////////////////////////////////////////////

IMPL_PTR_TYPE(TagCacheRetrieval)

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : TagCacheRetrieval::TagCacheRetrieval
//	METHOD TYPE : Constructor
//
//	DESCRIPTION : open file stream and keep pointer to tag parser
//		      for later value retrieval on-demand
//
TagCacheRetrieval::TagCacheRetrieval(const Pathname& name)
    : _name (name.asString())
    , _keep_open (false)
{
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : TagCacheRetrieval::~TagCacheRetrieval
//	METHOD TYPE : Destructor
//
//	DESCRIPTION :
//
TagCacheRetrieval::~TagCacheRetrieval()
{
}


///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : TagCacheRetrieval::retrieveData
//	METHOD TYPE : bool
//
//	DESCRIPTION : retrieves data at TagRetrievalPos pos and fills
//			data_r appropriately
//
bool
TagCacheRetrieval::retrieveData(const TagRetrievalPos& pos, std::list<std::string> &data_r)
{
    data_r.clear();

    if (!_stream.is_open())
    {
	_stream.open (_name.c_str());
	if (_stream.fail())
	{
	    ERR << "Open (" << _name << ") failed" << endl;
	    return false;
	}
    }
    bool ret = pos.retrieveData (_stream, data_r);
    if (!_keep_open)
    {
	_stream.close ();
    }
    return ret;
}

bool
TagCacheRetrieval::retrieveData(const TagRetrievalPos& pos, std::string &data_r)
{
    data_r.erase();

    if (pos.empty())
    {
	return true;
    }
    if (!_stream.is_open())
    {
	_stream.open (_name.c_str());
	if (_stream.fail())
	{
	    ERR << "Open (" << _name << ") failed";
	    return false;
	}
    }
    std::list<std::string> listdata;
    bool ret = false;
    if (pos.retrieveData (_stream, listdata)
	&& !listdata.empty())
    {
	data_r = listdata.front();
	ret = true;
    }
    if (!_keep_open)
    {
	_stream.close ();
    }
    return ret;
}



void
TagCacheRetrieval::startRetrieval()
{
    _keep_open = true;
}

void
TagCacheRetrieval::stopRetrieval()
{
    if (_stream.is_open())
	_stream.close();
    _keep_open = false;
}

