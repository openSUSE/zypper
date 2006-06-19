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

#include <iostream>
#include <fstream>
#include <iomanip>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/base/IOStream.h"

#include "zypp/ExternalProgram.h"
#include "zypp/PathInfo.h"
#include "zypp/Digest.h"


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
      if ( owner() == getuid() ) {
        return( uperm()/0100 );
      } else if ( group() == getgid() ) {
        return( gperm()/010 );
      }
      return operm();
    }

    /******************************************************************
     **
     **	FUNCTION NAME : PathInfo::major
     **	FUNCTION TYPE : unsigned int
     */
    unsigned int PathInfo::major() const
    {
      return isBlk() || isChr() ? ::major(statbuf_C.st_rdev) : 0;
    }

    /******************************************************************
     **
     **	FUNCTION NAME : PathInfo::minor
     **	FUNCTION TYPE : unsigned int
     */
    unsigned int PathInfo::minor() const
    {
      return isBlk() || isChr() ? ::minor(statbuf_C.st_rdev) : 0;
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
        str << "does not exist}";
      } else {
        str << obj.asStatMode() << " " << std::dec << obj.owner() << "/" << obj.group();

        if ( obj.isFile() )
          str << " size " << obj.size();

        str << "}";
      }

      return str;
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
    inline int _Log_Result( const int res, const char * rclass = "errno" )
    {
      if ( res )
        DBG << " FAILED: " << rclass << " " << res;
      DBG << std::endl;
      return res;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : PathInfo::mkdir
    //	METHOD TYPE : int
    //
    int mkdir( const Pathname & path, unsigned mode )
    {
      DBG << "mkdir " << path << ' ' << str::octstring( mode );
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
      string::size_type pos, lastpos = 0;
      string spath = path.asString()+"/";
      int ret = 0;

      if(path.empty())
        return ENOENT;

      // skip ./
      if(path.relative())
        lastpos=2;
      // skip /
      else
        lastpos=1;

      //    DBG << "about to create " << spath << endl;
      while((pos = spath.find('/',lastpos)) != string::npos )
        {
          string dir = spath.substr(0,pos);
          ret = ::mkdir(dir.c_str(), mode);
          if(ret == -1)
            {
              // ignore errors about already existing directorys
              if(errno == EEXIST)
                ret=0;
              else
                ret=errno;
            }
          //	DBG << "creating directory " << dir << (ret?" failed":" succeeded") << endl;
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
      DBG << "rmdir " << path;
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
    int recursive_rmdir( const Pathname & path )
    {
      DBG << "recursive_rmdir " << path << ' ';
      PathInfo p( path );

      if ( !p.isExist() ) {
        return _Log_Result( 0 );
      }

      if ( !p.isDir() ) {
        return _Log_Result( ENOTDIR );
      }

      try
        {
          boost::filesystem::path bp( path.asString(), boost::filesystem::native );
          boost::filesystem::remove_all( bp );
        }
      catch ( boost::filesystem::filesystem_error & excpt )
        {
          DBG << " FAILED: " << excpt.what() << std::endl;
          return -1;
        }

      return _Log_Result( 0 );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : clean_dir
    //	METHOD TYPE : int
    //
    int clean_dir( const Pathname & path )
    {
      DBG << "clean_dir " << path << ' ';
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
        DBG << "  " << output;
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
      DBG << "copy_dir " << srcpath << " -> " << destpath << ' ';

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
        DBG << "  " << output;
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
      DBG << "copy_dir " << srcpath << " -> " << destpath << ' ';

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
        DBG << "  " << output;
      }
      int ret = prog.close();
      return _Log_Result( ret, "returned" );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : readdir
    //  METHOD TYPE : int
    //
    int readdir( std::list<std::string> & retlist,
                 const Pathname & path, bool dots )
    {
      retlist.clear();

      DBG << "readdir " << path << ' ';

      DIR * dir = ::opendir( path.asString().c_str() );
      if ( ! dir ) {
        return _Log_Result( errno );
      }

      struct dirent *entry;
      while ( (entry = ::readdir( dir )) != 0 ) {

        if ( entry->d_name[0] == '.' ) {
          if ( !dots )
            continue;
          if ( entry->d_name[1] == '\0'
               || (    entry->d_name[1] == '.'
                    && entry->d_name[2] == '\0' ) )
            continue;
        }
        retlist.push_back( entry->d_name );
      }

      ::closedir( dir );

      return _Log_Result( 0 );
    }


    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : readdir
    //	METHOD TYPE : int
    //
    int readdir( std::list<Pathname> & retlist,
                 const Pathname & path, bool dots )
    {
      retlist.clear();

      std::list<string> content;
      int res = readdir( content, path, dots );

      if ( !res ) {
        for ( std::list<string>::const_iterator it = content.begin(); it != content.end(); ++it ) {
          retlist.push_back( path + *it );
        }
      }

      return res;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : readdir
    //	METHOD TYPE : int
    //
    int readdir( DirContent & retlist, const Pathname & path,
                 bool dots, PathInfo::Mode statmode )
    {
      retlist.clear();

      std::list<string> content;
      int res = readdir( content, path, dots );

      if ( !res ) {
        for ( std::list<string>::const_iterator it = content.begin(); it != content.end(); ++it ) {
          PathInfo p( path + *it, statmode );
          retlist.push_back( DirEntry( *it, p.fileType() ) );
        }
      }

      return res;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : is_empty_dir
    //	METHOD TYPE : int
    //
    int is_empty_dir(const Pathname & path)
    {
      DIR * dir = ::opendir( path.asString().c_str() );
      if ( ! dir ) {
        return _Log_Result( errno );
      }

      struct dirent *entry;
      while ( (entry = ::readdir( dir )) != NULL )
      {
        std::string name(entry->d_name);

        if ( name == "." || name == "..")
	  continue;

        break;
      }
      ::closedir( dir );

      return entry != NULL ? -1 : 0;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : unlink
    //	METHOD TYPE : int
    //
    int unlink( const Pathname & path )
    {
      DBG << "unlink " << path;
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
      DBG << "rename " << oldpath << " -> " << newpath;
      if ( ::rename( oldpath.asString().c_str(), newpath.asString().c_str() ) == -1 ) {
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
      DBG << "copy " << file << " -> " << dest << ' ';

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
        "--",
        file.asString().c_str(),
        dest.asString().c_str(),
        NULL
      };
      ExternalProgram prog( argv, ExternalProgram::Stderr_To_Stdout );
      for ( string output( prog.receiveLine() ); output.length(); output = prog.receiveLine() ) {
        DBG << "  " << output;
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
      DBG << "symlink " << newpath << " -> " << oldpath;
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
      DBG << "hardlink " << newpath << " -> " << oldpath;
      if ( ::link( oldpath.asString().c_str(), newpath.asString().c_str() ) == -1 ) {
        return _Log_Result( errno );
      }
      return _Log_Result( 0 );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : copy_file2dir
    //	METHOD TYPE : int
    //
    int copy_file2dir( const Pathname & file, const Pathname & dest )
    {
      DBG << "copy_file2dir " << file << " -> " << dest << ' ';

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
        DBG << "  " << output;
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
      DBG << "chmod " << path << ' ' << str::octstring( mode );
      if ( ::chmod( path.asString().c_str(), mode ) == -1 ) {
        return _Log_Result( errno );
      }
      return _Log_Result( 0 );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : zipType
    //	METHOD TYPE : ZIP_TYPE
    //
    ZIP_TYPE zipType( const Pathname & file )
    {
      ZIP_TYPE ret = ZT_NONE;

      int fd = open( file.asString().c_str(), O_RDONLY );

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

    /////////////////////////////////////////////////////////////////
  } // namespace filesystem
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
