/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/PathInfo.h
 *
*/
#ifndef ZYPP_PATHINFO_H
#define ZYPP_PATHINFO_H

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
}

#include <cerrno>
#include <iosfwd>
#include <list>
#include <set>
#include <map>

#include "zypp/Pathname.h"
#include "zypp/CheckSum.h"
#include "zypp/ByteCount.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class StrMatcher;

  ///////////////////////////////////////////////////////////////////
  /** Types and functions for filesystem operations.
   * \todo move zypp::filesystem stuff into separate header
   * \todo Add tmpfile and tmpdir handling.
   * \todo think about using Exceptions in zypp::filesystem
   * \todo provide a readdir iterator; at least provide an interface
   * using an insert_iterator to be independent from std::container.
  */
  namespace filesystem
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    /** File type information.
     * \todo Think about an \ref g_EnumerationClass
    */
    enum FileType
      {
        FT_NOT_AVAIL = 0x00, // no typeinfo available
        FT_NOT_EXIST = 0x01, // file does not exist
        FT_FILE      = 0x02,
        FT_DIR       = 0x04,
        FT_CHARDEV   = 0x08,
        FT_BLOCKDEV  = 0x10,
        FT_FIFO      = 0x20,
        FT_LINK      = 0x40,
        FT_SOCKET    = 0x80
      };
    ///////////////////////////////////////////////////////////////////

    /** \relates FileType Stram output. */
    extern std::ostream & operator<<( std::ostream & str, FileType obj );

    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : StatMode
    /**
     * @short Wrapper class for mode_t values as derived from ::stat
     **/
    class StatMode
    {
      friend std::ostream & operator<<( std::ostream & str, const StatMode & obj );

    public:
      /** Ctor taking  mode_t value from ::stat. */
      StatMode( const mode_t & mode_r = 0 )
      : _mode( mode_r )
      {}

    public:

      /** \name Query FileType. */
      //@{
      FileType fileType() const;

      bool   isFile()  const { return S_ISREG( _mode ); }
      bool   isDir ()  const { return S_ISDIR( _mode ); }
      bool   isLink()  const { return S_ISLNK( _mode ); }
      bool   isChr()   const { return S_ISCHR( _mode ); }
      bool   isBlk()   const { return S_ISBLK( _mode ); }
      bool   isFifo()  const { return S_ISFIFO( _mode ); }
      bool   isSock()  const { return S_ISSOCK( _mode ); }
      //@}

      /** \name Query user permissions. */
      //@{
      bool   isRUsr()  const { return (_mode & S_IRUSR); }
      bool   isWUsr()  const { return (_mode & S_IWUSR); }
      bool   isXUsr()  const { return (_mode & S_IXUSR); }

      /** Short for isRUsr().*/
      bool   isR()     const { return isRUsr(); }
      /** Short for isWUsr().*/
      bool   isW()     const { return isWUsr(); }
      /** Short for isXUsr().*/
      bool   isX()     const { return isXUsr(); }
      //@}

      /** \name Query group permissions. */
      //@{
      bool   isRGrp()  const { return (_mode & S_IRGRP); }
      bool   isWGrp()  const { return (_mode & S_IWGRP); }
      bool   isXGrp()  const { return (_mode & S_IXGRP); }
      //@}

      /** \name Query others permissions. */
      //@{
      bool   isROth()  const { return (_mode & S_IROTH); }
      bool   isWOth()  const { return (_mode & S_IWOTH); }
      bool   isXOth()  const { return (_mode & S_IXOTH); }
      //@}

      /** \name Query special permissions. */
      //@{
      /** Set UID bit. */
      bool   isUid()   const { return (_mode & S_ISUID); }
      /** Set GID bit. */
      bool   isGid()   const { return (_mode & S_ISGID); }
      /** Sticky bit. */
      bool   isVtx()   const { return (_mode & S_ISVTX); }
      //@}

      /** \name Query permission */
      //@{
      /** Test for equal permission bits. */
      bool   isPerm ( mode_t m ) const { return (m == perm()); }
      /** Test for set permission bits. */
      bool   hasPerm( mode_t m ) const { return (m == (m & perm())); }
      //@}

      /** \name Extract permission bits only. */
      //@{
      mode_t uperm()   const { return (_mode & S_IRWXU); }
      mode_t gperm()   const { return (_mode & S_IRWXG); }
      mode_t operm()   const { return (_mode & S_IRWXO); }
      mode_t perm()    const { return (_mode & (S_IRWXU|S_IRWXG|S_IRWXO|S_ISUID|S_ISGID|S_ISVTX)); }
      //@}

      /** Return the mode_t value. */
      mode_t st_mode() const { return _mode; }

    private:
      mode_t _mode;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates StatMode Stream output. */
    extern std::ostream & operator<<( std::ostream & str, const StatMode & obj );

    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : DevInoCache
    /** Simple cache remembering device/inode to detect hardlinks.
     * \code
     *     DevInoCache trace;
     *     for ( all files ) {
     *       if ( trace.insert( file.device, file.inode ) ) {
     *         // 1st occurance of file
     *       }
     *         // else: hardlink; already counted this device/inode
     *       }
     *     }
     * \endcode
     **/
    class DevInoCache
    {
    public:
      /** Ctor */
      DevInoCache() {}

      /** Clear cache. */
      void clear() { _devino.clear(); }

      /** Remember dev/ino.
       * \Return <code>true</code> if it's inserted the first
       * time, <code>false</code> if alredy present in cache
       * (a hardlink to a previously remembered file).
       **/
      bool insert( const dev_t & dev_r, const ino_t & ino_r ) {
        return _devino[dev_r].insert( ino_r ).second;
      }

    private:
      std::map<dev_t,std::set<ino_t> > _devino;
    };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : PathInfo
    /** Wrapper class for ::stat/::lstat.
     *
     * \note All attribute quieries test for isExist(), and return \c false or
     * \c 0, if stat was not successful.
     *
     * \note For convenience PathInfo is available as zypp::PathInfo too.
     **/
    class PathInfo
    {
      friend std::ostream & operator<<( std::ostream & str, const PathInfo & obj );

    public:
      /** stat() or lstat() */
      enum Mode { STAT, LSTAT };

    public:
      /** \name Construct from Pathname.
       * Default mode is \c STAT.
      */
      //@{
      PathInfo();
      explicit
      PathInfo( const Pathname & path, Mode initial = STAT );
      explicit
      PathInfo( const std::string & path, Mode initial = STAT );
      explicit
      PathInfo( const char * path, Mode initial = STAT );
      //@}

      /**Dtor */
      ~PathInfo();

      /** Return current Pathname. */
      const Pathname &    path()     const { return path_t; }
      /** Return current Pathname as String. */
      const std::string & asString() const { return path_t.asString(); }
      /** Return current Pathname as C-string. */
      const char * c_str()           const { return path_t.asString().c_str(); }
      /** Return current stat Mode. */
      Mode                mode()     const { return mode_e; }
      /** Return error returned from last stat/lstat call. */
      int                 error()    const { return error_i; }

      /** Set a new Pathname. */
      void setPath( const Pathname & path ) { if ( path != path_t ) error_i = -1; path_t = path; }
      /** Set a new Mode . */
      void setMode( Mode mode )             { if ( mode != mode_e ) error_i = -1; mode_e = mode; }

      /** STAT \a path. */
      bool stat      ( const Pathname & path ) { setPath( path ); setMode( STAT );  return operator()(); }
      /** LSTAT \a path. */
      bool lstat     ( const Pathname & path ) { setPath( path ); setMode( LSTAT ); return operator()(); }
      /** Restat \a path using current mode. */
      bool operator()( const Pathname & path ) { setPath( path ); return operator()(); }

      /** STAT current path. */
      bool stat()   { setMode( STAT );  return operator()(); }
      /** LSTAT current path. */
      bool lstat()  { setMode( LSTAT ); return operator()(); }
      /** Restat current path using current mode. */
      bool operator()();

    public:

      /** Return whether valid stat info exists.
       * That's usg. whether the file exist and you had permission to
       * stat it.
      */
      bool   isExist() const { return !error_i; }

      /** \name Query StatMode attibutes.
       * Combines \ref zypp::PathInfo::isExist and \ref zypp::filesystem::StatMode query.
      */
      //@{
      FileType fileType() const;

      bool   isFile()  const { return isExist() && S_ISREG( statbuf_C.st_mode ); }
      bool   isDir ()  const { return isExist() && S_ISDIR( statbuf_C.st_mode ); }
      bool   isLink()  const { return isExist() && S_ISLNK( statbuf_C.st_mode ); }
      bool   isChr()   const { return isExist() && S_ISCHR( statbuf_C.st_mode ); }
      bool   isBlk()   const { return isExist() && S_ISBLK( statbuf_C.st_mode ); }
      bool   isFifo()  const { return isExist() && S_ISFIFO( statbuf_C.st_mode ); }
      bool   isSock()  const { return isExist() && S_ISSOCK( statbuf_C.st_mode ); }

      // permission
      bool   isRUsr()  const { return isExist() && (statbuf_C.st_mode & S_IRUSR); }
      bool   isWUsr()  const { return isExist() && (statbuf_C.st_mode & S_IWUSR); }
      bool   isXUsr()  const { return isExist() && (statbuf_C.st_mode & S_IXUSR); }

      bool   isR()     const { return isRUsr(); }
      bool   isW()     const { return isWUsr(); }
      bool   isX()     const { return isXUsr(); }

      bool   isRGrp()  const { return isExist() && (statbuf_C.st_mode & S_IRGRP); }
      bool   isWGrp()  const { return isExist() && (statbuf_C.st_mode & S_IWGRP); }
      bool   isXGrp()  const { return isExist() && (statbuf_C.st_mode & S_IXGRP); }

      bool   isROth()  const { return isExist() && (statbuf_C.st_mode & S_IROTH); }
      bool   isWOth()  const { return isExist() && (statbuf_C.st_mode & S_IWOTH); }
      bool   isXOth()  const { return isExist() && (statbuf_C.st_mode & S_IXOTH); }

      bool   isUid()   const { return isExist() && (statbuf_C.st_mode & S_ISUID); }
      bool   isGid()   const { return isExist() && (statbuf_C.st_mode & S_ISGID); }
      bool   isVtx()   const { return isExist() && (statbuf_C.st_mode & S_ISVTX); }

      bool   isPerm ( mode_t m ) const { return isExist() && (m == perm()); }
      bool   hasPerm( mode_t m ) const { return isExist() && (m == (m & perm())); }

      mode_t uperm()   const { return isExist() ? (statbuf_C.st_mode & S_IRWXU) : 0; }
      mode_t gperm()   const { return isExist() ? (statbuf_C.st_mode & S_IRWXG) : 0; }
      mode_t operm()   const { return isExist() ? (statbuf_C.st_mode & S_IRWXO) : 0; }
      mode_t perm()    const { return isExist() ? (statbuf_C.st_mode & (S_IRWXU|S_IRWXG|S_IRWXO|S_ISUID|S_ISGID|S_ISVTX)) : 0; }

      mode_t st_mode() const { return isExist() ? statbuf_C.st_mode : 0; }
      //@}

      /** Return st_mode() as filesystem::StatMode. */
      StatMode asStatMode() const { return st_mode(); }

      nlink_t nlink()  const { return isExist() ? statbuf_C.st_nlink : 0; }

      /** \name Owner and group */
      //@{
      uid_t  owner()   const { return isExist() ? statbuf_C.st_uid : 0; }
      gid_t  group()   const { return isExist() ? statbuf_C.st_gid : 0; }
      //@}

      /** \name Permission according to current uid/gid. */
      //@{
      /** Returns current users permission (<tt>[0-7]</tt>)*/
      mode_t userMay() const;

      bool   userMayR() const { return( userMay() & 04 ); }
      bool   userMayW() const { return( userMay() & 02 ); }
      bool   userMayX() const { return( userMay() & 01 ); }

      bool   userMayRW()  const { return( (userMay() & 06) == 06 ); }
      bool   userMayRX()  const { return( (userMay() & 05) == 05 ); }
      bool   userMayWX()  const { return( (userMay() & 03) == 03 ); }

      bool   userMayRWX() const { return( userMay() == 07 ); }
      //@}

      /** \name Device and inode info. */
      //@{
      ino_t  ino()     const { return isExist() ? statbuf_C.st_ino  : 0; }
      dev_t  dev()     const { return isExist() ? statbuf_C.st_dev  : 0; }
      dev_t  rdev()    const { return isExist() ? statbuf_C.st_rdev : 0; }

      unsigned int devMajor() const;
      unsigned int devMinor() const;
      //@}

      /** \name Size info. */
      //@{
      off_t         size()    const { return isExist() ? statbuf_C.st_size : 0; }
      unsigned long blksize() const { return isExist() ? statbuf_C.st_blksize : 0; }
      unsigned long blocks()  const { return isExist() ? statbuf_C.st_blocks  : 0; }
      //@}

      /** \name Time stamps. */
      //@{
      time_t atime()   const { return isExist() ? statbuf_C.st_atime : 0; } /* time of last access */
      time_t mtime()   const { return isExist() ? statbuf_C.st_mtime : 0; } /* time of last modification */
      time_t ctime()   const { return isExist() ? statbuf_C.st_ctime : 0; }
      //@}

    private:
      Pathname    path_t;
      struct stat statbuf_C;
      Mode        mode_e;
      int         error_i;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates PathInfo Stream output. */
    extern std::ostream & operator<<( std::ostream & str, const PathInfo & obj );

    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    /** \name Directory related functions. */
    //@{
    /**
     * Like '::mkdir'. Attempt to create a new directory named path. mode
     * specifies the permissions to use. It is modified by the process's
     * umask in the usual way.
     *
     * @return 0 on success, errno on failure
     **/
    int mkdir( const Pathname & path, unsigned mode = 0755 );

    /**
     * Like 'mkdir -p'. No error if directory exists. Make parent directories
     * as needed. mode specifies the permissions to use, if directories have to
     * be created. It is modified by the process's umask in the usual way.
     *
     * @return 0 on success, errno on failure
     **/
    int assert_dir( const Pathname & path, unsigned mode = 0755 );

    /**
     * Like '::rmdir'. Delete a directory, which must be empty.
     *
     * @return 0 on success, errno on failure
     **/
    int rmdir( const Pathname & path );

    /**
     * Like 'rm -r DIR'. Delete a directory, recursively removing its contents.
     *
     * @return 0 on success, ENOTDIR if path is not a directory, otherwise the
     * commands return value.
     **/
    int recursive_rmdir( const Pathname & path );

    /**
     * Like 'rm -r DIR/ *'. Delete directory contents, but keep the directory itself.
     *
     * @return 0 on success, ENOTDIR if path is not a directory, otherwise the
     * commands return value.
     **/
    int clean_dir( const Pathname & path );

    /**
     * Like 'cp -a srcpath destpath'. Copy directory tree. srcpath/destpath must be
     * directories. 'basename srcpath' must not exist in destpath.
     *
     * @return 0 on success, ENOTDIR if srcpath/destpath is not a directory, EEXIST if
     * 'basename srcpath' exists in destpath, otherwise the commands return value.
     **/
    int copy_dir( const Pathname & srcpath, const Pathname & destpath );

    /**
     * Like 'cp -a srcpath/. destpath'. Copy the content of srcpath recursively
     * into destpath. Both \p srcpath and \p destpath has to exists.
     *
     * @return 0 on success, ENOTDIR if srcpath/destpath is not a directory,
     * EEXIST if srcpath and destpath are equal, otherwise the commands
     * return value.
     */
    int copy_dir_content( const Pathname & srcpath, const Pathname & destpath);

    /**
     * Convenience returning <tt>StrMatcher( "[^.]*", Match::GLOB )</tt>
     * \see \ref dirForEach
     */
    const StrMatcher & matchNoDots();

    /**
     * Invoke callback function \a fnc_r for each entry in directory \a dir_r.
     *
     * If \a fnc_r is a \c NULL function \c 0 is returned immediately without even
     * testing or accessing \a dir_r.
     *
     * Otherwise \c ::readdir is used to read the name of every entry in \a dir_r,
     * omitting  \c '.' and \c '..'. \a dir_r and the current entries name are passed
     * as arguments to \a fnc_r. If \a fnc_r returns \c false processing is aborted.
     *
     * @return 0 on success, -1 if aborted by callback, errno > 0 on ::readdir failure.
     */
    int dirForEach( const Pathname & dir_r, function<bool(const Pathname &, const char *const)> fnc_r );

    /**
     * \overload taking a \ref StrMatcher to filter the entries for which \a fnc_r is invoked.
     *
     * For convenience a \ref StrMatcher \ref matchNoDots is provided in this namespace.</tt>
     *
     * \code
     *   bool cbfnc( const Pathname & dir_r, const char *const str_r )
     *   {
     *     D BG <*< " - " << dir_r/str_r << endl;
     *     return true;
     *   }
     *   // Print no-dot files in "/tmp" via callback
     *   filesystem::dirForEach( "/tmp", filesystem::matchNoDots(), cbfnc );
     *
     *   // same via lambda
     *   filesystem::dirForEach( "/tmp", filesystem::matchNoDots(),
     *                           [](const Pathname & dir_r, const std::string & str_r)->bool
     *                           {
     *                             DBG << " - " << dir_r/str_r << endl;
     *                             return true;
     *                           });
     * \endcode
     */
    int dirForEach( const Pathname & dir_r, const StrMatcher & matcher_r, function<bool(const Pathname &, const char *const)> fnc_r );

    /**
     * Return content of directory via retlist. If dots is false
     * entries starting with '.' are not reported. "." and ".."
     * are never reported.
     *
     * Returns just the directory entries as string.
     *
     * @return 0 on success, errno on failure.
     *
     * \todo provide some readdirIterator.
     **/

    int readdir( std::list<std::string> & retlist,
                 const Pathname & path, bool dots = true );

    /**
     * Return content of directory via retlist. If dots is false
     * entries starting with '.' are not reported. "." and ".."
     * are never reported.
     *
     * Returns the directory entries prefixed with \a path.
     *
     * @return 0 on success, errno on failure.
     *
     * \todo provide some readdirIterator.
     **/

    int readdir( std::list<Pathname> & retlist,
                 const Pathname & path, bool dots = true );

    /** Listentry returned by readdir. */
    struct DirEntry {
      std::string name;
      FileType    type;
      DirEntry( const std::string & name_r = std::string(), FileType type_r = FT_NOT_AVAIL )
      : name( name_r )
      , type( type_r )
      {}

      bool operator==( const DirEntry &rhs ) const;
    };

    inline std::ostream & operator<<( std::ostream & str, const DirEntry & obj )
    { return str << '[' << obj.type << "] " << obj.name; }

    /** Returned by readdir. */
    typedef std::list<DirEntry> DirContent;

    std::ostream & operator<<( std::ostream & str, const DirContent & obj );

    /**
     * Return content of directory via retlist. If dots is false
     * entries starting with '.' are not reported. "." and ".."
     * are never reported.
     *
     * The type of individual directory entries is determined accoding to
     * statmode (i.e. via stat or lstat).
     *
     * @return 0 on success, errno on failure.
     **/
    int readdir( DirContent & retlist, const Pathname & path,
                 bool dots = true, PathInfo::Mode statmode = PathInfo::STAT );

    /**
     * Check if the specified directory is empty.
     * \param path The path of the directory to check.
     * \return 0 if directory is empty, -1 if not, errno > 0 on failure.
     */
    int is_empty_dir(const Pathname & path);

    //@}

    ///////////////////////////////////////////////////////////////////
    /** \name File related functions. */
    //@{
    /**
     * Create an empty file if it does not yet exist. Make parent directories
     * as needed. mode specifies the permissions to use. It is modified by the
     * process's umask in the usual way.
     *
     * @return 0 on success, errno on failure
     **/
    int assert_file( const Pathname & path, unsigned mode = 0644 );

    /**
     * Change file's modification and access times.
     *
     * \return 0 on success, errno on failure
     * \see man utime
     */
    int touch (const Pathname & path);

    /**
     * Like '::unlink'. Delete a file (symbolic link, socket, fifo or device).
     *
     * @return 0 on success, errno on failure
     **/
    int unlink( const Pathname & path );

    /**
     * Like '::rename'. Renames a file, moving it between directories if required.
     *
     * @return 0 on success, errno on failure
     **/
    int rename( const Pathname & oldpath, const Pathname & newpath );

    /** Exchanges two files or directories.
     *
     * Most common use is when building a new config file (or dir)
     * in a tempfile. After the job is done, configfile and tempfile
     * are exchanged. This includes moving away the configfile in case
     * the tempfile does not exist. Parent directories are created as
     * needed.
     *
     * \note Paths are exchanged using \c ::rename, so take care both paths
     * are located on the same filesystem.
     *
     * \code
     * Pathname configfile( "/etc/myconfig" );
     * TmpFile  newconfig( TmpFile::makeSibling( configfile ) );
     * // now write the new config:
     * std::ofstream o( newconfig.path().c_str() );
     * o << "mew values << endl;
     * o.close();
     * // If everything is fine, exchange the files:
     * exchange( newconfig.path(), configfile );
     * // Now the old configfile is still available at newconfig.path()
     * // until newconfig goes out of scope.
     * \endcode
     *
     * @return 0 on success, errno on failure
     */
    int exchange( const Pathname & lpath, const Pathname & rpath );

    /**
     * Like 'cp file dest'. Copy file to destination file.
     *
     * @return 0 on success, EINVAL if file is not a file, EISDIR if
     * destiantion is a directory, otherwise the commands return value.
     **/
    int copy( const Pathname & file, const Pathname & dest );

    /**
     * Like '::symlink'. Creates a symbolic link named newpath which contains
     * the string oldpath. If newpath exists it will not be overwritten.
     *
     * @return 0 on success, errno on failure.
     **/
    int symlink( const Pathname & oldpath, const Pathname & newpath );

    /**
     * Like '::link'. Creates a hard link named newpath to an existing file
     * oldpath. If newpath exists it will not be overwritten.
     *
     * @return 0 on success, errno on failure.
     **/
    int hardlink( const Pathname & oldpath, const Pathname & newpath );

    /**
     * Create \a newpath as hardlink or copy of \a oldpath.
     *
     * @return 0 on success, errno on failure.
     */
    int hardlinkCopy( const Pathname & oldpath, const Pathname & newpath );

    /**
     * Like '::readlink'. Return the contents of the symbolic link
     * \a symlink_r via \a target_r.
     *
     * @return 0 on success, errno on failure.
     */
    int readlink( const Pathname & symlink_r, Pathname & target_r );
    /** \overload Return an empty Pathname on error. */
    inline Pathname readlink( const Pathname & symlink_r )
    {
      Pathname target;
      readlink( symlink_r, target );
      return target;
    }

    /**
     * Recursively follows the symlink pointed to by \a path_r and returns
     * the Pathname to the real file or directory pointed to by the link.
     *
     * There is a recursion limit of 256 iterations to protect against a cyclic
     * link.
     *
     * @return Pathname of the file or directory pointed to by the given link
     *   if it is a valid link. If \a path_r is not a link, an exact copy of
     *   it is returned. If \a path_r is a broken or a cyclic link, an empty
     *   Pathname is returned and the event logged.
     */
    Pathname expandlink( const Pathname & path_r );

    /**
     * Like 'cp file dest'. Copy file to dest dir.
     *
     * @return 0 on success, EINVAL if file is not a file, ENOTDIR if dest
     * is no directory, otherwise the commands return value.
     **/
    int copy_file2dir( const Pathname & file, const Pathname & dest );
    //@}

    ///////////////////////////////////////////////////////////////////
    /** \name Digest computaion.
     * \todo check cooperation with zypp::Digest
    */
    //@{
    /**
     * Compute a files md5sum.
     *
     * @return the files md5sum on success, otherwise an empty string..
     **/
    std::string md5sum( const Pathname & file );

    /**
     * Compute a files sha1sum.
     *
     * @return the files sha1sum on success, otherwise an empty string..
     **/
    std::string sha1sum( const Pathname & file );
    //@}

    /**
     * Compute a files checksum
     *
     * @return the files checksum on success, otherwise an empty string..
     **/
    std::string checksum( const Pathname & file, const std::string &algorithm );

    /**
     * check files checksum
     *
     * @return true if the checksum matchs
     **/
    bool is_checksum( const Pathname & file, const CheckSum &checksum );

    ///////////////////////////////////////////////////////////////////
    /** \name Changing permissions. */
    //@{
    /**
     * Like '::chmod'. The mode of the file given by path is changed.
     *
     * @return 0 on success, errno on failure
     **/
    int chmod( const Pathname & path, mode_t mode );

    /**
     * Add the \c mode bits to the file given by path.
     *
     * @return 0 on success, errno on failure
     */
    int addmod( const Pathname & path, mode_t mode );

    /**
     * Remove the \c mode bits from the file given by path.
     *
     * @return 0 on success, errno on failure
     */
    int delmod( const Pathname & path, mode_t mode );
    //@}

    ///////////////////////////////////////////////////////////////////
    /** \name Misc. */
    //@{
    /**
     * Test whether a file is compressed (gzip/bzip2).
     *
     * @return ZT_GZ, ZT_BZ2 if file is compressed, otherwise ZT_NONE.
     **/
    enum ZIP_TYPE { ZT_NONE, ZT_GZ, ZT_BZ2 };

    ZIP_TYPE zipType( const Pathname & file );

    /**
     * Erase whatever happens to be located at path (file or directory).
     *
     * @return 0 on success.
     *
     * \todo check cooperation with zypp::TmpFile and zypp::TmpDir
     **/
    int erase( const Pathname & path );

    /**
     * Report free disk space on a mounted file system.
     *
     * path is the path name of any file within the mounted filesystem.
     *
     * @return Free disk space or -1 on error.
     **/
    ByteCount df( const Pathname & path );

    /**
     * Get the current umask (file mode creation mask)
     *
     * @return The current umask
     **/
    mode_t getUmask();

     /**
     * Modify \c mode_r according to the current umask
     * <tt>( mode_r & ~getUmask() )</tt>.
     * \see \ref getUmask.
     * @return The resulting permissions.
     **/
    inline mode_t applyUmaskTo( mode_t mode_r )
    { return mode_r & ~getUmask(); }
    //@}

    /////////////////////////////////////////////////////////////////
  } // namespace filesystem
  ///////////////////////////////////////////////////////////////////

  /** Dragged into namespace zypp. */
  using filesystem::PathInfo;

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PATHINFO_H
