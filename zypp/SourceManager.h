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
     * \throws Exception
     */    
    void store(Pathname root_r);
    
    /**
     * Restore the sources state to the given path. If the sources
     * database is not empty, it throws an exception
     *
     * \throws Exception
     */
    void restore(Pathname root_r);
    
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
     * Disable all registered sources
     */
#warning: this could be done by providing iterator-like methods
    void disableAllSources();

  private:
#warning move data to a PIMPL or anonymous namespace as it's a Singleton
    typedef std::map<unsigned, RW_pointer<Source_Ref> > SourceMap;

    SourceMap _sources;

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
