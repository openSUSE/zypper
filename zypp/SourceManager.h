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
    unsigned addSource(const Url & url_r, const Pathname & path_r = "/");

    unsigned removeSource(const unsigned id);

    const Pathname provideFile(const unsigned id,
			       const unsigned media_nr,
			       const Pathname & path_r);

    const Pathname provideDir(const unsigned id,
			      const unsigned media_nr,
			      const Pathname & path_r,
			      const bool recursive = false);

  private:
    typedef std::map<unsigned, RW_pointer<Source> > SourceMap;
    SourceMap _sources;

    RW_pointer<Source> findSource(const unsigned id);

    static unsigned _next_id;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates SourceManager Stream output */
  std::ostream & operator<<( std::ostream & str, const SourceManager & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCEMANAGER_H
