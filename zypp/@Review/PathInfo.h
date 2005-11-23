/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       PathInfo.h

   Author:     Michael Andres <ma@suse.de>
   Maintainer: Michael Andres <ma@suse.de>

/-*/
#ifndef PathInfo_h
#define PathInfo_h

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

#include <y2util/Pathname.h>

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : PathInfo
/**
 * @short Wrapper class for ::stat/::lstat and other file/directory related operations.
 **/
class PathInfo {

  friend std::ostream & operator<<( std::ostream & str, const PathInfo & obj );

  public:

    enum Mode { STAT, LSTAT };

    enum file_type {
      NOT_AVAIL  = 0x00, // no typeinfo available
      NOT_EXIST  = 0x01, // file does not exist
      T_FILE     = 0x02,
      T_DIR      = 0x04,
      T_CHARDEV  = 0x08,
      T_BLOCKDEV = 0x10,
      T_FIFO     = 0x20,
      T_LINK     = 0x40,
      T_SOCKET   = 0x80
    };
    friend std::ostream & operator<<( std::ostream & str, file_type obj );

    /**
     * Wrapper class for mode_t values as derived from ::stat
     **/
    class stat_mode;

    /**
     * Simple cache remembering device/inode to detect hardlinks.
     **/
    class devino_cache;

  private:

    Pathname    path_t;

    struct stat statbuf_C;
    Mode        mode_e;
    int         error_i;

  public:

    PathInfo( const Pathname & path = "", Mode initial = STAT );
    PathInfo( const std::string & path, Mode initial = STAT );
    PathInfo( const char * path, Mode initial = STAT );
    virtual ~PathInfo();

    const Pathname &    path()     const { return path_t; }
    const std::string & asString() const { return path_t.asString(); }
    Mode                mode()     const { return mode_e; }
    int                 error()    const { return error_i; }

    void setPath( const Pathname & path ) { if ( path != path_t ) error_i = -1; path_t = path; }
    void setMode( Mode mode )             { if ( mode != mode_e ) error_i = -1; mode_e = mode; }

    bool stat      ( const Pathname & path ) { setPath( path ); setMode( STAT );  return operator()(); }
    bool lstat     ( const Pathname & path ) { setPath( path ); setMode( LSTAT ); return operator()(); }
    bool operator()( const Pathname & path ) { setPath( path ); return operator()(); }

    bool stat()   { setMode( STAT );  return operator()(); }
    bool lstat()  { setMode( LSTAT ); return operator()(); }
    bool operator()();

  public:

    bool   isExist() const { return !error_i; }

    // file type
    file_type fileType() const;

    bool   isFile()  const { return isExist() && S_ISREG( statbuf_C.st_mode ); }
    bool   isDir ()  const { return isExist() && S_ISDIR( statbuf_C.st_mode ); }
    bool   isLink()  const { return isExist() && S_ISLNK( statbuf_C.st_mode ); }
    bool   isChr()   const { return isExist() && S_ISCHR( statbuf_C.st_mode ); }
    bool   isBlk()   const { return isExist() && S_ISBLK( statbuf_C.st_mode ); }
    bool   isFifo()  const { return isExist() && S_ISFIFO( statbuf_C.st_mode ); }
    bool   isSock()  const { return isExist() && S_ISSOCK( statbuf_C.st_mode ); }

    nlink_t nlink()  const { return isExist() ? statbuf_C.st_nlink : 0; }

    // owner
    uid_t  owner()   const { return isExist() ? statbuf_C.st_uid : 0; }
    gid_t  group()   const { return isExist() ? statbuf_C.st_gid : 0; }

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

    mode_t uperm()   const { return isExist() ? (statbuf_C.st_mode & S_IRWXU) : 0; }
    mode_t gperm()   const { return isExist() ? (statbuf_C.st_mode & S_IRWXG) : 0; }
    mode_t operm()   const { return isExist() ? (statbuf_C.st_mode & S_IRWXO) : 0; }
    mode_t perm()    const { return isExist() ? (statbuf_C.st_mode & (S_IRWXU|S_IRWXG|S_IRWXO|S_ISUID|S_ISGID|S_ISVTX)) : 0; }

    bool   isPerm ( mode_t m ) const { return (m == perm()); }
    bool   hasPerm( mode_t m ) const { return (m == (m & perm())); }

    mode_t st_mode() const { return isExist() ? statbuf_C.st_mode : 0; }

    // permission according to current uid/gid (returns [0-7])
    mode_t userMay() const;

    bool   userMayR() const { return( userMay() & 01 ); }
    bool   userMayW() const { return( userMay() & 02 ); }
    bool   userMayX() const { return( userMay() & 04 ); }

    bool   userMayRW()  const { return( (userMay() & 03) == 03 ); }
    bool   userMayRX()  const { return( (userMay() & 05) == 05 ); }
    bool   userMayWX()  const { return( (userMay() & 06) == 06 ); }

    bool   userMayRWX() const { return( userMay() == 07 ); }

