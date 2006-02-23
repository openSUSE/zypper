/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SourceManager.h
 *
*/
#ifndef ZYPP_SOURCEMANAGER_H
#define ZYPP_SOURCEMANAGER_H

#include <iosfwd>
#include <map>
#include <list>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"

#include "zypp/Source.h"
#include "zypp/Url.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(SourceManager)

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SourceManager
  //
  /** Manage a ResObject pool. */
  class SourceManager : public base::ReferenceCounted, private base::NonCopyable
  {
    friend std::ostream & operator<<( std::ostream & str, const SourceManager & obj );


  public:
    static SourceManager_Ptr sourceManager();

  private:
    /** Default ctor */
    SourceManager();
  public:
    /** Dtor */
    ~SourceManager();

  public:
  
    /**
     * Store the current state to the given path
     *
     * \param root_r root path for storing the source definitions
     * \param metadata_cache if true, this will also store/update
     * metadata caches for the sources.
     *
     * \throws Exception
     */    
    void store(Pathname root_r, bool metadata_cache );
    
    /**
     * Restore the sources state to the given path. If the sources
     * database is not empty, it throws an exception
     *
     * \param use_caches  if true, source creation will try to use source cache
     * and it's behavior on autorefresh. If false, it will not use
     * the cache at all.
     *
     * \return true on success
     *
     * \throws Exception
     */
    bool restore(Pathname root_r, bool use_caches = true);
    
    /**
     * Find a source with a specified ID
     *
     * \throws Exception
     */
    Source_Ref findSource(const unsigned id);

    /**
     * Find a source with a specified alias
     *
     * \throws Exception
     */
    Source_Ref findSource(const std::string & alias_r);
    
    /**
     * Return the list of the currently enabled sources
     *
     */
    std::list<unsigned int> enabledSources() const;

    /**
     * Return ids of all sources
     *
     */
    std::list<unsigned int> allSources() const;

    /**
     * Add a new source
     *
     * \throws Exception
     */
    unsigned addSource(const Url & url_r, const Pathname & path_r = "/", const std::string & name_r = "", const Pathname & cache_dir_r = "");

    /**
     * Add a new source
     *
     * \throws Exception
     */
    unsigned addSource(Source_Ref source_r);

    /**
     * Remove an existing source by ID
     *
     * \throws Exception
     */
    void removeSource(const unsigned id);

    /**
     * Remove an existing source by Alias
     *
     * \throws Exception
     */
    void removeSource(const std::string & alias_r);

    /**
     * Release all medias held by all sources
     *
     * \throws Exception
     */
    void releaseAllSources();

    /**
     * Disable all registered sources
     */
#warning: this could be done by providing iterator-like methods
    void disableAllSources();

  private:
#warning move data to a PIMPL or anonymous namespace as it's a Singleton
    typedef std::map<unsigned, RW_pointer<Source_Ref> > SourceMap;

    SourceMap _sources;
    SourceMap _deleted_sources;

    static unsigned _next_id;

  private:
    /** Singleton */
    static SourceManager_Ptr _source_manager;


  };
  ///////////////////////////////////////////////////////////////////

  /** \relates SourceManager Stream output */
  std::ostream & operator<<( std::ostream & str, const SourceManager & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCEMANAGER_H
