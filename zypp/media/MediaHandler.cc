/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaHandler.cc
 *
*/

#include <iostream>
#include <fstream>
#include <sstream>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include "zypp/media/MediaHandler.h"
#include "zypp/media/MediaManager.h"

using namespace std;

// use directory.yast on every media (not just via ftp/http)
#define NONREMOTE_DIRECTORY_YAST 1

namespace zypp {
  namespace media {



///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : MediaHandler
//
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::MediaHandler
//	METHOD TYPE : Constructor
//
//	DESCRIPTION :
//
MediaHandler::MediaHandler ( const Url &      url_r,
			     const Pathname & attach_point_r,
			     const Pathname & urlpath_below_attachpoint_r,
			     const bool       does_download_r )
    : _attachPoint( new AttachPoint())
    , _relativeRoot( urlpath_below_attachpoint_r)
    , _does_download( does_download_r )
    , _isAttached( false )
    , _url( url_r )
{
  if ( !attach_point_r.empty() ) {
    ///////////////////////////////////////////////////////////////////
    // check if provided attachpoint is usable.
    ///////////////////////////////////////////////////////////////////

    PathInfo adir( attach_point_r );
    // FIXME: verify if attach_point_r isn't a mountpoint of other device
    if ( !adir.isDir() ) {
      ERR << "Provided attach point is not a directory: " << adir << endl;
    }
    else {
      setAttachPoint( attach_point_r, false);
    }
  }
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::~MediaHandler
//	METHOD TYPE : Destructor
//
//	DESCRIPTION :
//
MediaHandler::~MediaHandler()
{
  removeAttachPoint();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::removeAttachPoint
//	METHOD TYPE : void
//
//	DESCRIPTION :
//
void
MediaHandler::removeAttachPoint()
{
  if ( _isAttached ) {
    INT << "MediaHandler deleted with media attached." << endl;
    return; // no cleanup if media still mounted!
  }

  if ( _attachPoint.unique() &&
       _attachPoint->temp    &&
       !_attachPoint->path.empty())
  {
    int res = recursive_rmdir( _attachPoint->path );
    if ( res == 0 ) {
      MIL << "Deleted default attach point " << _attachPoint->path << endl;
    } else {
      ERR << "Failed to Delete default attach point " << _attachPoint->path
	<< " errno(" << res << ")" << endl;
    }
  }
}


///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::attachPoint
//	METHOD TYPE : Pathname
//
//	DESCRIPTION :
//
Pathname
MediaHandler::attachPoint() const
{
  return _attachPoint->path;
}


///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::attachPoint
//	METHOD TYPE :
//
//	DESCRIPTION :
//
void
MediaHandler::setAttachPoint(const Pathname &path, bool temporary)
{
  _localRoot = Pathname();

  _attachPoint.reset( new AttachPoint(path, temporary));

  if( !_attachPoint->path.empty())
    _localRoot = _attachPoint->path + _relativeRoot;
}


///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::attachPoint
//	METHOD TYPE :
//
//	DESCRIPTION :
//
void
MediaHandler::setAttachPoint(const AttachPointRef &ref)
{
  _localRoot = Pathname();

  if( ref)
    AttachPointRef(ref).swap(_attachPoint);
  else
    _attachPoint.reset( new AttachPoint());

  if( !_attachPoint->path.empty())
    _localRoot = _attachPoint->path + _relativeRoot;
}


///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::findAttachedMedia
//	METHOD TYPE : AttachedMedia
//
//	DESCRIPTION :
//
AttachedMedia
MediaHandler::findAttachedMedia(const MediaSourceRef &media) const
{
	return MediaManager().findAttachedMedia(media);
}


///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::attach
//	METHOD TYPE : Pathname
//
//	DESCRIPTION :
//
Pathname
MediaHandler::createAttachPoint() const
{
  /////////////////////////////////////////////////////////////////
  // provide a default (temporary) attachpoint
  /////////////////////////////////////////////////////////////////
  const char * defmounts[] = {
      "/var/adm/mount", "/var/tmp", /**/NULL/**/
  };

  Pathname aroot;
  PathInfo adir;
  for ( const char ** def = defmounts; *def; ++def ) {
    adir( *def );
    if ( adir.isDir() && adir.userMayRWX() ) {
      aroot = adir.path();
      break;
    }
  }
  if ( aroot.empty() ) {
    ERR << "Create attach point: Can't find a writable directory to create an attach point" << std::endl;
    return aroot;
  }

  Pathname abase( aroot + "AP_" );
  Pathname apoint;
  for ( unsigned i = 1; i < 1000; ++i ) {
    adir( Pathname::extend( abase, str::hexstring( i ) ) );
    if ( ! adir.isExist() && mkdir( adir.path() ) == 0 ) {
            apoint = adir.path();
            break;
    }
  }
  if ( apoint.empty() ) {
    ERR << "Unable to create an attach point below " << aroot << std::endl;
  } else {
    MIL << "Created default attach point " << apoint << std::endl;
  }
  return apoint;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::attachedMedia
//	METHOD TYPE : AttachedMedia
//
//	DESCRIPTION :
//
AttachedMedia
MediaHandler::attachedMedia() const
{
  if ( _isAttached && _mediaSource && _attachPoint)
    return AttachedMedia(_mediaSource, _attachPoint);
  else
    return AttachedMedia();
}


///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::attach
//	METHOD TYPE : PMError
//
//	DESCRIPTION :
//
void MediaHandler::attach( bool next )
{
  if ( _isAttached )
    return;

/**
  if ( _attachPoint->empty() ) {
    ERR << "Bad Attachpoint" << endl;
    ZYPP_THROW( MediaBadAttachPointException(url()));
  }
*/

  attachTo( next ); // pass to concrete handler
  _isAttached = true;
  MIL << "Attached: " << *this << endl;
}




///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::localPath
//	METHOD TYPE : Pathname
//
Pathname MediaHandler::localPath( const Pathname & pathname ) const {
    if ( _localRoot.empty() )
        return _localRoot;

    // we must check maximum file name length
    // this is important for fetching the suseservers, the
    // url with all parameters can get too long (bug #42021)

    return _localRoot + pathname.absolutename();
}





///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::disconnect
//	METHOD TYPE : PMError
//
void MediaHandler::disconnect()
{
  if ( !_isAttached )
    return;

  disconnectFrom(); // pass to concrete handler
  MIL << "Disconnected: " << *this << endl;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::release
//	METHOD TYPE : PMError
//
//	DESCRIPTION :
//
void MediaHandler::release( bool eject )
{
  if ( !_isAttached ) {
    if ( eject )
      forceEject();
    return;
  }

  releaseFrom( eject ); // pass to concrete handler
  _isAttached = false;
  MIL << "Released: " << *this << endl;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::provideFile
//	METHOD TYPE : PMError
//
//	DESCRIPTION :
//
void MediaHandler::provideFileCopy( Pathname srcFilename,
                                       Pathname targetFilename ) const
{
  if ( !_isAttached ) {
    INT << "Media not_attached on provideFileCopy(" << srcFilename
        << "," << targetFilename << ")" << endl;
    ZYPP_THROW(MediaNotAttachedException(url()));
  }

  getFileCopy( srcFilename, targetFilename ); // pass to concrete handler
  DBG << "provideFileCopy(" << srcFilename << "," << targetFilename  << ")" << endl;
}

void MediaHandler::provideFile( Pathname filename ) const
{
  if ( !_isAttached ) {
    INT << "Error: Not attached on provideFile(" << filename << ")" << endl;
    ZYPP_THROW(MediaNotAttachedException(url()));
  }

  getFile( filename ); // pass to concrete handler
  DBG << "provideFile(" << filename << ")" << endl;
}


///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::provideDir
//	METHOD TYPE : PMError
//
//	DESCRIPTION :
//
void MediaHandler::provideDir( Pathname dirname ) const
{
  if ( !_isAttached ) {
    INT << "Error: Not attached on provideDir(" << dirname << ")" << endl;
    ZYPP_THROW(MediaNotAttachedException(url()));
  }

  getDir( dirname, /*recursive*/false ); // pass to concrete handler
  MIL << "provideDir(" << dirname << ")" << endl;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::provideDirTree
//	METHOD TYPE : PMError
//
//	DESCRIPTION :
//
void MediaHandler::provideDirTree( Pathname dirname ) const
{
  if ( !_isAttached ) {
    INT << "Error Not attached on provideDirTree(" << dirname << ")" << endl;
    ZYPP_THROW(MediaNotAttachedException(url()));
  }

  getDir( dirname, /*recursive*/true ); // pass to concrete handler
  MIL << "provideDirTree(" << dirname << ")" << endl;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::releasePath
//	METHOD TYPE : PMError
//
//	DESCRIPTION :
//
void MediaHandler::releasePath( Pathname pathname ) const
{
  if ( ! _does_download || _attachPoint->empty() )
    return;

  PathInfo info( localPath( pathname ) );

  if ( info.isFile() ) {
    unlink( info.path() );
  } else if ( info.isDir() ) {
    if ( info.path() != _localRoot ) {
      recursive_rmdir( info.path() );
    } else {
      clean_dir( info.path() );
    }
  }
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::dirInfo
//	METHOD TYPE : PMError
//
//	DESCRIPTION :
//
void MediaHandler::dirInfo( list<string> & retlist,
			       const Pathname & dirname, bool dots ) const
{
  retlist.clear();

  if ( !_isAttached ) {
    INT << "Error: Not attached on dirInfo(" << dirname << ")" << endl;
    ZYPP_THROW(MediaNotAttachedException(url()));
  }

  getDirInfo( retlist, dirname, dots ); // pass to concrete handler
  MIL << "dirInfo(" << dirname << ")" << endl;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::dirInfo
//	METHOD TYPE : PMError
//
//	DESCRIPTION :
//
void MediaHandler::dirInfo( filesystem::DirContent & retlist,
                            const Pathname & dirname, bool dots ) const
{
  retlist.clear();

  if ( !_isAttached ) {
    INT << "Error: Not attached on dirInfo(" << dirname << ")" << endl;
    ZYPP_THROW(MediaNotAttachedException(url()));
  }

  getDirInfo( retlist, dirname, dots ); // pass to concrete handler
  MIL << "dirInfo(" << dirname << ")" << endl;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::getDirectoryYast
//	METHOD TYPE : PMError
//
void MediaHandler::getDirectoryYast( std::list<std::string> & retlist,
					const Pathname & dirname, bool dots ) const
{
  retlist.clear();

  filesystem::DirContent content;
  getDirectoryYast( content, dirname, dots );

  // convert to std::list<std::string>
  for ( filesystem::DirContent::const_iterator it = content.begin(); it != content.end(); ++it ) {
    retlist.push_back( it->name );
  }
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::getDirectoryYast
//	METHOD TYPE : PMError
//
void MediaHandler::getDirectoryYast( filesystem::DirContent & retlist,
                                     const Pathname & dirname, bool dots ) const
{
  retlist.clear();

  // look for directory.yast
  Pathname dirFile = dirname + "directory.yast";
  getFile( dirFile );
  DBG << "provideFile(" << dirFile << "): " << "OK" << endl;

  // using directory.yast
  ifstream dir( localPath( dirFile ).asString().c_str() );
  if ( dir.fail() ) {
    ERR << "Unable to load '" << localPath( dirFile ) << "'" << endl;
    ZYPP_THROW(MediaSystemException(url(),
      "Unable to load '" + localPath( dirFile ).asString() + "'"));
  }

  string line;
  while( getline( dir, line ) ) {
    if ( line.empty() ) continue;
    if ( line == "directory.yast" ) continue;

    // Newer directory.yast append '/' to directory names
    // Remaining entries are unspecified, although most probabely files.
    filesystem::FileType type = filesystem::FT_NOT_AVAIL;
    if ( *line.rbegin() == '/' ) {
      line.erase( line.end()-1 );
      type = filesystem::FT_DIR;
    }

    if ( dots ) {
      if ( line == "." || line == ".." ) continue;
    } else {
      if ( *line.begin() == '.' ) continue;
    }

    retlist.push_back( filesystem::DirEntry( line, type ) );
  }
}

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : ostream &
*/
ostream & operator<<( ostream & str, const MediaHandler & obj )
{
  str << obj.url() << ( obj.isAttached() ? "" : " not" )
    << " attached; localRoot \"" << obj.localRoot() << "\"";
  return str;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::getFile
//	METHOD TYPE : PMError
//
//	DESCRIPTION : Asserted that media is attached.
//                    Default implementation of pure virtual.
//
void MediaHandler::getFile( const Pathname & filename ) const
{
    PathInfo info( localPath( filename ) );
    if( info.isFile() ) {
        return;
    }

    if (info.isExist())
      ZYPP_THROW(MediaNotAFileException(url(), localPath(filename)));
    else
      ZYPP_THROW(MediaFileNotFoundException(url(), filename));
}


void MediaHandler::getFileCopy ( const Pathname & srcFilename, const Pathname & targetFilename ) const
{
  getFile(srcFilename);

  if ( copy( localPath( srcFilename ), targetFilename ) != 0 ) {
    ZYPP_THROW(MediaWriteException(targetFilename));
  }
}



///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::getDir
//	METHOD TYPE : PMError
//
//	DESCRIPTION : Asserted that media is attached.
//                    Default implementation of pure virtual.
//
void MediaHandler::getDir( const Pathname & dirname, bool recurse_r ) const
{
  PathInfo info( localPath( dirname ) );
  if( info.isDir() ) {
    return;
  }

  if (info.isExist())
    ZYPP_THROW(MediaNotADirException(url(), localPath(dirname)));
  else
    ZYPP_THROW(MediaFileNotFoundException(url(), dirname));
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::getDirInfo
//	METHOD TYPE : PMError
//
//	DESCRIPTION : Asserted that media is attached and retlist is empty.
//                    Default implementation of pure virtual.
//
void MediaHandler::getDirInfo( std::list<std::string> & retlist,
                               const Pathname & dirname, bool dots ) const
{
  PathInfo info( localPath( dirname ) );
  if( ! info.isDir() ) {
    ZYPP_THROW(MediaNotADirException(url(), localPath(dirname)));
  }

#if NONREMOTE_DIRECTORY_YAST
  // use directory.yast if available
  try {
    getDirectoryYast( retlist, dirname, dots );
  }
  catch (const MediaException & excpt_r)
  {
#endif

  // readdir
    int res = readdir( retlist, info.path(), dots );
  if ( res )
    ZYPP_THROW(MediaSystemException(url(), "readdir failed"));

#if NONREMOTE_DIRECTORY_YAST
  }
#endif

  return;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::getDirInfo
//	METHOD TYPE : PMError
//
//	DESCRIPTION : Asserted that media is attached and retlist is empty.
//                    Default implementation of pure virtual.
//
void MediaHandler::getDirInfo( filesystem::DirContent & retlist,
                               const Pathname & dirname, bool dots ) const
{
  PathInfo info( localPath( dirname ) );
  if( ! info.isDir() ) {
    ZYPP_THROW(MediaNotADirException(url(), localPath(dirname)));
  }

#if NONREMOTE_DIRECTORY_YAST
  // use directory.yast if available
  try {
    getDirectoryYast( retlist, dirname, dots );
  }
  catch (const MediaException & excpt_r)
  {
#endif

  // readdir
  int res = readdir( retlist, info.path(), dots );
  if ( res )
    ZYPP_THROW(MediaSystemException(url(), "readdir failed"));
#if NONREMOTE_DIRECTORY_YAST
  }
#endif
}

  } // namespace media
} // namespace zypp