    // device
    dev_t  dev()     const { return isExist() ? statbuf_C.st_dev  : 0; }
    dev_t  rdev()    const { return isExist() ? statbuf_C.st_rdev : 0; }
    ino_t  ino()     const { return isExist() ? statbuf_C.st_ino  : 0; }

    // size
    off_t         size()    const { return isExist() ? statbuf_C.st_size : 0; }
    unsigned long blksize() const { return isExist() ? statbuf_C.st_blksize : 0; }
    unsigned long blocks()  const { return isExist() ? statbuf_C.st_blocks  : 0; }

    // time
    time_t atime()   const { return isExist() ? statbuf_C.st_atime : 0; } /* time of last access */
    time_t mtime()   const { return isExist() ? statbuf_C.st_mtime : 0; } /* time of last modification */
    time_t ctime()   const { return isExist() ? statbuf_C.st_ctime : 0; }

  public:

    ///////////////////////////////////////////////////////////////////
    // convenience stuff
    ///////////////////////////////////////////////////////////////////
    // static functions as they may or may not invalidate any stat info
    // stored by a PathiInfo.
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    // Directories
    ///////////////////////////////////////////////////////////////////

    /**
     * Like '::mkdir'. Attempt to create a new directory named path. mode
     * specifies the permissions to use. It is modified by the process's
     * umask in the usual way.
     *
     * @return 0 on success, errno on failure
     **/
    static int mkdir( const Pathname & path, unsigned mode = 0755 );

    /**
     * Like 'mkdir -p'. No error if directory exists. Make parent directories
     * as needed. mode specifies the permissions to use, if directories have to
     * be created. It is modified by the process's umask in the usual way.
     *
     * @return 0 on success, errno on failure
     **/
    static int assert_dir( const Pathname & path, unsigned mode = 0755 );

    /**
     * Like '::rmdir'. Delete a directory, which must be empty.
     *
     * @return 0 on success, errno on failure
     **/
    static int rmdir( const Pathname & path );

    /**
     * Like 'rm -r DIR'. Delete a directory, recursively removing its contents.
     *
     * @return 0 on success, ENOTDIR if path is not a directory, otherwise the
     * commands return value.
     **/
    static int recursive_rmdir( const Pathname & path );

    /**
     * Like 'rm -r DIR/ *'. Delete directory contents, but keep the directory itself.
     *
     * @return 0 on success, ENOTDIR if path is not a directory, otherwise the
     * commands return value.
     **/
    static int clean_dir( const Pathname & path );

    /**
     * Like 'cp -a srcpath destpath'. Copy directory tree. srcpath/destpath must be
     * directories. 'basename srcpath' must not exist in destpath.
     *
     * @return 0 on success, ENOTDIR if srcpath/destpath is not a directory, EEXIST if
     * 'basename srcpath' exists in destpath, otherwise the commands return value.
     **/
    static int copy_dir( const Pathname & srcpath, const Pathname & destpath );

    /**
     * Return content of directory via retlist. If dots is false
     * entries starting with '.' are not reported. "." and ".."
     * are never reported.
     *
     * @return 0 on success, errno on failure.
     **/
    static int readdir( std::list<std::string> & retlist,
			const Pathname & path, bool dots = true );

    struct direntry {
      std::string name;
      file_type   type;
      direntry( const std::string & name_r = std::string(), file_type type_r = NOT_AVAIL )
	: name( name_r )
	, type( type_r )
      {}
    };

    typedef std::list<direntry> dircontent;

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
    static int readdir( dircontent & retlist, const Pathname & path,
			bool dots = true, Mode statmode = STAT );

    ///////////////////////////////////////////////////////////////////
    // Files
    ///////////////////////////////////////////////////////////////////

    /**
     * Like '::unlink'. Delete a file (symbolic link, socket, fifo or device).
     *
     * @return 0 on success, errno on failure
     **/
    static int unlink( const Pathname & path );

    /**
     * Like '::rename'. Renames a file, moving it between directories if required.
     *
     * @return 0 on success, errno on failure
     **/
    static int rename( const Pathname & oldpath, const Pathname & newpath );

    /**
     * Like 'cp file dest'. Copy file to destination file.
     *
     * @return 0 on success, EINVAL if file is not a file, EISDIR if
     * destiantion is a directory, otherwise the commands return value.
     **/
    static int copy( const Pathname & file, const Pathname & dest );

    /**
     * Like '::symlink'. Creates a symbolic link named newpath which contains
     * the string oldpath. If newpath exists it will not be overwritten.
     *
     * @return 0 on success, errno on failure.
     **/
    static int symlink( const Pathname & oldpath, const Pathname & newpath );

    /**
     * Like '::link'. Creates a hard link named newpath to an existing file
     * oldpath. If newpath exists it will not be overwritten.
     *
     * @return 0 on success, errno on failure.
     **/
    static int hardlink( const Pathname & oldpath, const Pathname & newpath );

    /**
     * Like 'cp file dest'. Copy file to dest dir.
     *
     * @return 0 on success, EINVAL if file is not a file, ENOTDIR if dest
     * is no directory, otherwise the commands return value.
     **/
    static int copy_file2dir( const Pathname & file, const Pathname & dest );

