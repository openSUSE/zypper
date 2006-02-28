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
#include "zypp/media/Mount.h"

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
    , _attach_mtime(0)
    , _url( url_r )
    , _parentId(0)
    , _AttachPointHint()
{
  if ( !attach_point_r.empty() ) {
    ///////////////////////////////////////////////////////////////////
    // check if provided attachpoint is usable.
    ///////////////////////////////////////////////////////////////////

    PathInfo adir( attach_point_r );
    // FIXME: verify if attach_point_r isn't a mountpoint of other device
    if ( !adir.isDir() || !attach_point_r.absolute()) {
      ERR << "Provided attach point is not a directory: " << adir << endl;
    }
    else {
      attachPointHint( attach_point_r, false);
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
  try
    {
      removeAttachPoint();
    }
  catch(...) {}
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
  if ( _mediaSource ) {
    INT << "MediaHandler deleted with media attached." << endl;
    return; // no cleanup if media still mounted!
  }

  DBG << "MediaHandler - checking if to remove attach point" << endl;
  if ( _attachPoint.unique() &&
       _attachPoint->temp    &&
       !_attachPoint->path.empty() &&
       PathInfo(_attachPoint->path).isDir())
  {
    Pathname path(_attachPoint->path);

    setAttachPoint("", true);

    int res = recursive_rmdir( path );
    if ( res == 0 ) {
      MIL << "Deleted default attach point " << path << endl;
    } else {
      ERR << "Failed to Delete default attach point " << path
	<< " errno(" << res << ")" << endl;
    }
  }
  else
  {
    if( !_attachPoint->temp)
      DBG << "MediaHandler - attachpoint is not temporary" << endl;
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
  _attachPoint.reset( new AttachPoint(path, temporary));
}

Pathname
MediaHandler::localRoot() const
{
  if( _attachPoint->path.empty())
    return Pathname();
  else
    return _attachPoint->path + _relativeRoot;
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
  if( ref)
    AttachPointRef(ref).swap(_attachPoint);
  else
    _attachPoint.reset( new AttachPoint());
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::attachPointHint
//	METHOD TYPE : void
//
//	DESCRIPTION :
//
void
MediaHandler::attachPointHint(const Pathname &path, bool temporary)
{
  _AttachPointHint.path = path;
  _AttachPointHint.temp = temporary;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::attachPointHint
//	METHOD TYPE : AttachPoint
//
//	DESCRIPTION :
//
AttachPoint
MediaHandler::attachPointHint() const
{
  return _AttachPointHint;
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

  Pathname apoint;
  Pathname aroot;
  for ( const char ** def = defmounts; *def && apoint.empty(); ++def ) {
    aroot = *def;
    if( aroot.empty())
      continue;

    apoint = createAttachPoint(aroot);
  }

  if ( aroot.empty() ) {
    ERR << "Create attach point: Can't find a writable directory to create an attach point" << std::endl;
    return aroot;
  }

  if ( !apoint.empty() ) {
    MIL << "Created default attach point " << apoint << std::endl;
  }
  return apoint;
}

Pathname
MediaHandler::createAttachPoint(const Pathname &attach_root) const
{
  Pathname apoint;

  if( attach_root.empty() || !attach_root.absolute()) {
    ERR << "Create attach point: invalid attach root: '"
        << attach_root << "'" << std::endl;
    return apoint;
  }

  PathInfo adir( attach_root);
  if( !adir.isDir() || !adir.userMayRWX()) {
    DBG << "Create attach point: attach root is not a writable directory: '"
        << attach_root << "'" << std::endl;
    return apoint;
  }

  DBG << "Trying to create attach point in " << attach_root << std::endl;

  //
  // FIXME: use mkdtemp?
  //
  Pathname abase( attach_root + "AP_" );
  //        ma and sh need more than 42 for debugging :-)
  //        since the readonly fs are handled now, ...
  for ( unsigned i = 1; i < 1000; ++i ) {
    adir( Pathname::extend( abase, str::hexstring( i ) ) );
    if ( ! adir.isExist() ) {
      int err = mkdir( adir.path() );
      if (err == 0 ) {
        apoint = adir.path();
        break;
      }
      else
      if (err != EEXIST)	// readonly fs or other, dont try further
        break;
    }
  }

  if ( apoint.empty()) {
    ERR << "Unable to create an attach point below of "
        << attach_root << std::endl;
  }
  return apoint;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::isUseableAttachPoint
//	METHOD TYPE : bool
//
//	DESCRIPTION :
//
bool
MediaHandler::isUseableAttachPoint(const Pathname &path) const
{
  MediaManager  manager;
  return manager.isUseableAttachPoint(path);
}


///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::setMediaSource
//	METHOD TYPE : void
//
//	DESCRIPTION :
//
void
MediaHandler::setMediaSource(const MediaSourceRef &ref)
{
  _mediaSource.reset();
  if( ref && !ref->type.empty() && !ref->name.empty())
    _mediaSource = ref;
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
  if ( _mediaSource && _attachPoint)
    return AttachedMedia(_mediaSource, _attachPoint);
  else
    return AttachedMedia();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::isSharedMedia
//	METHOD TYPE : bool
//
//	DESCRIPTION :
//
bool
MediaHandler::isSharedMedia() const
{
  return !_mediaSource.unique();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::checkAttached
//	METHOD TYPE : bool
//
//	DESCRIPTION :
//
bool
MediaHandler::checkAttached(bool isDevice,  bool fsType) const
{
  bool _isAttached = false;

  AttachedMedia ref( attachedMedia());
  if( ref.mediaSource)
  {
    MediaManager  manager;

    time_t old = _attach_mtime;
    _attach_mtime = manager.getMountTableMTime();
    if( !(old <= 0 || _attach_mtime != old))
    {
      // OK, skip the check
      _isAttached = true;
    }
    else
    {
      DBG << "Mount table changed - rereading it" << std::endl;
      MountEntries  entries( manager.getMountEntries());
      MountEntries::const_iterator e;
      for( e = entries.begin(); e != entries.end(); ++e)
      {
        bool is_device = (Pathname(e->src).dirname() == "/dev");
        if( is_device == isDevice)
        {
          PathInfo    dinfo(e->src);
          std::string mtype(fsType ? e->type : ref.mediaSource->type);
          MediaSource media(mtype, e->src, dinfo.major(), dinfo.minor());

          if( ref.mediaSource->equals( media) &&
              ref.attachPoint->path == Pathname(e->dir))
          {
            DBG << "Found media "
                << ref.mediaSource->asString()
                << " in the mount table" << std::endl;
            _isAttached = true;
            break;
          }
          // differs
        }
        else
        {
          std::string mtype(fsType ? e->type : ref.mediaSource->type);
          MediaSource media(mtype, e->src);
          if( ref.mediaSource->equals( media) &&
              ref.attachPoint->path == Pathname(e->dir))
          {
            DBG << "Found media "
                << ref.mediaSource->asString()
                << " in the mount table" << std::endl;
            _isAttached = true;
            break;
          }
          // differs
        }
      }

      if( !_isAttached)
      {
        ERR << "Attached media not in mount entry table any more"
            << std::endl;
      }
    }
  }
  return _isAttached;
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
  if ( isAttached() )
    return;

  // reset it in case of overloaded isAttached()
  // that checks the media against /etc/mtab ...
  setMediaSource(MediaSourceRef());

  attachTo( next ); // pass to concrete handler
  MIL << "Attached: " << *this << endl;
}




///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaHandler::localPath
//	METHOD TYPE : Pathname
//
Pathname MediaHandler::localPath( const Pathname & pathname ) const
{
    Pathname _localRoot( localRoot());
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
  if ( !isAttached() )
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
  if ( !isAttached() ) {
    DBG << "Request to release media - not attached" << std::endl;
    if ( eject )
      forceEject();
    return;
  }

  DBG << "Request to release attached media "
      << _mediaSource->asString()
      << ", use count=" << _mediaSource.use_count()
      << std::endl;

  if( _mediaSource.unique())
  {
    DBG << "Releasing media " << _mediaSource->asString() << std::endl;
    releaseFrom( eject ); // pass to concrete handler
    _mediaSource.reset(NULL);
    removeAttachPoint();
  }
  else if( eject) {
    //
    // Can't eject a shared media
    //
    //ZYPP_THROW(MediaIsSharedException(_mediaSource->asString()));

    MediaSourceRef media( new MediaSource(*_mediaSource));
    _mediaSource.reset(NULL);

    MediaManager manager;
    manager.forceMediaRelease(media);

    setMediaSource(media);
    DBG << "Releasing media (forced) " << _mediaSource->asString() << std::endl;
    releaseFrom( eject ); // pass to concrete handler
    _mediaSource.reset(NULL);
    removeAttachPoint();
  }
  else {
    DBG << "Releasing shared media reference only" << std::endl;
    _mediaSource.reset(NULL);
    //setAttachPoint("", true);
  }
  MIL << "Released: " << *this << endl;
}

bool
MediaHandler::checkAttachPoint(const Pathname &apoint) const
{
  return checkAttachPoint( apoint, true, false);
}

bool
MediaHandler::checkAttachPoint(const Pathname &apoint,
			       bool            emptydir,
	                       bool            writeable) const
{
  if( apoint.empty() || !apoint.absolute())
  {
    ERR << "Attach point '" << apoint << "' is not absolute"
        << std::endl;
    return false;
  }

  PathInfo ainfo(apoint);
  if( !ainfo.isDir())
  {
    ERR << "Attach point '" << apoint << "' is not a directory"
        << std::endl;
    return false;
  }

  if( emptydir)
  {
    if( 0 != zypp::filesystem::is_empty_dir(apoint))
    {
      ERR << "Attach point '" << apoint << "' is not a empty directory"
          << std::endl;
      return false;
    }
  }

  if( writeable)
  {
    Pathname apath(apoint + "XXXXXX");
    char    *atemp = ::strdup( apath.asString().c_str());
    char    *atest = NULL;
    if( !ainfo.userMayRWX() || atemp == NULL ||
        (atest=::mkdtemp(atemp)) == NULL)
    {
      if( atemp != NULL)
	::free(atemp);

      ERR << "Attach point '" << ainfo.path()
          << "' is not a writeable directory" << std::endl;
      return false;
    }
    else if( atest != NULL)
      ::rmdir(atest);

    if( atemp != NULL)
      ::free(atemp);
  }
  return true;
}

void
MediaHandler::reattach(const Pathname &attach_point,
                       bool            temporary)
{
  // ignore if it is equal to current attach point hint
  AttachPoint hint( attachPointHint());
  if( hint.temp == temporary && hint.path == attach_point)
  {
    DBG << "Ignored reattach - equals to current one" << std::endl;
    return;
  }

  if( temporary)
  {
    if( !attach_point.empty())
    {
      // check if the new attach point root hint is
      // a writable directory; may contains files.
      if( !checkAttachPoint(attach_point, false, true))
        ZYPP_THROW( MediaBadAttachPointException(url()));
    }
  }
  else
  {
    // use attach_point as non-temporary attach point
    if( !checkAttachPoint(attach_point))
      ZYPP_THROW( MediaBadAttachPointException(url()));

    if( !isUseableAttachPoint(attach_point))
      ZYPP_THROW( MediaBadAttachPointException(url()));
  }

  // pass new hint to specific handler
  reattachTo(attach_point, temporary);
}

void
MediaHandler::reattachTo(const Pathname &attach_point,
                         bool            temporary)
{
  if( !isAttached())
  {
    DBG << "Accepted reattach to '"
        << attach_point
        << "' -- as hint for the next attach"
	<< std::endl;

    attachPointHint(attach_point, temporary);
  }
  else
  {
    DBG << "Ignoring reattach to '" << attach_point
        << "'" << (temporary ? " (attach point root)" : "")
	<< " not supported by this access handler."
        << std::endl;
  }
}


///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : MediaHandler::dependsOnParent
//	METHOD TYPE : bool
//
//	DESCRIPTION :
//
bool
MediaHandler::dependsOnParent(MediaAccessId parentId)
{
  if( _parentId != 0)
  {
    if(parentId == _parentId)
      return true;
 
    MediaManager mm;
    AttachedMedia am1 = mm.getAttachedMedia(_parentId);
    AttachedMedia am2 = mm.getAttachedMedia(parentId);
    if( am1.mediaSource && am2.mediaSource)
    {
      return am1.mediaSource->equals( *(am2.mediaSource));
    }
  }
  return false;
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
  if ( !isAttached() ) {
    INT << "Media not_attached on provideFileCopy(" << srcFilename
        << "," << targetFilename << ")" << endl;
    ZYPP_THROW(MediaNotAttachedException(url()));
  }

  getFileCopy( srcFilename, targetFilename ); // pass to concrete handler
  DBG << "provideFileCopy(" << srcFilename << "," << targetFilename  << ")" << endl;
}

void MediaHandler::provideFile( Pathname filename ) const
{
  if ( !isAttached() ) {
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
  if ( !isAttached() ) {
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
  if ( !isAttached() ) {
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
    if ( info.path() != localRoot() ) {
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

  if ( !isAttached() ) {
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

  if ( !isAttached() ) {
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
// vim: set ts=8 sts=2 sw=2 ai noet:
