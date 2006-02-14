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
#include "zypp/target/store/PersistentStorage.h"

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

  unsigned SourceManager::addSource(const Url & url_r, const Pathname & path_r, const std::string & alias_r, const Pathname & cache_dir_r)
  {
    Source_Ref src( SourceFactory().createFrom(url_r, path_r, alias_r, cache_dir_r) );
    RW_pointer<Source_Ref> src_ptr = RW_pointer<Source_Ref>(new Source_Ref(src));
    _sources[_next_id] = src_ptr;
    return _next_id++;
  }

  unsigned SourceManager::addSource(Source_Ref source_r)
  {
    RW_pointer<Source_Ref> src_ptr = RW_pointer<Source_Ref>(new Source_Ref(source_r));
    _sources[_next_id] = src_ptr;
    return _next_id++;
  }

  void SourceManager::removeSource(const unsigned id)
  {
    SourceMap::iterator it = _sources.find(id);
    if (it != _sources.end())
    {
      RW_pointer<Source_Ref> src_ptr = RW_pointer<Source_Ref>(new Source_Ref(*(it->second)));
      _deleted_sources[id] = src_ptr;

      _sources.erase(it);
    }
  }

  void SourceManager::removeSource(const std::string & alias_r)
  {
    for (SourceMap::iterator it = _sources.begin(); it != _sources.end(); ++it)
    {
	if (it->second->alias() == alias_r) {
            RW_pointer<Source_Ref> src_ptr = RW_pointer<Source_Ref>(new Source_Ref(*(it->second)));
            _deleted_sources[it->first] = src_ptr;

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

  std::list<unsigned int> SourceManager::enabledSources() const
  {
    std::list<unsigned int> res;
    
    for( SourceMap::const_iterator it = _sources.begin(); it != _sources.end(); it++)
    {
	if( it->second->enabled() )
	    res.push_back(it->first);
    }
    
    return res;
  }

  std::list<unsigned int> SourceManager::allSources() const
  {
    std::list<unsigned int> res;
    
    for( SourceMap::const_iterator it = _sources.begin(); it != _sources.end(); it++)
    {
	res.push_back(it->first);
    }
    
    return res;
  }

  void SourceManager::store(Pathname root_r)
  {
    storage::PersistentStorage store;    
    store.init( root_r );
  
    for( SourceMap::iterator it = _sources.begin(); it != _sources.end(); it++)
    {
	storage::PersistentStorage::SourceData descr;
	
	descr.url = it->second->url().asString();
        descr.enabled = it->second->enabled();
        descr.alias = it->second->alias();
	descr.autorefresh = it->second->autorefresh();
	descr.type = it->second->type();
	
	// FIXME: product_dir
	store.storeSource( descr );
    }

    for( SourceMap::iterator it = _deleted_sources.begin(); it != _deleted_sources.end(); it++)
    {
	MIL << "Deleting source " << it->second << " from persistent store" << endl;
	store.deleteSource( it->second->alias() );
    }
    
    _deleted_sources.clear();
  }

  void SourceManager::restore(Pathname root_r)
  {
    if (! _sources.empty() )
	ZYPP_THROW(Exception ("At least one source already registered, cannot restore sources from persistent store.") );
    
    storage::PersistentStorage store;    
    store.init( root_r );
    
    std::list<storage::PersistentStorage::SourceData> new_sources = store.storedSources();

    for( std::list<storage::PersistentStorage::SourceData>::iterator it = new_sources.begin();
	it != new_sources.end(); ++it)
    {
	MIL << "Restoring source: " << it->url << it->product_dir << " with alias " << it->alias << endl;
	unsigned id = addSource(it->url, it->product_dir, it->alias);
	// FIXME: enable, autorefresh
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
