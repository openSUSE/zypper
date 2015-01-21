/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/PathInfo.cc
 *
*/

#include <sys/types.h> // for ::minor, ::major macros
#include <utime.h>     // for ::utime
#include <sys/statvfs.h>

#include <iostream>
#include <fstream>
#include <iomanip>

#include "zypp/base/LogTools.h"
#include "zypp/base/String.h"
#include "zypp/base/IOStream.h"
#include "zypp/base/StrMatcher.h"
#include "zypp/base/Errno.h"

#include "zypp/AutoDispose.h"
#include "zypp/ExternalProgram.h"
#include "zypp/PathInfo.h"
#include "zypp/Digest.h"
#include "zypp/TmpPath.h"

using std::endl;
using std::string;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace filesystem
  { /////////////////////////////////////////////////////////////////

    /******************************************************************
     **
     **	FUNCTION NAME : operator<<
     **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, FileType obj )
    {
      switch ( obj ) {
#define EMUMOUT(T) case T: return str << #T; break
        EMUMOUT( FT_NOT_AVAIL );
        EMUMOUT( FT_NOT_EXIST );
        EMUMOUT( FT_FILE );
        EMUMOUT( FT_DIR );
        EMUMOUT( FT_CHARDEV );
        EMUMOUT( FT_BLOCKDEV );
        EMUMOUT( FT_FIFO );
        EMUMOUT( FT_LINK );
        EMUMOUT( FT_SOCKET );
#undef EMUMOUT
      }
      return str;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : StatMode::fileType
    //	METHOD TYPE : FileType
    //
    FileType StatMode::fileType() const
    {
      if ( isFile() )
        return FT_FILE;
      if ( isDir() )
        return FT_DIR;
      if ( isLink() )
        return FT_LINK;
      if ( isChr() )
        return FT_CHARDEV;
      if ( isBlk() )
        return FT_BLOCKDEV;
      if ( isFifo() )
        return FT_FIFO;
      if ( isSock() )
        return FT_SOCKET ;

      return FT_NOT_AVAIL;
    }

    /******************************************************************
     **
     **	FUNCTION NAME : operator<<
     **	FUNCTION TYPE : std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const StatMode & obj )
    {
      iostr::IosFmtFlagsSaver autoResoreState( str );

      char t = '?';
      if ( obj.isFile() )
        t = '-';
      else if ( obj.isDir() )
        t = 'd';
      else if ( obj.isLink() )
        t = 'l';
      else if ( obj.isChr() )
        t = 'c';
      else if ( obj.isBlk() )
        t = 'b';
      else if ( obj.isFifo() )
        t = 'p';
      else if ( obj.isSock() )
        t = 's';

      str << t << " " << std::setfill( '0' ) << std::setw( 4 ) << std::oct << obj.perm();
      return str;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	Class : PathInfo
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : PathInfo::PathInfo
    //	METHOD TYPE : Constructor
    //
    PathInfo::PathInfo()
    : mode_e( STAT )
    , error_i( -1 )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : PathInfo::PathInfo
    //	METHOD TYPE : Constructor
    //
    PathInfo::PathInfo( const Pathname & path, Mode initial )
    : path_t( path )
    , mode_e( initial )
    , error_i( -1 )
    {
      operator()();
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : PathInfo::PathInfo
    //	METHOD TYPE : Constructor
    //
    PathInfo::PathInfo( const std::string & path, Mode initial )
    : path_t( path )
    , mode_e( initial )
    , error_i( -1 )
    {
      operator()();
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : PathInfo::PathInfo
    //	METHOD TYPE : Constructor
    //
    PathInfo::PathInfo( const char * path, Mode initial )
    : path_t( path )
    , mode_e( initial )
    , error_i( -1 )
    {
      operator()();
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : PathInfo::~PathInfo
    //	METHOD TYPE : Destructor
    //
    PathInfo::~PathInfo()
    {
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : PathInfo::operator()
    //	METHOD TYPE : bool
    //
    bool PathInfo::operator()()
    {
      if ( path_t.empty() ) {
        error_i = -1;
      } else {
        switch ( mode_e ) {
        case STAT:
          error_i = ::stat( path_t.asString().c_str(), &statbuf_C );
          break;
        case LSTAT:
          error_i = ::lstat( path_t.asString().c_str(), &statbuf_C );
          break;
        }
        if ( error_i == -1 )
          error_i = errno;
      }
      return !error_i;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : PathInfo::fileType
    //	METHOD TYPE : File_type
    //
    FileType PathInfo::fileType() const
    {
      if ( isExist() )
        return asStatMode().fileType();
      return FT_NOT_EXIST;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : PathInfo::userMay
    //	METHOD TYPE : mode_t
    //
    mode_t PathInfo::userMay() const
    {
      if ( !isExist() )
        return 0;
      if ( owner() == geteuid() ) {
        return( uperm()/0100 );
      } else if ( group() == getegid() ) {
        return( gperm()/010 );
      }
      return operm();
    }

    /******************************************************************
     **
     **	FUNCTION NAME : PathInfo::devMajor
     **	FUNCTION TYPE : unsigned int
     */
    unsigned int PathInfo::devMajor() const
    {
      return isBlk() || isChr() ? major(statbuf_C.st_rdev) : 0;
    }

    /******************************************************************
     **
     **	FUNCTION NAME : PathInfo::devMinor
     **	FUNCTION TYPE : unsigned int
     */
    unsigned int PathInfo::devMinor() const
    {
      return isBlk() || isChr() ? minor(statbuf_C.st_rdev) : 0;
    }

    /******************************************************************
     **
     **	FUNCTION NAME : operator<<
     **	FUNCTION TYPE :  std::ostream &
    */
    std::ostream & operator<<( std::ostream & str, const PathInfo & obj )
    {
      iostr::IosFmtFlagsSaver autoResoreState( str );

      str << obj.asString() << "{";
      if ( !obj.isExist() ) {
        str << Errno( obj.error() );
      } else {
        str << obj.asStatMode() << " " << std::dec << obj.owner() << "/" << obj.group();

        if ( obj.isFile() )
          str << " size " << obj.size();
      }

      return str << "}";
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	filesystem utilities
    //
    ///////////////////////////////////////////////////////////////////

    /******************************************************************
     **
     **	FUNCTION NAME : _Log_Result
     **	FUNCTION TYPE : int
     **
     **	DESCRIPTION : Helper function to log return values.
    */
#define _Log_Result MIL << endl, __Log_Result
    inline int __Log_Result( const int res, const char * rclass = 0 /*errno*/ )
    {
      if ( res )
      {
        if ( rclass )
          WAR << " FAILED: " << rclass << " " << res << endl;
        else
          WAR << " FAILED: " << str::strerror( res ) << endl;
      }
      return res;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : PathInfo::mkdir
    //	METHOD TYPE : int
    //
    int mkdir( const Pathname & path, unsigned mode )
    {
      MIL << "mkdir " << path << ' ' << str::octstring( mode );
      if ( ::mkdir( path.asString().c_str(), mode ) == -1 ) {
        return _Log_Result( errno );
      }
      return _Log_Result( 0 );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : assert_dir()
    //	METHOD TYPE : int
    //
    int assert_dir( const Pathname & path, unsigned mode )
    {
      if ( path.empty() )
        return ENOENT;

      { // Handle existing paths in advance.
        PathInfo pi( path );
        if ( pi.isDir() )
          return 0;
        if ( pi.isExist() )
          return EEXIST;
      }

      string spath = path.asString()+"/";
      string::size_type lastpos = ( path.relative() ? 2 : 1 ); // skip leasding './' or '/'
      string::size_type pos = string::npos;
      int ret = 0;

      while ( (pos = spath.find('/',lastpos)) != string::npos )
      {
        string dir( spath.substr(0,pos) );
        ret = ::mkdir( dir.c_str(), mode );
        if ( ret == -1 )
        {
          if ( errno == EEXIST ) // ignore errors about already existing paths
            ret = 0;
          else
          {
            ret = errno;
            WAR << " FAILED: mkdir " << dir << ' ' << str::octstring( mode ) << " errno " << ret << endl;
          }
        }
        else
        {
          MIL << "mkdir " << dir << ' ' << str::octstring( mode ) << endl;
        }
        lastpos = pos+1;
      }

      return ret;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : rmdir
    //	METHOD TYPE : int
    //
    int rmdir( const Pathname & path )
    {
      MIL << "rmdir " << path;
      if ( ::rmdir( path.asString().c_str() ) == -1 ) {
        return _Log_Result( errno );
      }
      return _Log_Result( 0 );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : recursive_rmdir
    //	METHOD TYPE : int
    //
    static int recursive_rmdir_1( const Pathname & dir )
    {
      DIR * dp;
      struct dirent * d;

      if ( ! (dp = opendir( dir.c_str() )) )
        return _Log_Result( errno );

      while ( (d = readdir(dp)) )
      {
        std::string direntry = d->d_name;
        if ( direntry == "." || direntry == ".." )
          continue;
        Pathname new_path( dir / d->d_name );

        struct stat st;
        if ( ! lstat( new_path.c_str(), &st ) )
        {
          if ( S_ISDIR( st.st_mode ) )
            recursive_rmdir_1( new_path );
          else
            ::unlink( new_path.c_str() );
        }
      }
      closedir( dp );

      if ( ::rmdir( dir.c_str() ) < 0 )
        return errno;

      return 0;
    }
    ///////////////////////////////////////////////////////////////////
    int recursive_rmdir( const Pathname & path )
    {
      MIL << "recursive_rmdir " << path << ' ';
      PathInfo p( path );

      if ( !p.isExist() ) {
        return _Log_Result( 0 );
      }

      if ( !p.isDir() ) {
        return _Log_Result( ENOTDIR );
      }

      return _Log_Result( recursive_rmdir_1( path ) );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : clean_dir
    //	METHOD TYPE : int
    //
    int clean_dir( const Pathname & path )
    {
      MIL << "clean_dir " << path << ' ';
      PathInfo p( path );

      if ( !p.isExist() ) {
        return _Log_Result( 0 );
      }

      if ( !p.isDir() ) {
        return _Log_Result( ENOTDIR );
      }

      string cmd( str::form( "cd '%s' && rm -rf --preserve-root -- *", path.asString().c_str() ) );
      ExternalProgram prog( cmd, ExternalProgram::Stderr_To_Stdout );
      for ( string output( prog.receiveLine() ); output.length(); output = prog.receiveLine() ) {
        MIL << "  " << output;
      }
      int ret = prog.close();
      return _Log_Result( ret, "returned" );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : copy_dir
    //	METHOD TYPE : int
    //
    int copy_dir( const Pathname & srcpath, const Pathname & destpath )
    {
      MIL << "copy_dir " << srcpath << " -> " << destpath << ' ';

      PathInfo sp( srcpath );
      if ( !sp.isDir() ) {
        return _Log_Result( ENOTDIR );
      }

      PathInfo dp( destpath );
      if ( !dp.isDir() ) {
        return _Log_Result( ENOTDIR );
      }

      PathInfo tp( destpath + srcpath.basename() );
      if ( tp.isExist() ) {
        return _Log_Result( EEXIST );
      }


      const char *const argv[] = {
        "/bin/cp",
        "-dR",
        "--",
        srcpath.asString().c_str(),
        destpath.asString().c_str(),
        NULL
      };
      ExternalProgram prog( argv, ExternalProgram::Stderr_To_Stdout );
      for ( string output( prog.receiveLine() ); output.length(); output = prog.receiveLine() ) {
        MIL << "  " << output;
      }
      int ret = prog.close();
      return _Log_Result( ret, "returned" );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : copy_dir_content
    //	METHOD TYPE : int
    //
    int copy_dir_content(const Pathname & srcpath, const Pathname & destpath)
    {
      MIL << "copy_dir " << srcpath << " -> " << destpath << ' ';

      PathInfo sp( srcpath );
      if ( !sp.isDir() ) {
        return _Log_Result( ENOTDIR );
      }

      PathInfo dp( destpath );
      if ( !dp.isDir() ) {
        return _Log_Result( ENOTDIR );
      }

      if ( srcpath == destpath ) {
        return _Log_Result( EEXIST );
      }

      std::string src( srcpath.asString());
      src += "/.";
      const char *const argv[] = {
        "/bin/cp",
        "-dR",
        "--",
        src.c_str(),
        destpath.asString().c_str(),
        NULL
      };
      ExternalProgram prog( argv, ExternalProgram::Stderr_To_Stdout );
      for ( string output( prog.receiveLine() ); output.length(); output = prog.receiveLine() ) {
        MIL << "  " << output;
      }
      int ret = prog.close();
      return _Log_Result( ret, "returned" );
    }

    ///////////////////////////////////////////////////////////////////////
    // dirForEach
    ///////////////////////////////////////////////////////////////////////

    const StrMatcher & matchNoDots()
    {
      static StrMatcher noDots( "[^.]*", Match::GLOB );
      return noDots;
    }

    int dirForEach( const Pathname & dir_r, function<bool(const Pathname &, const char *const)> fnc_r )
    {
      if ( ! fnc_r )
	return 0;

      AutoDispose<DIR *> dir( ::opendir( dir_r.c_str() ),
			      []( DIR * dir_r ) { if ( dir_r ) ::closedir( dir_r ); } );

      MIL << "readdir " << dir_r << ' ';
      if ( ! dir )
	return _Log_Result( errno );
      MIL << endl; // close line before callbacks are invoked.

      int ret = 0;
      for ( struct dirent * entry = ::readdir( dir ); entry; entry = ::readdir( dir ) )
      {
        if ( entry->d_name[0] == '.' && ( entry->d_name[1] == '\0' || ( entry->d_name[1] == '.' && entry->d_name[2] == '\0' ) ) )
	  continue; // omitt . and ..

	if ( ! fnc_r( dir_r, entry->d_name ) )
	{
	  ret = -1;
	  break;
	}
      }
      return ret;
    }

    int dirForEach( const Pathname & dir_r, const StrMatcher & matcher_r, function<bool( const Pathname &, const char *const)> fnc_r )
    {
      if ( ! fnc_r )
	return 0;

      bool nodots = ( &matcher_r == &matchNoDots() );
      return dirForEach( dir_r,
			 [&]( const Pathname & dir_r, const char *const name_r )->bool
			 {
			   if ( ( nodots && name_r[0] == '.' ) || ! matcher_r( name_r ) )
			     return true;
			   return fnc_r( dir_r, name_r );
			 } );
    }

    ///////////////////////////////////////////////////////////////////
    // readdir
    ///////////////////////////////////////////////////////////////////

    int readdir( std::list<std::string> & retlist_r, const Pathname & path_r, bool dots_r )
    {
      retlist_r.clear();
      return dirForEach( path_r,
			 [&]( const Pathname & dir_r, const char *const name_r )->bool
			 {
			   if ( dots_r || name_r[0] != '.' )
			     retlist_r.push_back( name_r );
			   return true;
			 } );
    }


    int readdir( std::list<Pathname> & retlist_r, const Pathname & path_r, bool dots_r )
    {
      retlist_r.clear();
      return dirForEach( path_r,
			 [&]( const Pathname & dir_r, const char *const name_r )->bool
			 {
			   if ( dots_r || name_r[0] != '.' )
			     retlist_r.push_back( dir_r/name_r );
			   return true;
			 } );
    }

    bool DirEntry::operator==( const DirEntry &rhs ) const
    {
      // if one of the types is not known, use the name only
      if ( type == FT_NOT_AVAIL || rhs.type == FT_NOT_AVAIL )
        return ( name == rhs.name );
      return ((name == rhs.name ) && (type == rhs.type));
    }

    int readdir( DirContent & retlist_r, const Pathname & path_r, bool dots_r, PathInfo::Mode statmode_r )
    {
      retlist_r.clear();
      return dirForEach( path_r,
			 [&]( const Pathname & dir_r, const char *const name_r )->bool
			 {
			   if ( dots_r || name_r[0] != '.' )
			     retlist_r.push_back( DirEntry( name_r, PathInfo( dir_r/name_r, statmode_r ).fileType() ) );
			   return true;
			 } );
    }

    std::ostream & operator<<( std::ostream & str, const DirContent & obj )
    { return dumpRange( str, obj.begin(), obj.end() ); }

    ///////////////////////////////////////////////////////////////////
    // is_empty_dir
    ///////////////////////////////////////////////////////////////////

    int is_empty_dir( const Pathname & path_r )
    {
      return dirForEach( path_r,
			 [&]( const Pathname & dir_r, const char *const name_r )->bool
			 { return false; } );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : unlink
    //	METHOD TYPE : int
    //
    int unlink( const Pathname & path )
    {
      MIL << "unlink " << path;
      if ( ::unlink( path.asString().c_str() ) == -1 ) {
        return _Log_Result( errno );
      }
      return _Log_Result( 0 );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : rename
    //	METHOD TYPE : int
    //
    int rename( const Pathname & oldpath, const Pathname & newpath )
    {
      MIL << "rename " << oldpath << " -> " << newpath;
      if ( ::rename( oldpath.asString().c_str(), newpath.asString().c_str() ) == -1 ) {
        return _Log_Result( errno );
      }
      return _Log_Result( 0 );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : exchange
    //	METHOD TYPE : int
    //
    int exchange( const Pathname & lpath, const Pathname & rpath )
    {
      MIL << "exchange " << lpath << " <-> " << rpath;
      if ( lpath.empty() || rpath.empty() )
        return _Log_Result( EINVAL );

      PathInfo linfo( lpath );
      PathInfo rinfo( rpath );

      if ( ! linfo.isExist() )
      {
        if ( ! rinfo.isExist() )
          return _Log_Result( 0 ); // both don't exist.

        // just rename rpath -> lpath
        int ret = assert_dir( lpath.dirname() );
        if ( ret != 0 )
          return _Log_Result( ret );
        if ( ::rename( rpath.c_str(), lpath.c_str() ) == -1 ) {
          return _Log_Result( errno );
        }
        return _Log_Result( 0 );
      }

      // HERE: lpath exists:
      if ( ! rinfo.isExist() )
      {
        // just rename lpath -> rpath
        int ret = assert_dir( rpath.dirname() );
        if ( ret != 0 )
          return _Log_Result( ret );
        if ( ::rename( lpath.c_str(), rpath.c_str() ) == -1 ) {
          return _Log_Result( errno );
        }
        return _Log_Result( 0 );
      }

      // HERE: both exist
      TmpFile tmpfile( TmpFile::makeSibling( rpath ) );
      if ( ! tmpfile )
        return _Log_Result( errno );
      Pathname tmp( tmpfile.path() );
      ::unlink( tmp.c_str() );

      if ( ::rename( lpath.c_str(), tmp.c_str() ) == -1 ) {
        return _Log_Result( errno );
      }
      if ( ::rename( rpath.c_str(), lpath.c_str() ) == -1 ) {
        ::rename( tmp.c_str(), lpath.c_str() );
        return _Log_Result( errno );
      }
      if ( ::rename( tmp.c_str(), rpath.c_str() ) == -1 ) {
        ::rename( lpath.c_str(), rpath.c_str() );
        ::rename( tmp.c_str(), lpath.c_str() );
        return _Log_Result( errno );
      }
      return _Log_Result( 0 );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : copy
    //	METHOD TYPE : int
    //
    int copy( const Pathname & file, const Pathname & dest )
    {
      MIL << "copy " << file << " -> " << dest << ' ';

      PathInfo sp( file );
      if ( !sp.isFile() ) {
        return _Log_Result( EINVAL );
      }

      PathInfo dp( dest );
      if ( dp.isDir() ) {
        return _Log_Result( EISDIR );
      }

      const char *const argv[] = {
        "/bin/cp",
        "--remove-destination",
        "--",
        file.asString().c_str(),
        dest.asString().c_str(),
        NULL
      };
      ExternalProgram prog( argv, ExternalProgram::Stderr_To_Stdout );
      for ( string output( prog.receiveLine() ); output.length(); output = prog.receiveLine() ) {
        MIL << "  " << output;
      }
      int ret = prog.close();
      return _Log_Result( ret, "returned" );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : symlink
    //	METHOD TYPE : int
    //
    int symlink( const Pathname & oldpath, const Pathname & newpath )
    {
      MIL << "symlink " << newpath << " -> " << oldpath;
      if ( ::symlink( oldpath.asString().c_str(), newpath.asString().c_str() ) == -1 ) {
        return _Log_Result( errno );
      }
      return _Log_Result( 0 );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : hardlink
    //	METHOD TYPE : int
    //
    int hardlink( const Pathname & oldpath, const Pathname & newpath )
    {
      MIL << "hardlink " << newpath << " -> " << oldpath;
      if ( ::link( oldpath.asString().c_str(), newpath.asString().c_str() ) == -1 ) {
        return _Log_Result( errno );
      }
      return _Log_Result( 0 );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : hardlink
    //	METHOD TYPE : int
    //
    int hardlinkCopy( const Pathname & oldpath, const Pathname & newpath )
    {
      MIL << "hardlinkCopy " << oldpath << " -> " << newpath;

      PathInfo pi( oldpath, PathInfo::LSTAT );
      if ( pi.isLink() )
      {
	// dont hardlink symlinks!
	return copy( oldpath, newpath );
      }

      pi.lstat( newpath );
      if ( pi.isExist() )
      {
	int res = unlink( newpath );
	if ( res != 0 )
	  return _Log_Result( res );
      }

      // Here: no symlink, no newpath
      if ( ::link( oldpath.asString().c_str(), newpath.asString().c_str() ) == -1 )
      {
        switch ( errno )
        {
	  case EPERM: // /proc/sys/fs/protected_hardlink in proc(5)
          case EXDEV: // oldpath  and  newpath are not on the same mounted file system
            return copy( oldpath, newpath );
            break;
        }
        return _Log_Result( errno );
      }
      return _Log_Result( 0 );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : readlink
    //	METHOD TYPE : int
    //
    int readlink( const Pathname & symlink_r, Pathname & target_r )
    {
      static const ssize_t bufsiz = 2047;
      static char buf[bufsiz+1];
      ssize_t ret = ::readlink( symlink_r.c_str(), buf, bufsiz );
      if ( ret == -1 )
      {
        target_r = Pathname();
        MIL << "readlink " << symlink_r;
        return _Log_Result( errno );
      }
      buf[ret] = '\0';
      target_r = buf;
      return 0;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //  METHOD NAME : expandlink
    //  METHOD TYPE : Pathname
    //
    Pathname expandlink( const Pathname & path_r )
    {
      static const unsigned int level_limit = 256;
      static unsigned int count;
      Pathname path(path_r);
      PathInfo info(path_r, PathInfo::LSTAT);

      for (count = level_limit; info.isLink() && count; count--)
      {
        DBG << "following symlink " << path;
        path = path.dirname() / readlink(path);
        DBG << "->" << path << std::endl;
        info = PathInfo(path, PathInfo::LSTAT);
      }

      // expand limit reached
      if (count == 0)
      {
        ERR << "Expand level limit reached. Probably a cyclic symbolic link." << endl;
        return Pathname();
      }
      // symlink
      else if (count < level_limit)
      {
        // check for a broken link
        if (PathInfo(path).isExist())
          return path;
        // broken link, return an empty path
        else
        {
          ERR << path << " is broken (expanded from " << path_r << ")" << endl;
          return Pathname();
        }
      }

      // not a symlink, return the original pathname
      DBG << "not a symlink" << endl;
      return path;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : copy_file2dir
    //	METHOD TYPE : int
    //
    int copy_file2dir( const Pathname & file, const Pathname & dest )
    {
      MIL << "copy_file2dir " << file << " -> " << dest << ' ';

      PathInfo sp( file );
      if ( !sp.isFile() ) {
        return _Log_Result( EINVAL );
      }

      PathInfo dp( dest );
      if ( !dp.isDir() ) {
        return _Log_Result( ENOTDIR );
      }

      const char *const argv[] = {
        "/bin/cp",
        "--",
        file.asString().c_str(),
        dest.asString().c_str(),
        NULL
      };
      ExternalProgram prog( argv, ExternalProgram::Stderr_To_Stdout );
      for ( string output( prog.receiveLine() ); output.length(); output = prog.receiveLine() ) {
        MIL << "  " << output;
      }
      int ret = prog.close();
      return _Log_Result( ret, "returned" );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : md5sum
    //	METHOD TYPE : std::string
    //
    std::string md5sum( const Pathname & file )
    {
      if ( ! PathInfo( file ).isFile() ) {
        return string();
      }
      std::ifstream istr( file.asString().c_str() );
      if ( ! istr ) {
        return string();
      }
      return Digest::digest( "MD5", istr );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : sha1sum
    //	METHOD TYPE : std::string
    //
    std::string sha1sum( const Pathname & file )
    {
      return checksum(file, "SHA1");
    }

    ///////////////////////////////////////////////////////////////////
    //
    //  METHOD NAME : checksum
    //  METHOD TYPE : std::string
    //
    std::string checksum( const Pathname & file, const std::string &algorithm )
    {
      if ( ! PathInfo( file ).isFile() ) {
        return string();
      }
      std::ifstream istr( file.asString().c_str() );
      if ( ! istr ) {
        return string();
      }
      return Digest::digest( algorithm, istr );
    }

    bool is_checksum( const Pathname & file, const CheckSum &checksum )
    {
      return ( filesystem::checksum(file,  checksum.type()) == checksum.checksum() );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : erase
    //	METHOD TYPE : int
    //
    int erase( const Pathname & path )
    {
      int res = 0;
      PathInfo p( path, PathInfo::LSTAT );
      if ( p.isExist() )
        {
          if ( p.isDir() )
            res = recursive_rmdir( path );
          else
            res = unlink( path );
        }
      return res;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : chmod
    //	METHOD TYPE : int
    //
    int chmod( const Pathname & path, mode_t mode )
    {
      MIL << "chmod " << path << ' ' << str::octstring( mode );
      if ( ::chmod( path.asString().c_str(), mode ) == -1 ) {
        return _Log_Result( errno );
      }
      return _Log_Result( 0 );
    }

    int addmod( const Pathname & path, mode_t mode )
    {
      mode_t omode( PathInfo( path ).st_mode() );
      mode_t tmode( omode | mode );
      if ( omode != mode )
        return chmod( path, tmode );
      return 0;
    }

    int delmod( const Pathname & path, mode_t mode )
    {
      mode_t omode( PathInfo( path ).st_mode() );
      mode_t tmode( omode & ~mode );
      if ( omode != mode )
        return chmod( path, tmode );
      return 0;
    }

   //////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : zipType
    //	METHOD TYPE : ZIP_TYPE
    //
    ZIP_TYPE zipType( const Pathname & file )
    {
      ZIP_TYPE ret = ZT_NONE;

      int fd = open( file.asString().c_str(), O_RDONLY|O_CLOEXEC );

      if ( fd != -1 ) {
        const int magicSize = 3;
        unsigned char magic[magicSize];
        memset( magic, 0, magicSize );
        if ( read( fd, magic, magicSize ) == magicSize ) {
          if ( magic[0] == 0037 && magic[1] == 0213 ) {
            ret = ZT_GZ;
          } else if ( magic[0] == 'B' && magic[1] == 'Z' && magic[2] == 'h' ) {
            ret = ZT_BZ2;
          }
        }
        close( fd );
      }

      return ret;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : df
    //	METHOD TYPE : ByteCount
    //
    ByteCount df( const Pathname & path_r )
    {
      ByteCount ret( -1 );
      struct statvfs sb;
      if ( statvfs( path_r.c_str(), &sb ) == 0 )
        {
          ret = sb.f_bfree * sb.f_bsize;
        }
      return ret;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : getUmask
    //	METHOD TYPE : mode_t
    //
    mode_t getUmask()
    {
      mode_t mask = ::umask( 0022 );
      ::umask( mask );
      return mask;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : getUmask
    //	METHOD TYPE : mode_t
    //
    int assert_file( const Pathname & path, unsigned mode )
    {
      int ret = assert_dir( path.dirname() );
      MIL << "assert_file " << str::octstring( mode ) << " " << path;
      if ( ret != 0 )
        return _Log_Result( ret );

      PathInfo pi( path );
      if ( pi.isExist() )
        return _Log_Result( pi.isFile() ? 0 : EEXIST );

      int fd = ::creat( path.c_str(), mode );
      if ( fd == -1 )
        return _Log_Result( errno );

      ::close( fd );
      return _Log_Result( 0 );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //  METHOD NAME : touch
    //  METHOD TYPE : int
    //
    int touch (const Pathname & path)
    {
      MIL << "touch " << path;
      struct ::utimbuf times;
      times.actime = ::time( 0 );
      times.modtime = ::time( 0 );
      if ( ::utime( path.asString().c_str(), &times ) == -1 ) {
        return _Log_Result( errno );
      }
      return _Log_Result( 0 );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace filesystem
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
