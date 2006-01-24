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

#include "zypp/Source.h"
#include "zypp/Url.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SourceManager
  //
  /** Manage a ResObject pool. */
  class SourceManager
  {
    friend std::ostream & operator<<( std::ostream & str, const SourceManager & obj );


  public:
    /** Default ctor */
    SourceManager();
    /** Dtor */
    ~SourceManager();

  public:
    /**
     * \throws Exception
     */
    unsigned addSource(const Url & url_r, const Pathname & path_r = "/");

    /**
     * \throws Exception
     */
    void removeSource(const unsigned id);

    /**
     * \throws Exception
     */
    const Pathname provideFile(const unsigned id,
			       const unsigned media_nr,
			       const Pathname & path_r);

    /**
     * \throws Exception
     */
    const Pathname provideDir(const unsigned id,
			      const unsigned media_nr,
			      const Pathname & path_r,
			      const bool recursive = false);

    /**
     * \throws Exception
     */
    const bool enabled(const unsigned id) const;

    /**
     * \throws Exception
     */
    void enable(const unsigned id);

    /**
     * \throws Exception
     */
    void disable(const unsigned id);

  private:
    typedef std::map<unsigned, RW_pointer<Source> > SourceMap;
    SourceMap _sources;

    /**
     * \throws Exception
     */
    RW_pointer<Source> findSource(const unsigned id) const;

    static unsigned _next_id;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates SourceManager Stream output */
  std::ostream & operator<<( std::ostream & str, const SourceManager & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCEMANAGER_H
