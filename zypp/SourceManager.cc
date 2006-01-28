/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SourceManager.cc
 *
*/
#include <iostream>
#include "zypp/base/Logger.h"

#include "zypp/SourceManager.h"
#include "zypp/SourceFactory.h"
#include "zypp/Source.h"
#include "zypp/source/SourceImpl.h"


using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE(SourceManager)

  SourceManager_Ptr SourceManager::_source_manager;

  SourceManager_Ptr SourceManager::sourceManager()
  {
    if ( ! _source_manager )
      _source_manager = new SourceManager;
    return _source_manager;
  }


  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SourceManager::SourceManager
  //	METHOD TYPE : Ctor
  //
  SourceManager::SourceManager()
  {}

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SourceManager::~SourceManager
  //	METHOD TYPE : Dtor
  //
  SourceManager::~SourceManager()
  {}

  unsigned SourceManager::addSource(const Url & url_r, const Pathname & path_r, const std::string & alias_r)
  {
    Source src = SourceFactory().createFrom(url_r, path_r, alias_r);
    RW_pointer<Source> src_ptr = RW_pointer<Source>(new Source(src));
    _sources[_next_id] = src_ptr;
    return _next_id++;
  }

  unsigned SourceManager::addSource(Source_Ref source_r)
  {
    RW_pointer<Source> src_ptr = RW_pointer<Source>(new Source(source_r));
    _sources[_next_id] = src_ptr;
    return _next_id++;
  }

  void SourceManager::removeSource(const unsigned id)
  {
    SourceMap::iterator it = _sources.find(id);
    if (it != _sources.end())
    {
      _sources.erase(it);
    }
  }

  void SourceManager::removeSource(const std::string & alias_r)
  {
    for (SourceMap::iterator it = _sources.begin(); it != _sources.end(); ++it)
    {
	if (it->second->alias() == alias_r) {
	    _sources.erase(it);
	    break;
	}
    }
  }

  void SourceManager::disableAllSources()
  {
    for( SourceMap::iterator it = _sources.begin(); it != _sources.end(); it++)
    {
	it->second->disable ();
    }
  }

  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const SourceManager & obj )
  {
    return str << "Source Manager has " << " sources" << endl;
  }

  unsigned SourceManager::_next_id = 0;

  Source_Ref SourceManager::findSource(const unsigned id)
  {
    SourceMap::iterator it = _sources.find(id);
    if (it == _sources.end())
    {
      ZYPP_THROW(Exception("Unknown source ID"));
    }
    return *(it->second);
  }

  Source_Ref SourceManager::findSource(const std::string & alias_r)
  {
    SourceMap::iterator it;
    for (it = _sources.begin(); it != _sources.end(); ++it)
    {
	if (it->second->alias() == alias_r) {
	    return *(it->second);
	    break;
	}
	
    }
    ZYPP_THROW(Exception("Unknown source name '"+alias_r+"'"));
    /*NOTREACHED*/
    return *(it->second); // just to keep gcc happy
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
