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
#include <map>

#include "zypp/base/Logger.h"
#include "zypp/base/Gettext.h"

#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"
#include "zypp/SourceManager.h"
#include "zypp/SourceFactory.h"
#include "zypp/Source.h"
#include "zypp/source/SourceImpl.h"
#include "zypp/target/store/PersistentStorage.h"
#include "zypp/Pathname.h"
#include "zypp/PathInfo.h"

#define ZYPP_METADATA_PREFIX ( getZYpp()->homePath().asString()+"/cache/" )

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  IMPL_PTR_TYPE(SourceManager)

  SourceManager_Ptr SourceManager::sourceManager()
  {
    static SourceManager_Ptr _source_manager( new SourceManager );
    return _source_manager;
  }

  namespace
  {
    typedef std::map<SourceManager::SourceId, Source_Ref> SourceMap;

    static SourceMap _sources;
    static SourceMap _deleted_sources;
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

  void SourceManager::reset()
  {
    MIL << "Cleaning up all sources" << endl;

    _sources.clear();
    _deleted_sources.clear();

    // don't change the _next_id to avoid using
    // the same ID for dangling old sources and newly introduced sources
  }

  SourceManager::SourceId SourceManager::addSource(const Url & url_r, const Pathname & path_r, const std::string & alias_r, const Pathname & cache_dir_r)
  {
    Source_Ref src( SourceFactory().createFrom(url_r, path_r, alias_r, cache_dir_r) );
    _sources[src.numericId()] = src;
    return src.numericId();
  }

  SourceManager::SourceId SourceManager::addSource(Source_Ref source_r)
  {
    _sources[source_r.numericId()] = source_r;
    return source_r.numericId();
  }

  void SourceManager::removeSource(SourceManager::SourceId id)
  {
    SourceMap::iterator it = _sources.find(id);
    if (it != _sources.end())
    {
      _deleted_sources[id] = it->second;
      _sources.erase(it);
    }
  }

  void SourceManager::releaseAllSources()
  {
    for (SourceMap::iterator it = _sources.begin();
	 it != _sources.end(); it++)
    {
      it->second.release();
    }
  }

  void SourceManager::reattachSources(const Pathname &attach_point)
  {
    for (SourceMap::iterator it = _sources.begin();
	 it != _sources.end(); it++)
    {
      it->second.reattach(attach_point);
    }
  }


  void SourceManager::removeSource(const std::string & alias_r)
  {
    for (SourceMap::iterator it = _sources.begin(); it != _sources.end(); ++it)
    {
	if (it->second.alias() == alias_r) {
            _deleted_sources[it->first] = it->second;
	    _sources.erase(it);
	    break;
	}
    }
  }

  void SourceManager::disableAllSources()
  {
    for( SourceMap::iterator it = _sources.begin(); it != _sources.end(); it++)
    {
	it->second.disable();
    }
  }

  std::list<SourceManager::SourceId> SourceManager::enabledSources() const
  {
    std::list<SourceManager::SourceId> res;

    for( SourceMap::const_iterator it = _sources.begin(); it != _sources.end(); it++)
    {
	if( it->second.enabled() )
	    res.push_back(it->first);
    }

    return res;
  }

  std::list<SourceManager::SourceId> SourceManager::allSources() const
  {
    std::list<SourceManager::SourceId> res;

    for( SourceMap::const_iterator it = _sources.begin(); it != _sources.end(); it++)
    {
	res.push_back(it->first);
    }

    return res;
  }

  void SourceManager::store(Pathname root_r, bool metadata_cache )
  {
    storage::PersistentStorage store;
    store.init( root_r );


    // make sure to create the source metadata cache
    if( metadata_cache )
    {
	// make sure our root exists

	filesystem::assert_dir( root_r / getZYpp()->homePath() );
        Pathname topdir( root_r / ZYPP_METADATA_PREFIX );
        filesystem::assert_dir( topdir );
    	MIL << "Created..." << topdir << std::endl;
    }

    unsigned id = 0;

    // first, gather all known cache dirs
    std::set<std::string> known_caches;
    for( SourceMap::iterator it = _sources.begin(); it != _sources.end(); it++)
    {
	if( ! it->second.cacheDir().empty() )
	    known_caches.insert( it->second.cacheDir().asString() );
    }

    for( SourceMap::iterator it = _sources.begin(); it != _sources.end(); it++)
    {
	storage::PersistentStorage::SourceData descr;

	descr.url = it->second.url().asCompleteString();
        descr.enabled = it->second.enabled();
        descr.alias = it->second.alias();
	descr.autorefresh = it->second.autorefresh();
	descr.type = it->second.type();
	descr.product_dir = it->second.path().asString();

	descr.cache_dir = it->second.cacheDir().asString();

	if( metadata_cache && it->second.cacheDir().empty() )
	{
	    if( descr.cache_dir.empty() )
	    {
		// generate the new cache name

		std::string cache = ZYPP_METADATA_PREFIX + str::numstring(id); // we should strip root here

		// generate a new cache dir
		while( id < 1000 && known_caches.find( cache ) != known_caches.end() )
		{
		    ++id;
		    cache = ZYPP_METADATA_PREFIX + str::numstring(id); // we should strip root here
		}

		if ( id == 1000 )
		{
		    ERR << "Unable to generate a new cache directory name" << endl;
		    metadata_cache = false;
		    continue;
		}

		descr.cache_dir = cache;

		known_caches.insert( cache );
	    }

	    filesystem::assert_dir ( root_r.asString() + descr.cache_dir );

	    MIL << "Storing metadata to (" << root_r.asString() << ")/" << descr.cache_dir << endl;

	    try {
		it->second.storeMetadata( root_r.asString() + descr.cache_dir );
	    }
	    catch(const Exception &excp) {
		WAR << "Creating local metadata cache failed, not using cache" << endl;
		descr.cache_dir = "";
	    }
	}

	store.storeSource( descr );
    }

    for( SourceMap::iterator it = _deleted_sources.begin(); it != _deleted_sources.end(); it++)
    {
	MIL << "Deleting source " << it->second << " from persistent store" << endl;
	store.deleteSource( it->second.alias() );
    }

    _deleted_sources.clear();
  }

  /** \todo Broken design: either use return value or Exception to
  * indicate errors, not both.
  */
  bool SourceManager::restore(Pathname root_r, bool use_caches )
  {
    if (! _sources.empty() )
	ZYPP_THROW(Exception ( N_("At least one source already registered, cannot restore sources from persistent store.") ) );

    FailedSourcesRestoreException report;

    storage::PersistentStorage store;
    store.init( root_r );

    std::list<storage::PersistentStorage::SourceData> new_sources = store.storedSources();

    MIL << "Found sources: " << new_sources.size() << endl;

    for( std::list<storage::PersistentStorage::SourceData>::iterator it = new_sources.begin();
	it != new_sources.end(); ++it)
    {
      MIL << "Restoring source: url:[" << it->url << "] product_dir:[" << it->product_dir << "] alias:[" << it->alias << "] cache_dir:[" << it->cache_dir << "]" << endl;

	SourceId id = 0;

	try {
	    id = addSource(it->url, it->product_dir, it->alias, it->cache_dir);
	}
	catch ( Exception expt ){
	    ERR << "Unable to restore source from " << it->url << endl;
	    report.append( it->url + it->product_dir, expt );
	    continue;
	}
	DBG << "Added source as id " << id << endl;
	// should not throw, we've just created the source
	Source_Ref src = findSource( id );

	if ( it->enabled ) {
	    DBG << "enable source" << endl;
	    src.enable();
	}
	else {
	    DBG << "disable source" << endl;
	    src.disable();
	}
	src.setAutorefresh ( it->autorefresh );
    }

    if( !report.empty() )
    {
	ZYPP_THROW(report);
    }
    return true;
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

  Source_Ref SourceManager::findSource(SourceId id)
  {
    SourceMap::iterator it = _sources.find(id);
    if (it == _sources.end())
    {
      ZYPP_THROW(Exception("Unknown source ID"));
    }
    return it->second;
  }

  Source_Ref SourceManager::findSource(const std::string & alias_r)
  {
    SourceMap::iterator it;
    for (it = _sources.begin(); it != _sources.end(); ++it)
    {
	if (it->second.alias() == alias_r) {
	    return it->second;
	    break;
	}

    }
    ZYPP_THROW(Exception("Unknown source name '"+alias_r+"'"));
    /*NOTREACHED*/
    return it->second; // just to keep gcc happy
  }

  /////////////////////////////////////////////////////////////////
  // FailedSourcesRestoreException
  ///////////////////////////////////////////////////////////////////

  std::ostream & FailedSourcesRestoreException::dumpOn( std::ostream & str ) const
  {
	return str << _summary;
  }

  std::ostream & FailedSourcesRestoreException::dumpOnTranslated( std::ostream & str ) const
  {
	return str << Exception::asUserString() << endl << _translatedSummary;
  }

  bool FailedSourcesRestoreException::empty () const
  {
	return _summary.empty();
  }

  void FailedSourcesRestoreException::append( std::string source, const Exception& expt)
  {
	_summary = _summary + "\n" + source + ": " + expt.asString();
	_translatedSummary = _translatedSummary + "\n" + source + ": " + expt.asUserString();

  }
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
