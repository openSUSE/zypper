/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Fetcher.h
 *
*/
#ifndef ZYPP_FETCHER_H
#define ZYPP_FETCHER_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/Pathname.h"
#include "zypp/Url.h"
#include "zypp/OnMediaLocation.h"
#include "zypp/MediaSetAccess.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /**
  * This class allows to retrieve a group of files which can
  * be cached already on the local disk.
  *
  * \code
  * MediaSetAccess access(url, path);
  * Fetcher fetcher;
  * fetcher.enqueue( OnMediaLocation().filename("/content") );
  * fetcher.addCachePath("/tmp/cache")
  * fetcher.start( "/download-dir, access );
  * fetcher.reset();
  * \endcode
  */
  class Fetcher
  {
    friend std::ostream & operator<<( std::ostream & str, const Fetcher & obj );

  public:
    /** Implementation  */
    class Impl;

  public:
    /** Default ctor */
    Fetcher();
    /** Dtor */
    ~Fetcher();

  public:
   /**
    * Enqueue a object for transferal, they will not
    * be transfered until \ref start() is called
    */
    void enqueue( const OnMediaLocation &resource );
    /**
    * adds a directory to the list of directories
    * where to look for cached files
    */
    void addCachePath( const Pathname &cache_dir );
    /**
    * Reset the transfer list and cache list
    */
    void reset();
    /**
    * start the transfer to a destination directory
    * \a dest_dir
    * You have to provde a media set access
    * \a media to get the files from
    * The file tree will be replicated inside this
    * directory
    *
    */
    void start( const Pathname &dest_dir, MediaSetAccess &media );

  private:
    /** Pointer to implementation */
    RWCOW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Fetcher Stream output */
  std::ostream & operator<<( std::ostream & str, const Fetcher & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_FETCHER_H
