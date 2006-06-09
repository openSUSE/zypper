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
#include "zypp/base/Algorithm.h"
#include "zypp/base/Gettext.h"

#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"
#include "zypp/SourceManager.h"
#include "zypp/SourceFactory.h"
#include "zypp/Source.h"
#include "zypp/source/SourceImpl.h"
#include "zypp/target/store/PersistentStorage.h"
#include "zypp/TmpPath.h"
#include "zypp/Pathname.h"
#include "zypp/PathInfo.h"

///////////////////////////////////////////////////////////////////
#undef  ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "zypp::SourceManager"
///////////////////////////////////////////////////////////////////

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

    struct PrintSourceMapEntry
    {
      void operator()( const SourceMap::value_type & el ) const
      {
        _str << endl << "    - " << el.second;
      }
      PrintSourceMapEntry( std::ostream & str )
      : _str( str )
      {}
      std::ostream & _str;
    };

    inline std::ostream & dumpSourceTableOn( std::ostream & str, bool trailingENDL = true )
    {
      str << "SourceManager: =========================" << endl
          << "  known Sources " << _sources.size();
      std::for_each( _sources.begin(), _sources.end(), PrintSourceMapEntry(str) );

      str << endl
          << "  deleted Sources " << _deleted_sources.size();
      std::for_each( _deleted_sources.begin(), _deleted_sources.end(), PrintSourceMapEntry(str) );
      str << endl
          << "========================================";
      if ( trailingENDL )
        str << endl;
      return str;
    }

    inline bool sourceTableRemove( SourceMap::iterator it )
    {
      if ( it == _sources.end() )
        return false;

      MIL << "SourceManager remove " << it->second << endl;
      _deleted_sources[it->second.numericId()] = it->second;
      _sources.erase(it);

      // release all media of this source, not needed anymore (#159754)
      it->second.release();

      dumpSourceTableOn( DBG );
      return true;
    }

    inline SourceManager::SourceId sourceTableAdd( Source_Ref source_r )
    {
      if ( source_r.numericId() )
        {
          MIL << "SourceManager add " << source_r << endl;
          _sources[source_r.numericId()] = source_r;

          dumpSourceTableOn( DBG );
        }
      else
        {
          // Not worth an Exception. Request to add noSource, adds no Source,
          // and returns the noSource Id.
          WAR << "SourceManager does not add Source::noSource" << endl;
        }
      return source_r.numericId();
    }

  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SourceManager::SourceManager
  //	METHOD TYPE : Ctor
  //
  SourceManager::SourceManager()
  {
    MIL << "Created SourceManager Singleton." << endl;
  }

  ///////////////////////////////////////////////////////////////////
  //
  //	METHOD NAME : SourceManager::~SourceManager
  //	METHOD TYPE : Dtor
  //
  SourceManager::~SourceManager()
  {
    MIL << "Deleted SourceManager Singleton." << endl;
  }

  SourceManager::const_iterator SourceManager::begin() const
  { return _sources.begin(); }

  SourceManager::const_iterator SourceManager::end() const
  { return _sources.end();  }

  SourceManager::SourceId_const_iterator SourceManager::SourceId_begin() const
  { return make_map_key_begin( _sources ); }

  SourceManager::SourceId_const_iterator SourceManager::SourceId_end() const
  { return make_map_key_end( _sources ); }

  SourceManager::Source_const_iterator SourceManager::Source_begin() const
  { return make_map_value_begin( _sources ); }

  SourceManager::Source_const_iterator SourceManager::Source_end() const
  { return make_map_value_end( _sources ); }

  void SourceManager::reset()
  {
    MIL << "SourceManager reset (forget all sources)" << endl;
    _sources.clear();
    _deleted_sources.clear();
  }

  SourceManager::SourceId SourceManager::addSource( Source_Ref source_r )
  {
    return sourceTableAdd( source_r );
  }

  void SourceManager::removeSource(SourceManager::SourceId id)
  {
    if ( ! sourceTableRemove( _sources.find(id) ) )
      {
        WAR << "SourceManager remove: no source with SourceId " << id << endl;
      }
  }

  void SourceManager::removeSource( const std::string & alias_r )
  {
    SourceMap::iterator it = _sources.begin();
    for ( ; it != _sources.end() && it->second.alias() != alias_r; ++it )
      ; // empty body

    if ( ! sourceTableRemove( it ) )
      {
        WAR << "SourceManager remove: no source with alias " << alias_r << endl;
      }
  }

  void SourceManager::releaseAllSources()
  {
    MIL << "SourceManager releasing all sources ..." << endl;
    for (SourceMap::iterator it = _sources.begin();
	 it != _sources.end(); it++)
    {
      it->second.release();
    }
    MIL << "SourceManager releasing all sources done." << endl;
  }

  void SourceManager::reattachSources(const Pathname &attach_point)
  {
    MIL << "SourceManager reattach all sources to '" << attach_point << " ..." << endl;
    for (SourceMap::iterator it = _sources.begin();
	 it != _sources.end(); it++)
    {
      it->second.reattach(attach_point);
    }
    MIL << "SourceManager reattach all sources to '" << attach_point << " done." << endl;
  }


  void SourceManager::disableAllSources()
  {
    MIL << "SourceManager disable all sources ..." << endl;
    for( SourceMap::iterator it = _sources.begin(); it != _sources.end(); it++)
    {
	it->second.disable();
    }
    MIL << "SourceManager disable all sources done." << endl;
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
    MIL << "SourceManager store '" << root_r << ( metadata_cache ? "' (metadata_cache)" : "'" )
        << " ..." << endl;

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

    // delete before modifying and creating
    // so that we can recreate a deleted one (#174295)
    for( SourceMap::iterator it = _deleted_sources.begin(); it != _deleted_sources.end(); it++)
    {
	MIL << "Deleting source " << it->second << " from persistent store" << endl;
	store.deleteSource( it->second.alias() );
	filesystem::recursive_rmdir( it->second.cacheDir() );
    }

    _deleted_sources.clear();

    for( SourceMap::iterator it = _sources.begin(); it != _sources.end(); it++)
    {
	storage::PersistentStorage::SourceData descr;

	descr.url = it->second.url().asCompleteString();
        descr.enabled = it->second.enabled();
        descr.alias = it->second.alias();
	descr.autorefresh = it->second.autorefresh();
	descr.type = it->second.type();
	descr.product_dir = it->second.path();

	descr.cache_dir = it->second.cacheDir();

        if( metadata_cache && descr.cache_dir.empty() )
	{
	    if( descr.cache_dir.empty() )
	    {
              filesystem::TmpDir newCache( root_r /  ZYPP_METADATA_PREFIX, "Source." );
              descr.cache_dir = ZYPP_METADATA_PREFIX + newCache.path().basename();
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

    MIL << "SourceManager store done." << endl;
  }

  /** \todo Broken design: either use return value or Exception to
  * indicate errors, not both.
  */
  bool SourceManager::restore( Pathname root_r, bool use_caches, std::string alias_filter )
  {
    MIL << "SourceManager restore '" << root_r << ( use_caches ? "' (use_caches)" : "'" )
        << " ..." << endl;

    if (! _sources.empty() )
	ZYPP_THROW(SourcesAlreadyRestoredException());
//Exception ( N_("At least one source already registered, cannot restore sources from persistent store.") ) );

    FailedSourcesRestoreException report;

    storage::PersistentStorage store;
    store.init( root_r );

    std::list<storage::PersistentStorage::SourceData> new_sources = store.storedSources();

    MIL << "Found sources: " << new_sources.size() << endl;

    for( std::list<storage::PersistentStorage::SourceData>::iterator it = new_sources.begin();
	it != new_sources.end(); ++it)
    {
      // Note: Url(it->url).asString() to hide password in logs
      MIL << "Restoring source: url:[" << Url(it->url).asString() << "] product_dir:[" << it->product_dir << "] alias:[" << it->alias << "] cache_dir:[" << it->cache_dir << "]" << endl;

	SourceId id = 0;

	try {
          id = addSource( SourceFactory().createFrom(it->type, it->url, it->product_dir, it->alias, it->cache_dir) );
	}
	catch (const Exception &expt )
	{
	    // Note: Url(it->url).asString() to hide password in logs
	    ERR << "Unable to restore source from " << Url(it->url).asString()
	        << endl;

	    id = 0;
	    Url url2;
	    try {
		url2 = it->url;
		std::string scheme( url2.getScheme());

		if( (scheme == "cd" || scheme == "dvd") &&
		    !url2.getQueryParam("devices").empty())
		{
		    url2.setQueryParam("devices", "");

		    DBG << "CD/DVD devices changed - try again without a devices list"
		        << std::endl;

		    id = addSource( SourceFactory().createFrom(url2, it->product_dir, it->alias, it->cache_dir) );

		    // This worked ... update it->url ?
		    //it->url = url2.asCompleteString();
		}
	    }
	    catch (const Exception &e2)
	    {
		// Note: Url(it->url).asString() to hide password in logs
		ERR << "Unable to restore source from " << url2.asString()
	            << endl;
		id = 0;
		ZYPP_CAUGHT(e2);
	    }

	    if( id == 0)
	    {
              report.append( it->url.asString() + it->product_dir.asString(), it->alias, expt );
		continue;
	    }
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
        
        // user only wanted one source
        if ( (alias_filter.size() > 0) && (alias_filter ==src.alias()) )
          break;
    }

    if( !report.empty() )
    {
	ZYPP_THROW(report);
    }

    MIL << "SourceManager restore done." << endl;
    dumpSourceTableOn( DBG );
    return true;
  }

  void SourceManager::disableSourcesAt( const Pathname & root_r )
  {
    storage::PersistentStorage store;
    store.init( root_r );

    std::list<storage::PersistentStorage::SourceData> new_sources = store.storedSources();

    MIL << "Disabling all sources in store at " << root_r << endl;

    for( std::list<storage::PersistentStorage::SourceData>::iterator it = new_sources.begin();
	it != new_sources.end(); ++it)
    {
	MIL << "Disabling source " << it->alias << endl;
	it->enabled = false;
	store.storeSource( *it );
    }
  }

  std::list<std::string> SourceManager::knownAliases(const Pathname &root_r)
  {
    storage::PersistentStorage store;
    std::list<std::string> aliases;
    store.init( root_r );

    std::list<storage::PersistentStorage::SourceData> sources = store.storedSources();

    MIL << "Found sources: " << sources.size() << endl;

    for( std::list<storage::PersistentStorage::SourceData>::iterator it = sources.begin();
         it != sources.end(); ++it)
    {
      aliases.push_back(it->alias);
    }
    return aliases;
  }
    
  std::list<Url> SourceManager::knownUrls(const Pathname &root_r)
  {
    storage::PersistentStorage store;
    std::list<Url> urls;
    store.init( root_r );

    std::list<storage::PersistentStorage::SourceData> sources = store.storedSources();

    MIL << "Found sources: " << sources.size() << endl;

    for( std::list<storage::PersistentStorage::SourceData>::iterator it = sources.begin();
         it != sources.end(); ++it)
    {
      urls.push_back(it->url);
    }
    return urls;
  }
  
  /******************************************************************
  **
  **	FUNCTION NAME : operator<<
  **	FUNCTION TYPE : std::ostream &
  */
  std::ostream & operator<<( std::ostream & str, const SourceManager & obj )
  {
    return dumpSourceTableOn( str, /*tailingENDL*/false );
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
	}
    }
    ZYPP_THROW(Exception("Unknown source name '"+alias_r+"'"));
    /*NOTREACHED*/
    return it->second; // just to keep gcc happy
  }

  Source_Ref SourceManager::findSourceByUrl(const Url & url_r)
  {
    SourceMap::iterator it;
    for (it = _sources.begin(); it != _sources.end(); ++it)
    {
	if (it->second.url().asCompleteString() == url_r.asCompleteString()) {
	    return it->second;
	}
    }
    ZYPP_THROW(Exception("Unknown source URL '"+url_r.asString()+"'"));
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

  std::set<std::string> FailedSourcesRestoreException::aliases () const
  {
	return _aliases;
  }

  void FailedSourcesRestoreException::append( std::string source, std::string alias, const Exception& expt)
  {
	_summary = _summary + "\n" + source + ": " + expt.asString();
	_translatedSummary = _translatedSummary + "\n" + source + ": " + expt.asUserString();
	_aliases.insert( alias );
  }

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
