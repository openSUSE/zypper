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
#include <list>

#include "zypp/base/ReferenceCounted.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/Gettext.h"

#include "zypp/Source.h"
#include "zypp/Url.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  DEFINE_PTR_TYPE(SourceManager)

  class FailedSourcesRestoreException : public Exception
  {
    public:
      FailedSourcesRestoreException()
      : Exception(N_("Unable to restore all sources."))
      , _summary()
      , _translatedSummary()
      {}
      virtual ~FailedSourcesRestoreException() throw() {};

      void append( std::string source, const Exception& problem );
      bool empty() const;
    protected:
      virtual std::ostream & dumpOn( std::ostream & str ) const;
      virtual std::ostream & dumpOnTranslated( std::ostream & str ) const;
    private:
      std::string _summary;
      std::string _translatedSummary;
  };

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SourceManager
  //
  /** Provide the known Sources.
   * \todo make it a resl singleton
   * \todo throwing findSource is not acceptable, return either
   * a Source or noSource.
   * \todo Make restore either void or nonthrowing, but two ways of
   * error reporting is bad.
  */
  class SourceManager : public base::ReferenceCounted, private base::NonCopyable
  {
    friend std::ostream & operator<<( std::ostream & str, const SourceManager & obj );


  public:
    /** Singleton access */
    static SourceManager_Ptr sourceManager();

  public:
    /** Dtor */
    ~SourceManager();

  public:
    /** Runtime unique numeric Source Id. */
    typedef Source_Ref::NumericId SourceId;

  public:

    /**
     * Reset the manager - discard the sources database,
     * do not store the changes to the persistent store.
     *
     * \throws Exception
     */
    void reset() ;

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
    Source_Ref findSource(SourceId id);

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
    std::list<SourceId> enabledSources() const;

    /**
     * Return ids of all sources
     */
    std::list<SourceId> allSources() const;

    /** Add a new source.
     * An attempt to add Source_Ref::noSource does nothing but
     * returning Source_Ref::noSource.numericId(). Thus it
     * results in adding no Source.
     */
    SourceId addSource(Source_Ref source_r);

    /** Remove an existing source by ID. */
    void removeSource(SourceId id);

    /** Remove an existing source by Alias. */
    void removeSource(const std::string & alias_r);

    /**
     * Release all medias held by all sources
     *
     * \throws Exception
     */
    void releaseAllSources();

    /**
     * Reattach all sources which are not mounted, but downloaded,
     * to different directory
     *
     * \throws Exception
     */
    void reattachSources(const Pathname & attach_point);

    /**
     * Disable all registered sources
     */
    void disableAllSources();

  private:
    /** Singleton ctor */
    SourceManager();
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates SourceManager Stream output */
  std::ostream & operator<<( std::ostream & str, const SourceManager & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCEMANAGER_H
