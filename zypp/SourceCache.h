/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/SourceCache.h
 *
*/
#ifndef ZYPP_SOURCECACHE_H
#define ZYPP_SOURCECACHE_H

#include <iosfwd>
#include <string>
#include <set>

#include "zypp/base/PtrTypes.h"

#include "zypp/Source.h"
#include "zypp/Url.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : SourceCache
  //
  //	singleton
  //
  class SourceCache
  {
    friend std::ostream & operator<<( std::ostream & str, const SourceCache & obj );

  public:
    /** Default ctor */
    SourceCache();
    /** Dtor */
    ~SourceCache();
    
    void setCacheDir(const Pathname& dir_r);

  public:
    void storeSource(Source_Ref src);

    void restoreSources();

    void removeSource(unsigned id);

    void removeSource(const Url & url_r, const Pathname & path_r = "/");

  private:
    /** directory to store cached data */
    static Pathname _cache_dir;
    /** counter of caches */
    static unsigned _next_cache_id;


  ///////////////////////////////////////////////////////////////////
  };

  /** \relates SourceCache Stream output */
  extern std::ostream & operator<<( std::ostream & str, const SourceCache & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCECACHE_H
