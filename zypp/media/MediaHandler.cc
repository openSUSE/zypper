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

#include "zypp/TmpPath.h"
#include "zypp/Date.h"
#include "zypp/base/LogTools.h"
#include "zypp/base/String.h"
#include "zypp/media/MediaHandler.h"
#include "zypp/media/MediaManager.h"
#include "zypp/media/Mount.h"
#include <limits.h>
#include <stdlib.h>
#include <errno.h>


using namespace std;

// use directory.yast on every media (not just via ftp/http)
#define NONREMOTE_DIRECTORY_YAST 1

namespace zypp {
  namespace media {

  Pathname MediaHandler::_attachPrefix("");

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
    : _mediaSource()
    , _attachPoint( new AttachPoint())
    , _AttachPointHint()
    , _relativeRoot( urlpath_below_attachpoint_r)
    , _does_download( does_download_r )
    , _attach_mtime(0)
    , _url( url_r )
    , _parentId(0)
{
  Pathname real_attach_point( getRealPath(attach_point_r.asString()));

  if ( !real_attach_point.empty() ) {
    ///////////////////////////////////////////////////////////////////
    // check if provided attachpoint is usable.
    ///////////////////////////////////////////////////////////////////

    PathInfo adir( real_attach_point );
    //
    // The verify if attach_point_r isn't a mountpoint of another
    // device is done in the particular media handler (if needed).
    //
    // We just verify, if attach_point_r is a directory and for
    // schemes other than "file" and "dir", if it is absolute.
    //
    if ( !adir.isDir()
	 || (_url.getScheme() != "file"
	     && _url.getScheme() != "dir"
	     && !real_attach_point.absolute()) )
    {
      ERR << "Provided attach point is not a absolute directory: "
          << adir << endl;
    }
    else {
      attachPointHint( real_attach_point, false);
      setAttachPoint( real_attach_point, false);
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

void
MediaHandler::resetParentId()
{
  _parentId = 0;
}

std::string
MediaHandler::getRealPath(const std::string &path)
{
  std::string real;
  if( !path.empty())
  {
#if __GNUC__ > 2
    /** GNU extension */
    char *ptr = ::realpath(path.c_str(), NULL);
    if( ptr != NULL)
    {
      real = ptr;
      free( ptr);
    }
    else
    /** the SUSv2 way */
    if( EINVAL == errno)
    {
      char buff[PATH_MAX + 2];
      memset(buff, '\0', sizeof(buff));
      if( ::realpath(path.c_str(), buff) != NULL)
      {
	real = buff;
      }
    }
#else
    char buff[PATH_MAX + 2];
    memset(buff, '\0', sizeof(buff));
    if( ::realpath(path.c_str(), buff) != NULL)
    {
      real = buff;
    }
#endif
  }
  return real;
}

zypp::Pathname
MediaHandler::getRealPath(const Pathname &path)
{
  return zypp::Pathname(getRealPath(path.asString()));
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
    if( !_attachPoint->path.empty() && !_attachPoint->temp)
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
//	METHOD NAME : MediaHandler::setAttachPrefix
//	METHOD TYPE : void
//
//	DESCRIPTION :
//
bool
MediaHandler::setAttachPrefix(const Pathname &attach_prefix)
{
  if( attach_prefix.empty())
  {
    MIL << "Reseting to built-in attach point prefixes."
        << std::endl;
    MediaHandler::_attachPrefix = attach_prefix;
    return true;
  }
  else
  if( MediaHandler::checkAttachPoint(attach_prefix, false, true))
  {
    MIL << "Setting user defined attach point prefix: "
        << attach_prefix << std::endl;
    MediaHandler::_attachPrefix = attach_prefix;
    return true;
  }
  return false;
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
      "/var/adm/mount", filesystem::TmpPath::defaultLocation().c_str(), /**/NULL/**/
  };

  Pathname apoint;
  Pathname aroot( MediaHandler::_attachPrefix);

  if( !aroot.empty())
  {
    apoint = createAttachPoint(aroot);
  }
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

  PathInfo adir( attach_root );
  if( !adir.isDir() || (geteuid() != 0 && !adir.userMayRWX())) {
    DBG << "Create attach point: attach root is not a writable directory: '"
        << attach_root << "'" << std::endl;
    return apoint;
  }

  static bool cleanup_once( true );
  if ( cleanup_once )
  {
    cleanup_once = false;
    DBG << "Look for orphaned attach points in " << adir << std::endl;
    std::list<std::string> entries;
    filesystem::readdir( entries, attach_root, false );
    for ( const std::string & entry : entries )
    {
      if ( ! str::hasPrefix( entry, "AP_0x" ) )
	continue;
      PathInfo sdir( attach_root + entry );
      if ( sdir.isDir()
	&& sdir.dev() == adir.dev()
	&& ( Date::now()-sdir.mtime() > Date::month ) )
      {
	DBG << "Remove orphaned attach point " << sdir << std::endl;
	filesystem::recursive_rmdir( sdir.path() );
      }
    }
  }

  filesystem::TmpDir tmpdir( attach_root, "AP_0x" );
  if ( tmpdir )
  {
    apoint = getRealPath( tmpdir.path().asString() );
    if ( ! apoint.empty() )
    {
      tmpdir.autoCleanup( false );	// Take responsibility for cleanup.
    }
    else
    {
      ERR << "Unable to resolve real path for attach point " << tmpdir << std::endl;
    }
  }
  else
  {
    ERR << "Unable to create attach point below " << attach_root << std::endl;
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
MediaHandler::isUseableAttachPoint(const Pathname &path, bool mtab) const
{
  MediaManager  manager;
  return manager.isUseableAttachPoint(path, mtab);
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
MediaHandler::checkAttached(bool matchMountFs) const
{
  bool _isAttached = false;

  AttachedMedia ref( attachedMedia());
  if( ref.mediaSource )
  {
    time_t old_mtime = _attach_mtime;
    _attach_mtime = MediaManager::getMountTableMTime();
    if( !(old_mtime <= 0 || _attach_mtime != old_mtime) )
    {
      // OK, skip the check (we've seen it at least once)
      _isAttached = true;
    }
    else
    {
      if( old_mtime > 0)
        DBG << "Mount table changed - rereading it" << std::endl;
      else
        DBG << "Forced check of the mount table" << std::endl;

      MountEntries entries( MediaManager::getMountEntries());
      for_( e, entries.begin(), entries.end() )
      {
        bool        is_device = false;
        PathInfo    dev_info;

        if( str::hasPrefix( Pathname(e->src).asString(), "/dev/" ) &&
            dev_info(e->src) && dev_info.isBlk())
        {
          is_device = true;
        }

        if( is_device &&  (ref.mediaSource->maj_nr &&
	                   ref.mediaSource->bdir.empty()))
        {
          std::string mtype(matchMountFs ? e->type : ref.mediaSource->type);
          MediaSource media(mtype, e->src, dev_info.major(), dev_info.minor());

          if( ref.mediaSource->equals( media) &&
              ref.attachPoint->path == Pathname(e->dir))
          {
            DBG << "Found media device "
                << ref.mediaSource->asString()
                << " in the mount table as " << e->src << std::endl;
            _isAttached = true;
            break;
          }
          // differs
        }
        else
        if(!is_device && (!ref.mediaSource->maj_nr ||
	                  !ref.mediaSource->bdir.empty()))
        {
	  if( ref.mediaSource->bdir.empty())
	  {
	    // bnc#710269: Type nfs may appear as nfs4 in in the mount table
	    // and maybe vice versa. Similar cifs/smb. Need to unify these types:
	    if ( matchMountFs && e->type != ref.mediaSource->type )
	    {
	      if ( str::hasPrefix( e->type, "nfs" ) && str::hasPrefix( ref.mediaSource->type, "nfs" ) )
		matchMountFs = false;
	      else if ( ( e->type == "cifs" || e->type == "smb" ) && ( ref.mediaSource->type == "cifs" || ref.mediaSource->type == "smb" ) )
		matchMountFs = false;
	    }
	    std::string mtype(matchMountFs ? e->type : ref.mediaSource->type);
	    MediaSource media(mtype, e->src);

	    if( ref.mediaSource->equals( media) &&
                ref.attachPoint->path == Pathname(e->dir))
	    {
	      DBG << "Found media name "
                  << ref.mediaSource->asString()
                  << " in the mount table as " << e->src << std::endl;
	      _isAttached = true;
	      break;
	    }
	  }
	  else
	  {
	    if(ref.mediaSource->bdir == e->src &&
	       ref.attachPoint->path == Pathname(e->dir))
	    {
	      DBG << "Found bound media "
	          << ref.mediaSource->asString()
		  << " in the mount table as " << e->src << std::endl;
	      _isAttached = true;
	      break;
	    }
	  }
          // differs
        }
      }

      if( !_isAttached)
      {
        MIL << "Looking for " << ref << endl;
	if( entries.empty() )
	{
	  ERR << "Unable to find any entry in the /etc/mtab file" << std::endl;
	}
	else
	{
          dumpRange( DBG << "MountEntries: ", entries.begin(), entries.end() ) << endl;
	}
	if( old_mtime > 0 )
	{
          ERR << "Attached media not in mount table any more - forcing reset!"
              << std::endl;

	  _mediaSource.reset();
	}
	else
	{
          WAR << "Attached media not in mount table ..." << std::endl;
	}

        // reset the mtime and force a new check to make sure,
        // that we've found the media at least once in the mtab.
        _attach_mtime = 0;
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

  AttachPoint ap( attachPointHint());
  setAttachPoint(ap.path, ap.temp);

  try
  {
    attachTo( next ); // pass to concrete handler
  }
  catch(const MediaException &e)
  {
    removeAttachPoint();
    ZYPP_RETHROW(e);
  }
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
void MediaHandler::release( const std::string & ejectDev )
{
  if ( !isAttached() ) {
    DBG << "Request to release media - not attached; eject '" << ejectDev << "'"
        << std::endl;
    if ( !ejectDev.empty() )
      forceEject(ejectDev);
    return;
  }

  DBG << "Request to release attached media "
      << _mediaSource->asString()
      << ", use count=" << _mediaSource.use_count()
      << std::endl;

  if( _mediaSource.unique())
  {
    DBG << "Releasing media " << _mediaSource->asString() << std::endl;
    try {
      releaseFrom( ejectDev ); // pass to concrete handler
    }
    catch(const MediaNotEjectedException &e)
    {
      // not ejected because the media
      // is mounted by somebody else
      // (if our attach point is busy,
      //  we get an umount exception)
      _mediaSource.reset(NULL);
      removeAttachPoint();
      // OK, retrow now
      ZYPP_RETHROW(e);
    }
    _mediaSource.reset(NULL);
    removeAttachPoint();
  }
  else if( !ejectDev.empty() ) {
    //
    // Can't eject a shared media
    //
    //ZYPP_THROW(MediaIsSharedException(_mediaSource->asString()));

    MediaSourceRef media( new MediaSource(*_mediaSource));
    _mediaSource.reset(NULL);

    MediaManager manager;
    manager.forceReleaseShared(media);

    setMediaSource(media);
    DBG << "Releasing media (forced) " << _mediaSource->asString() << std::endl;
    try {
      releaseFrom( ejectDev ); // pass to concrete handler
    }
    catch(const MediaNotEjectedException &e)
    {
      // not ejected because the media
      // is mounted by somebody else
      // (if our attach point is busy,
      //  we get an umount exception)
      _mediaSource.reset(NULL);
      removeAttachPoint();
      // OK, retrow now
      ZYPP_RETHROW(e);
    }
    _mediaSource.reset(NULL);
    removeAttachPoint();
  }
  else {
    DBG << "Releasing shared media reference only" << std::endl;
    _mediaSource.reset(NULL);
    setAttachPoint("", true);
  }
  MIL << "Released: " << *this << endl;
}

void MediaHandler::forceRelaseAllMedia(bool matchMountFs)
{
  forceRelaseAllMedia( attachedMedia().mediaSource, matchMountFs);
}

void MediaHandler::forceRelaseAllMedia(const MediaSourceRef &ref,
                                       bool                  matchMountFs)
{
  if( !ref)
    return;

  MountEntries  entries( MediaManager::getMountEntries());
  MountEntries::const_iterator e;
  for( e = entries.begin(); e != entries.end(); ++e)
  {
    bool        is_device = false;
    PathInfo    dev_info;

    if( str::hasPrefix( Pathname(e->src).asString(), "/dev/" ) &&
        dev_info(e->src) && dev_info.isBlk())
    {
      is_device = true;
    }

    if( is_device &&  ref->maj_nr)
    {
      std::string mtype(matchMountFs ? e->type : ref->type);
      MediaSource media(mtype, e->src, dev_info.major(), dev_info.minor());

      if( ref->equals( media) && e->type != "subfs")
      {
        DBG << "Forcing release of media device "
            << ref->asString()
            << " in the mount table as "
	    << e->src << std::endl;
	try {
	  Mount mount;
	  mount.umount(e->dir);
	}
	catch (const Exception &e)
	{
	  ZYPP_CAUGHT(e);
	}
      }
    }
    else
    if(!is_device && !ref->maj_nr)
    {
      std::string mtype(matchMountFs ? e->type : ref->type);
      MediaSource media(mtype, e->src);
      if( ref->equals( media))
      {
	DBG << "Forcing release of media name "
	    << ref->asString()
	    << " in the mount table as "
	    << e->src << std::endl;
	try {
	  Mount mount;
	  mount.umount(e->dir);
	}
	catch (const Exception &e)
	{
	  ZYPP_CAUGHT(e);
	}
      }
    }
  }
}

bool
MediaHandler::checkAttachPoint(const Pathname &apoint) const
{
  return MediaHandler::checkAttachPoint( apoint, true, false);
}

// STATIC
bool
MediaHandler::checkAttachPoint(const Pathname &apoint,
			       bool            emptydir,
	                       bool            writeable)
{
  if( apoint.empty() || !apoint.absolute())
  {
    ERR << "Attach point '" << apoint << "' is not absolute"
        << std::endl;
    return false;
  }
  if( apoint == "/")
  {
    ERR << "Attach point '" << apoint << "' is not allowed"
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

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : MediaHandler::dependsOnParent
//	METHOD TYPE : bool
//
//	DESCRIPTION :
//
bool
MediaHandler::dependsOnParent()
{
  return _parentId != 0;
}

bool
MediaHandler::dependsOnParent(MediaAccessId parentId, bool exactIdMatch)
{
  if( _parentId != 0)
  {
    if(parentId == _parentId)
      return true;

    if( !exactIdMatch)
    {
      MediaManager mm;
      AttachedMedia am1 = mm.getAttachedMedia(_parentId);
      AttachedMedia am2 = mm.getAttachedMedia(parentId);
      if( am1.mediaSource && am2.mediaSource)
      {
	return am1.mediaSource->equals( *(am2.mediaSource));
      }
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
void MediaHandler::dirInfo( std::list<std::string> & retlist,
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
//      METHOD NAME : MediaHandler::doesFileExist
//      METHOD TYPE : PMError
//
//      DESCRIPTION :
//
bool MediaHandler::doesFileExist( const Pathname & filename ) const
{
  // TODO do some logging
  if ( !isAttached() ) {
    INT << "Error Not attached on doesFileExist(" << filename << ")" << endl;
    ZYPP_THROW(MediaNotAttachedException(url()));
  }
  return getDoesFileExist( filename );
  MIL << "doesFileExist(" << filename << ")" << endl;
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
    {
      MediaSystemException nexcpt(url(), "readdir failed");
#if NONREMOTE_DIRECTORY_YAST
      nexcpt.remember(excpt_r);
#endif
      ZYPP_THROW(nexcpt);
    }

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
    {
	MediaSystemException nexcpt(url(), "readdir failed");
#if NONREMOTE_DIRECTORY_YAST
	nexcpt.remember(excpt_r);
#endif
	ZYPP_THROW(nexcpt);
    }
#if NONREMOTE_DIRECTORY_YAST
  }
#endif
}

///////////////////////////////////////////////////////////////////
//
//
//      METHOD NAME : MediaHandler::getDoesFileExist
//      METHOD TYPE : PMError
//
//      DESCRIPTION : Asserted that file is not a directory
//                    Default implementation of pure virtual.
//
bool MediaHandler::getDoesFileExist( const Pathname & filename ) const
{
  PathInfo info( localPath( filename ) );
  if( info.isDir() ) {
    ZYPP_THROW(MediaNotAFileException(url(), localPath(filename)));
  }
  return info.isExist();
}

bool MediaHandler::hasMoreDevices()
{
  return false;
}

void MediaHandler::getDetectedDevices(std::vector<std::string> & devices,
                                      unsigned int & index) const
{
  // clear the vector by default
  if (!devices.empty())
    devices.clear();
  index = 0;

  DBG << "No devices for this medium" << endl;
}

void MediaHandler::setDeltafile( const Pathname & filename ) const
{
  _deltafile = filename;
}

Pathname MediaHandler::deltafile() const {
  return _deltafile;
}

  } // namespace media
} // namespace zypp
// vim: set ts=8 sts=2 sw=2 ai noet:
