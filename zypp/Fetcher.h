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
#include <list>

#include "zypp/base/PtrTypes.h"
#include "zypp/Pathname.h"
#include "zypp/Url.h"
#include "zypp/OnMediaLocation.h"
#include "zypp/Digest.h"
#include "zypp/MediaSetAccess.h"
#include "zypp/FileChecker.h"
#include "zypp/ProgressData.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /**
  * This class allows to retrieve a group of files in a confortable
  * way, providing some smartness that does not belong to the
  * media layer like:
  *
  * \li Configurable local caches to retrieve already
  *     donwloaded files.
  * \li File checkers that can check for right checksums
  *     digital signatures, etc.
  *
  * \code
  * MediaSetAccess access(url, path);
  * Fetcher fetcher;
  * fetcher.enqueue( OnMediaLocation().filename("/content") );
  * fetcher.addCachePath("/tmp/cache")
  * fetcher.start( "/download-dir, access );
  * fetcher.reset();
  * \endcode
  *
  * To use the checkers. just create a functor implementing
  * bool operator()(const Pathname &file) \see FileChecker.
  * Pass the necessary validation data in the constructor
  * of the functor, and pass the object to the \ref enqueue
  * method.
  *
  * \code
  * ChecksumFileChecker checker(CheckSum("sha1", "....");
  * fetcher.enqueue( location, checker);
  * \endcode
  *
  * If you need to use more than one checker
  * \see CompositeFileChecker
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
    *
    */
    void enqueue( const OnMediaLocation &resource, const FileChecker &checker = NullFileChecker() );
    
    /**
    * Enqueue a object for transferal, they will not
    * be transfered until \ref start() is called
    *
    * \note As \ref OnMediaLocation contains the digest information,
    * a \ref ChecksumFileChecker is automatically added to the
    * transfer job, so make sure you don't add another one or
    * the user could be asked twice.
    *
    * \todo FIXME implement checker == operator to avoid this.
    */
    void enqueueDigested( const OnMediaLocation &resource, const FileChecker &checker = NullFileChecker() );
    
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
    void start( const Pathname &dest_dir,
                MediaSetAccess &media,
                 const ProgressData::ReceiverFnc & progress = ProgressData::ReceiverFnc() );

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
