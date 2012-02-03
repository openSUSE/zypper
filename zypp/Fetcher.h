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

#include "zypp/base/Flags.h"
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
  * fetcher.enqueue( OnMediaLocation().setLocation("/somefile") );
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
  *
  * Additionally, you can automatically enqueue a job
  * with a checksum checker by using \ref enqueueDigested
  * which will use the \ref OnMediaLocation checksum
  * automatically.
  *
  * \code
  * location.setChecksum(CheckSum("sha1", "...."));
  * fetcher.enqueueDigested(location);
  * \endcode
  *
  * \note If the checksum of the location is empty, but
  * \ref enqueueDigested is used, then the user will get a
  * warning that the file has no checksum.
  *
  * Additionally, Fetcher supports checking the downloaded
  * content by using signed indexes on the remote side.
  *
  * \code
  * MediaSetAccess access(url, path);
  * Fetcher fetcher;
  * fetcher.addIndex(OnMediaLocation("/content"));
  * fetcher.enqueue( OnMediaLocation().setLocation("/somefile") );
  * fetcher.start( "/download-dir, access );
  * fetcher.reset();
  * \endcode
  *
  * Indexes are supported in CHECKSUMS format (simple text file)
  * with checksum and file name, or content file, whith
  * HASH SHA1 line entries.
  *
  * \note The indexes file names are relative to the directory
  * where the index is.
  *
  * \note libzypp-11.x: Introduction of sha256 lead to the insight
  * that using SHA1SUMS as filename was a bad choice. New media store
  * the checksums in a CHECKSUMS file. If a CHECKSUMS file is not
  * present, we fall back looking for a SHA1SUMS file. The checksum
  * type (md5,sha1,sha256) is auto detected by looking at the cheksums
  * length. No need to somehow encode it in the filename.
  */
  class Fetcher
  {
    friend std::ostream & operator<<( std::ostream & str,
                                      const Fetcher & obj );
  public:
    /** Implementation  */
    class Impl;
  public:

    /**
     * Various option flags to change behavior
     */
    enum Option
    {
      /**
       * If a content file is found, it is
       * downloaded and read.
       */
      AutoAddContentFileIndexes = 0x0001,
      /**
       * If a CHECKSUMS file is found, it is
       * downloaded and read.
       */
      AutoAddChecksumsIndexes = 0x0002,
      /**
       * If a content or CHECKSUMS file is found,
       * it is downloaded and read.
       */
      AutoAddIndexes = AutoAddContentFileIndexes | AutoAddChecksumsIndexes,
    };
    ZYPP_DECLARE_FLAGS(Options, Option);

    /** Default ctor */
    Fetcher();
    /** Dtor */
    virtual ~Fetcher();

  public:

   /**
    * Set the Fetcher options
    * \see Fetcher::Options
    */
    void setOptions( Options options );

   /**
    * Get current options
    * \see Fetcher::Options
    */
    Options options() const;

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
    * Indexes in the SHA1SUM format, and YaST
    * content file
    *
    * The file has to be signed or the user will be
    * warned that the file is unsigned. You can
    * place the signature next to the file adding the
    * .asc extension.
    *
    * If you expect the key to not to be in the target
    * system, then you can place it next to the index
    * using adding the .key extension.
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
    *
    * the optional deltafile argument describes a file that can
    * be used for delta download algorithms. Usable files are
    * uncompressed files or files compressed with gzip --rsyncable.
    * Other files like rpms do not work, as the compression
    * breaks the delta algorithm.
    */
    void enqueueDigested( const OnMediaLocation &resource,
                          const FileChecker &checker = FileChecker(), const Pathname &deltafile = Pathname());


    /**
     * Enqueue a directory
     *
     * As the files to be enqueued are not known
     * in advance, all files whose checksum can
     * be found in some index are enqueued with
     * checksum checking. Otherwise they are not.
     *
     * Some index may provide
     * the checksums, either by \ref addIndex or
     * using \ref AutoAddIndexes flag.
     *
     * Files are checked by providing a
     * CHECKSUMS or content file listing
     * <checksum> filename
     * and a respective CHECKSUMS.asc/content.asc which has
     * the signature for the checksums.
     *
     * If you expect the user to not have the key of
     * the signature either in the trusted or untrusted
     * keyring, you can offer it as CHECKSUMS.key (or content.key)
     *
     * \param recursive True if the complete tree should
     * be enqueued.
     *
     * \note As \ref checksums are read from the index,
     * a \ref ChecksumFileChecker is automatically added to
     * transfer jobs having a checksum available,
     * so make sure you don't add another one or
     * the user could be asked twice.
     *
     * \note The format of the file CHECKSUMS is the output of:
     * ls | grep -v CHECKSUMS | xargs sha256sum > CHECKSUMS
     * in each subdirectory.
     *
     * \note Every file CHECKSUMS.* except of CHECKSUMS.(asc|key|(void)) will
     * not be transfered and will be ignored.
     *
     */
    void enqueueDir( const OnMediaLocation &resource,
                     bool recursive = false,
                     const FileChecker &checker = FileChecker() );

    /**
     * Enqueue a directory and always check for
     * checksums.
     *
     * As the files to be enqueued are not known
     * in advance, all files are enqueued with
     * checksum checking. If the checksum of some file is
     * not in some index, then there will be a verification
     * warning ( \ref DigestReport ).
     *
     * Therefore some index will need to provide
     * the checksums, either by \ref addIndex or
     * using \ref AutoAddIndexes flag.
     *
     * Files are checked by providing a
     * CHECKSUMS or content file listing
     * <checksum> filename
     * and a respective CHECKSUMS.asc/content.asc which has
     * the signature for the checksums.
     *
     * If you expect the user to not have the key of
     * the signature either in the trusted or untrusted
     * keyring, you can offer it as CHECKSUMS.key (or content.key)
     *
     * \param recursive True if the complete tree should
     * be enqueued.
     *
     * \note As \ref checksums are read from the index,
     * a \ref ChecksumFileChecker is automatically added to every
     * transfer job, so make sure you don't add another one or
     * the user could be asked twice.
     *
     * \note The format of the file CHECKSUMS is the output of:
     * ls | grep -v CHECKSUMS | xargs sha256sum > CHECKSUMS
     * in each subdirectory.
     *
     * \note Every file CHECKSUMS.* except of CHECKSUMS.(asc|key|(void)) will
     * not be transfered and will be ignored.
     *
     */
    void enqueueDigestedDir( const OnMediaLocation &resource,
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
  ZYPP_DECLARE_OPERATORS_FOR_FLAGS(Fetcher::Options);

  /** \relates Fetcher Stream output */
  std::ostream & operator<<( std::ostream & str, const Fetcher & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_FETCHER_H