    /**
     * Compute a files md5sum.
     *
     * @return the files md5sum on success, otherwise an empty string..
     **/
    static std::string md5sum( const Pathname & file );

    /**
     * Compute a files sha1sum.
     *
     * @return the files sha1sum on success, otherwise an empty string..
     **/
    static std::string sha1sum( const Pathname & file );

    ///////////////////////////////////////////////////////////////////
    //
    ///////////////////////////////////////////////////////////////////

    /**
     * Erase whatever happens to be located at path (file or directory).
     *
     * @return 0 on success.
     **/
    static int erase( const Pathname & path );

    ///////////////////////////////////////////////////////////////////
    // permissions
    ///////////////////////////////////////////////////////////////////

    /**
     * Like '::chmod'. The mode of the file given by path is changed.
     *
     * @return 0 on success, errno on failure
     **/
    static int chmod( const Pathname & path, mode_t mode );

    ///////////////////////////////////////////////////////////////////
    // magic
    ///////////////////////////////////////////////////////////////////

    /**
     * Test whether a file is compressed (gzip/bzip2).
     *
     * @return ZT_GZ, ZT_BZ2 if file is compressed, otherwise ZT_NONE.
     **/
    enum ZIP_TYPE { ZT_NONE, ZT_GZ, ZT_BZ2 };

    static ZIP_TYPE zipType( const Pathname & file );
};

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : PathInfo::stat_mode
/**
 * @short Wrapper class for mode_t values as derived from ::stat
 **/
class PathInfo::stat_mode {
  friend std::ostream & operator<<( std::ostream & str, const stat_mode & obj );
  private:
    mode_t _mode;
  public:
    stat_mode( const mode_t & mode_r = 0 ) : _mode( mode_r ) {}
  public:
    // file type
    file_type fileType() const;

    bool   isFile()  const { return S_ISREG( _mode ); }
    bool   isDir ()  const { return S_ISDIR( _mode ); }
    bool   isLink()  const { return S_ISLNK( _mode ); }
    bool   isChr()   const { return S_ISCHR( _mode ); }
    bool   isBlk()   const { return S_ISBLK( _mode ); }
    bool   isFifo()  const { return S_ISFIFO( _mode ); }
    bool   isSock()  const { return S_ISSOCK( _mode ); }

    // permission
    bool   isRUsr()  const { return (_mode & S_IRUSR); }
    bool   isWUsr()  const { return (_mode & S_IWUSR); }
    bool   isXUsr()  const { return (_mode & S_IXUSR); }

    bool   isR()     const { return isRUsr(); }
    bool   isW()     const { return isWUsr(); }
    bool   isX()     const { return isXUsr(); }

    bool   isRGrp()  const { return (_mode & S_IRGRP); }
    bool   isWGrp()  const { return (_mode & S_IWGRP); }
    bool   isXGrp()  const { return (_mode & S_IXGRP); }

    bool   isROth()  const { return (_mode & S_IROTH); }
    bool   isWOth()  const { return (_mode & S_IWOTH); }
    bool   isXOth()  const { return (_mode & S_IXOTH); }

    bool   isUid()   const { return (_mode & S_ISUID); }
    bool   isGid()   const { return (_mode & S_ISGID); }
    bool   isVtx()   const { return (_mode & S_ISVTX); }

    mode_t uperm()   const { return (_mode & S_IRWXU); }
    mode_t gperm()   const { return (_mode & S_IRWXG); }
    mode_t operm()   const { return (_mode & S_IRWXO); }
    mode_t perm()    const { return (_mode & (S_IRWXU|S_IRWXG|S_IRWXO|S_ISUID|S_ISGID|S_ISVTX)); }

    bool   isPerm ( mode_t m ) const { return (m == perm()); }
    bool   hasPerm( mode_t m ) const { return (m == (m & perm())); }

    mode_t st_mode() const { return _mode; }
};

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : PathInfo::devino_cache
/**
 * @short Simple cache remembering device/inode to detect hardlinks.
 * <pre>
 *     PathInfo::devino_cache trace;
 *     for ( all files ) {
 *       if ( trace.insert( file.device, file.inode ) ) {
 *         // 1st occurance of file
 *       }
 *         // else: hardlink; already counted this device/inode
 *       }
 *     }
 * </pre>
 **/
class PathInfo::devino_cache {

  private:

    std::map<dev_t,std::set<ino_t> > _devino;

  public:
    /**
     * Constructor
     **/
    devino_cache() {}

    /**
     * Clear cache
     **/
    void clear() { _devino.clear(); }

    /**
     * Remember dev/ino. Return <code>true</code> if it's inserted the first
     * time, <code>false</code> if alredy present in cache (a hardlink to a
     * previously remembered file.
     **/
    bool insert( const dev_t & dev_r, const ino_t & ino_r ) {
      return _devino[dev_r].insert( ino_r ).second;
    }
};

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////

#endif // PathInfo_h
