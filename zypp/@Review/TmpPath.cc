/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                         (C) SuSE Linux Products GmbH |
\----------------------------------------------------------------------/

  File:       TmpPath.cc

  Author:     Michael Andres <ma@suse.de>
  Maintainer: Michael Andres <ma@suse.de>

  Purpose: Provide temporary files/directories, automaticaly
           deleted when no longer needed.

/-*/

#include <cstdlib>
#include <cstring>
#include <cerrno>

#include <iostream>

#include <y2util/Y2SLog.h>
#include <y2util/PathInfo.h>
#include <y2util/TmpPath.h>

using namespace std;

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : TmpPath::Impl
/**
 * Clean or delete a directory on destruction.
 **/
class TmpPath::Impl :public Rep
{
  public:

    enum Flags
      {
        NoOp         = 0,
        Autodelete   = 1L << 0,
        KeepTopdir   = 1L << 1,
        //
        CtorDefault  = Autodelete
      };

  public:

    Impl( const Pathname & path_r, Flags flags_r = CtorDefault )
    : _path( path_r ), _flags( flags_r )
    {}

    ~Impl()
    {
      if ( ! (_flags & Autodelete) || _path.empty() )
        return;

      PathInfo p( _path, PathInfo::LSTAT );
      if ( ! p.isExist() )
        return;

      int res = 0;
      if ( p.isDir() )
        {
          if ( _flags & KeepTopdir )
            res = PathInfo::clean_dir( _path );
          else
            res = PathInfo::recursive_rmdir( _path );
        }
      else
        res = PathInfo::unlink( _path );

      if ( res )
        INT << "TmpPath cleanup error (" << res << ") " << p << endl;
      else
        DBG << "TmpPath cleaned up " << p << endl;
    }

    const Pathname &
    path() const
    { return _path; }

  private:
    Pathname _path;
    Flags    _flags;
};
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : TmpPath
//
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : TmpPath::TmpPath
//	METHOD TYPE : Constructor
//
TmpPath::TmpPath()
:_impl( 0 ) // empty Pathname
{
}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : TmpPath::TmpPath
//	METHOD TYPE : Constructor
//
TmpPath::TmpPath( const Pathname & tmpPath_r )
:_impl( tmpPath_r.empty() ? 0 : new Impl( tmpPath_r ) )
{
}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : TmpPath::~TmpPath
//	METHOD TYPE : Destructor
//
TmpPath::~TmpPath()
{
  // virtual not inlined dtor.
}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : TmpPath::operator const void *const
//	METHOD TYPE :
//
TmpPath::operator const void *const() const
{
  return _impl;
}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : TmpPath::path
//	METHOD TYPE : Pathname
//
Pathname
TmpPath::path() const
{
  return _impl ? _impl->path() : Pathname();
}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : TmpPath::defaultLocation
//	METHOD TYPE : const Pathname &
//
const Pathname &
TmpPath::defaultLocation()
{
  static Pathname p( "/var/tmp" );
  return p;
}
///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : TmpFile
//
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : TmpFile::TmpFile
//	METHOD TYPE : Constructor
//
TmpFile::TmpFile( const Pathname & inParentDir_r,
                  const std::string & prefix_r )
{
  // parent dir must exist
  PathInfo p( inParentDir_r );
  if ( ! p.isDir() )
    {
      ERR << "Parent directory does not exist: " << p << endl;
      return;
    }

  // create the temp file
  Pathname tmpPath = (inParentDir_r + prefix_r).extend( "XXXXXX");
  char * buf = ::strdup( tmpPath.asString().c_str() );
  if ( ! buf )
    {
      ERR << "Out of memory" << endl;
      return;
    }

  int tmpFd = ::mkstemp( buf );
  if ( tmpFd != -1 )
    {
      // success; create _impl
      ::close( tmpFd );
      _impl = makeVarPtr( new Impl( buf ) );
    }
  else
    ERR << "Cant create '" << buf << "' " << ::strerror( errno ) << endl;

  ::free( buf );
}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : TmpFile::defaultPrefix
//	METHOD TYPE : const std::string &
//
const std::string &
TmpFile::defaultPrefix()
{
  static string p( "TmpFile." );
  return p;
}

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : TmpDir
//
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : TmpDir::TmpDir
//	METHOD TYPE : Constructor
//
TmpDir::TmpDir( const Pathname & inParentDir_r,
                const std::string & prefix_r )
{
  // parent dir must exist
  PathInfo p( inParentDir_r );
  if ( ! p.isDir() )
    {
      ERR << "Parent directory does not exist: " << p << endl;
      return;
    }

  // create the temp dir
  Pathname tmpPath = (inParentDir_r + prefix_r).extend( "XXXXXX");
  char * buf = ::strdup( tmpPath.asString().c_str() );
  if ( ! buf )
    {
      ERR << "Out of memory" << endl;
      return;
    }

  char * tmp = ::mkdtemp( buf );
  if ( tmp )
    // success; create _impl
    _impl = makeVarPtr( new Impl( tmp ) );
  else
    ERR << "Cant create '" << tmpPath << "' " << ::strerror( errno ) << endl;

  ::free( buf );
}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : TmpDir::defaultPrefix
//	METHOD TYPE : const std::string &
//
const std::string &
TmpDir::defaultPrefix()
{
  static string p( "TmpDir." );
  return p;
}
