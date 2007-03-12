/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_FETCHER_H
#define ZYPP_FETCHER_H

#include <list>
#include "zypp/Pathname.h"
#include "zypp/Url.h"
#include "zypp/OnMediaLocation.h"

namespace zypp
{
  
  /** 
  * Edition represents <code>[epoch:]version[-release]</code>
  *
  * This class allows to retrieve a group of files which can
  * be cached already on the local disk.
  *
  * \code
  * Fetcher fetcher(url, path);
  * fetcher.enqueue( OnMediaLocation().filename("/content") );
  * fetcher.insertCache("/tmp/cache")
  * fetcher.start( "/download-dir );
  * fetcher.reset();
  * \endcode
  */
  class Fetcher
  {
  public:
    /**
    * Constructs a fetcher from a url and path
    */
    Fetcher( const Url &url, const Pathname &path );
    
    /**
    * Enqueue a object for transferal, they will not
    * be transfered until \ref start() is called
    */
    void enqueue( const OnMediaLocation &resource );
    /**
    * adds a directory to the list of directories
    * where to look for cached files
    */
    void insertCache( const Pathname &cache_dir );
    /**
    * Reset the transfer list and cache list
    */
    void reset();
    /**
    * start the transfer to a destination directory
    * The file tree will be replicated inside this
    * directory
    */
    void start( const Pathname &dest_dir );
  
  private:
    Url _url;
    Pathname _path;
    std::list<OnMediaLocation> _resources;
    std::list<Pathname> _caches;
  };

} // ns zypp
#endif