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
    friend std::ostream & operator<<( std::ostream & str,
                                      const Fetcher & obj );

  public:
    /** Implementation  */
    class Impl;
  public:
    /** Default ctor */
    Fetcher();
    /** Dtor */
    virtual ~Fetcher();

  public:


   /**
    * Adds an index containing metadata (for example
    * checksums ) that will be retrieved and read
    * before the job processing starts.
    *
    * Nothing will be transfered or checked
    * until \ref start() is called.
    *
    * The index is relative to the media path, and
    * the listed files too.
    *
    */
    void addIndex( const OnMediaLocation &resource );
   

   /**
    * Enqueue a object for transferal, they will not
    * be transfered until \ref start() is called
    *
    */
    void enqueue( const OnMediaLocation &resource,
                  const FileChecker &checker = FileChecker() );
    
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
    void enqueueDigested( const OnMediaLocation &resource,
                          const FileChecker &checker = FileChecker() );


    /**
     * Enqueue a digested directory
     *
     * Directories are digested by providing a
     * SHA1SUMS file listing
     * <checksum> filename
     * and a respective SHA1SUMS.asc which has
     * the signature for the checksums.
     *
     * If you expect the user to not have the key of
     * the signature either in the trusted or untrusted
     * keyring, you can offer it as SHA1SUMS.key
     *
     * \param recursive True if the complete tree should
     * be enqueued. One SHA1SUMS is required per subdirectory
     *
     * \note As \ref checksums are read from SHA1SUMS,
     * a \ref ChecksumFileChecker is automatically added to every
     * transfer job, so make sure you don't add another one or
     * the user could be asked twice.
     *
     * \note The format of the file SHA1SUMS is the output of:
     * ls | grep -v SHA1SUMS | xargs sha1sum > SHA1SUMS
     * in each subdirectory.
     *
     * \note Every file appart of SHA1SUMS.(asc|key|(void)) will
     * not be transfered and will be ignored.
     *
     */
    void enqueueDir( const OnMediaLocation &resource,
                     bool recursive = false,
                     const FileChecker &checker = FileChecker() );
    
    /**
    * adds a directory to the list of directories
    * where to look for cached files
    */
    void addCachePath( const Pathname &cache_dir );
    
    /**
     * Reset the transfer (jobs) list
     * \note It does not reset the cache directory list
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
