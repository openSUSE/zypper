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

   File:       PathInfo.cc

   Author:     Michael Andres <ma@suse.de>
   Maintainer: Michael Andres <ma@suse.de>

/-*/

#include <iostream>
#include <fstream>
#include <iomanip>

#include <y2util/Y2SLog.h>
#include <y2util/stringutil.h>
#include <y2util/ExternalProgram.h>

#include <y2util/PathInfo.h>
#include <y2util/Digest.h>

using namespace std;

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PathInfo::PathInfo
//	METHOD TYPE : Constructor
//
//	DESCRIPTION :
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
//
//	METHOD NAME : PathInfo::PathInfo
//	METHOD TYPE : Constructor
//
//	DESCRIPTION :
//
PathInfo::PathInfo( const string & path, Mode initial )
    : path_t( path )
    , mode_e( initial )
    , error_i( -1 )
{
  operator()();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PathInfo::PathInfo
//	METHOD TYPE : Constructor
//
//	DESCRIPTION :
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
//
//	METHOD NAME : PathInfo::~PathInfo
//	METHOD TYPE : Destructor
//
//	DESCRIPTION :
//
PathInfo::~PathInfo()
{
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PathInfo::operator()
//	METHOD TYPE : bool
//
//	DESCRIPTION :
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
//
//	METHOD NAME : PathInfo::fileType
//	METHOD TYPE : PathInfo::file_type
//
PathInfo::file_type PathInfo::fileType() const
{
  if ( isExist() )
    return stat_mode( st_mode() ).fileType();
  return NOT_EXIST;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PathInfo::userMay
//	METHOD TYPE : mode_t
//
//	DESCRIPTION :
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
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : ostream &
**
**	DESCRIPTION :
*/
ostream & operator<<( ostream & str, const PathInfo & obj )
{
  ios::fmtflags state_ii = str.flags();

  str << obj.asString() << "{";
  if ( !obj.isExist() ) {
    str << "does not exist}";
  } else {
    str << PathInfo::stat_mode( obj.st_mode() ) << " " << dec << obj.owner() << "/" << obj.group();

    if ( obj.isFile() )
      str << " size " << obj.size();

    str << "}";
  }
  str.flags( state_ii );
  return str;
}

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : std::ostream &
**
**	DESCRIPTION :
*/
ostream & operator<<( ostream & str, PathInfo::file_type obj )
{
  switch ( obj ) {
#define EMUMOUT(T) case PathInfo::T: return str << #T; break
    EMUMOUT( NOT_AVAIL );
    EMUMOUT( NOT_EXIST );
    EMUMOUT( T_FILE );
    EMUMOUT( T_DIR );
    EMUMOUT( T_CHARDEV );
    EMUMOUT( T_BLOCKDEV );
    EMUMOUT( T_FIFO );
    EMUMOUT( T_LINK );
    EMUMOUT( T_SOCKET );
#undef EMUMOUT
  }
  return str;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PathInfo::stat_mode::fileType
//	METHOD TYPE : PathInfo::file_type
//
PathInfo::file_type PathInfo::stat_mode::fileType() const
{
  if ( isFile() )
    return T_FILE;
  if ( isDir() )
    return T_DIR;
  if ( isLink() )
    return T_LINK;
  if ( isChr() )
    return T_CHARDEV;
  if ( isBlk() )
    return T_BLOCKDEV;
  if ( isFifo() )
    return T_FIFO;
  if ( isSock() )
    return T_SOCKET ;

  return NOT_AVAIL;
}

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : std::ostream &
**
**	DESCRIPTION :
*/
std::ostream & operator<<( std::ostream & str, const PathInfo::stat_mode & obj )
{
  ios::fmtflags state_ii = str.flags();
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

  str << t << " " << setfill( '0' ) << setw( 4 ) << oct << obj.perm();
  str.flags( state_ii );
  return str;
}

/******************************************************************
**
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
  DBG << endl;
  return res;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PathInfo::mkdir
//	METHOD TYPE : int
//
//	DESCRIPTION :
//
int PathInfo::mkdir( const Pathname & path, unsigned mode )
{
  DBG << "mkdir " << path << ' ' << stringutil::octstring( mode );
  if ( ::mkdir( path.asString().c_str(), mode ) == -1 ) {
    return _Log_Result( errno );
  }
  return _Log_Result( 0 );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PathInfo::assert_dir()
//	METHOD TYPE : int
//
//	DESCRIPTION :
//
int PathInfo::assert_dir( const Pathname & path, unsigned mode )
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
//
//	METHOD NAME : PathInfo::rmdir
//	METHOD TYPE : int
//
//	DESCRIPTION :
//
int PathInfo::rmdir( const Pathname & path )
{
  DBG << "rmdir " << path;
  if ( ::rmdir( path.asString().c_str() ) == -1 ) {
    return _Log_Result( errno );
  }
  return _Log_Result( 0 );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PathInfo::recursive_rmdir
//	METHOD TYPE : int
//
//	DESCRIPTION :
//
int PathInfo::recursive_rmdir( const Pathname & path )
{
  DBG << "recursive_rmdir " << path << ' ';
  PathInfo p( path );

  if ( !p.isExist() ) {
    return _Log_Result( 0 );
  }

  if ( !p.isDir() ) {
    return _Log_Result( ENOTDIR );
  }

  const char *const argv[] = {
    "/bin/rm",
    "-rf",
    "--preserve-root",
    "--",
    path.asString().c_str(),
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
//
//	METHOD NAME : PathInfo::clean_dir
//	METHOD TYPE : int
//
//	DESCRIPTION :
//
int PathInfo::clean_dir( const Pathname & path )
{
  DBG << "clean_dir " << path << ' ';
  PathInfo p( path );

  if ( !p.isExist() ) {
    return _Log_Result( 0 );
  }

  if ( !p.isDir() ) {
    return _Log_Result( ENOTDIR );
  }

  string cmd( stringutil::form( "cd '%s' && rm -rf --preserve-root -- *", path.asString().c_str() ) );
  ExternalProgram prog( cmd, ExternalProgram::Stderr_To_Stdout );
  for ( string output( prog.receiveLine() ); output.length(); output = prog.receiveLine() ) {
    DBG << "  " << output;
  }
  int ret = prog.close();
  return _Log_Result( ret, "returned" );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PathInfo::copy_dir
//	METHOD TYPE : int
//
//	DESCRIPTION :
//
int PathInfo::copy_dir( const Pathname & srcpath, const Pathname & destpath )
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
//
//	METHOD NAME : PathInfo::readdir
//	METHOD TYPE : int
//
//	DESCRIPTION :
//
int PathInfo::readdir( std::list<std::string> & retlist,
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
//
//	METHOD NAME : PathInfo::readdir
//	METHOD TYPE : int
//
int PathInfo::readdir( dircontent & retlist, const Pathname & path,
		       bool dots, Mode statmode )
{
  retlist.clear();

  list<string> content;
  int res = readdir( content, path, dots );

  if ( !res ) {
    for ( list<string>::const_iterator it = content.begin(); it != content.end(); ++it ) {
      PathInfo p( path + *it, statmode );
      retlist.push_back( direntry( *it, p.fileType() ) );
    }
  }

  return res;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PathInfo::unlink
//	METHOD TYPE : int
//
//	DESCRIPTION :
//
int PathInfo::unlink( const Pathname & path )
{
  DBG << "unlink " << path;
  if ( ::unlink( path.asString().c_str() ) == -1 ) {
    return _Log_Result( errno );
  }
  return _Log_Result( 0 );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PathInfo::rename
//	METHOD TYPE : int
//
//	DESCRIPTION :
//
int PathInfo::rename( const Pathname & oldpath, const Pathname & newpath )
{
  DBG << "rename " << oldpath << " -> " << newpath;
  if ( ::rename( oldpath.asString().c_str(), newpath.asString().c_str() ) == -1 ) {
    return _Log_Result( errno );
  }
  return _Log_Result( 0 );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PathInfo::copy
//	METHOD TYPE : int
//
//	DESCRIPTION :
//
int PathInfo::copy( const Pathname & file, const Pathname & dest )
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
//
//	METHOD NAME : PathInfo::symlink
//	METHOD TYPE : int
//
//	DESCRIPTION :
//
int PathInfo::symlink( const Pathname & oldpath, const Pathname & newpath )
{
  DBG << "symlink " << newpath << " -> " << oldpath;
  if ( ::symlink( oldpath.asString().c_str(), newpath.asString().c_str() ) == -1 ) {
    return _Log_Result( errno );
  }
  return _Log_Result( 0 );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PathInfo::hardlink
//	METHOD TYPE : int
//
//	DESCRIPTION :
//
int PathInfo::hardlink( const Pathname & oldpath, const Pathname & newpath )
{
  DBG << "hardlink " << newpath << " -> " << oldpath;
  if ( ::link( oldpath.asString().c_str(), newpath.asString().c_str() ) == -1 ) {
    return _Log_Result( errno );
  }
  return _Log_Result( 0 );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PathInfo::copy_file2dir
//	METHOD TYPE : int
//
//	DESCRIPTION :
//
int PathInfo::copy_file2dir( const Pathname & file, const Pathname & dest )
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
//
//	METHOD NAME : PathInfo::md5sum
//	METHOD TYPE : std::string
//
std::string PathInfo::md5sum( const Pathname & file )
{
  if ( ! PathInfo( file ).isFile() ) {
    return string();
  }
  ifstream istr( file.asString().c_str() );
  if ( ! istr ) {
    return string();
  }
  return Digest::digest( "MD5", istr );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PathInfo::sha1sum
//	METHOD TYPE : std::string
//
std::string PathInfo::sha1sum( const Pathname & file )
{
  if ( ! PathInfo( file ).isFile() ) {
    return string();
  }
  ifstream istr( file.asString().c_str() );
  if ( ! istr ) {
    return string();
  }
  return Digest::digest( "SHA1", istr );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PathInfo::erase
//	METHOD TYPE : int
//
//	DESCRIPTION :
//
int PathInfo::erase( const Pathname & path )
{
  int res = 0;
  PathInfo p( path, LSTAT );
  if ( p.isExist() )
    {
      if ( p.isDir() )
        res = PathInfo::recursive_rmdir( path );
      else
        res = PathInfo::unlink( path );
    }
  return res;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PathInfo::chmod
//	METHOD TYPE : int
//
int PathInfo::chmod( const Pathname & path, mode_t mode )
{
  DBG << "chmod " << path << ' ' << stringutil::octstring( mode );
  if ( ::chmod( path.asString().c_str(), mode ) == -1 ) {
    return _Log_Result( errno );
  }
  return _Log_Result( 0 );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PathInfo::zipType
//	METHOD TYPE : PathInfo::ZIP_TYPE
//
PathInfo::ZIP_TYPE PathInfo::zipType( const Pathname & file )
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
