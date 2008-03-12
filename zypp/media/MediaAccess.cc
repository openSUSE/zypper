/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaAccess.cc
 *
*/

#include <ctype.h>

#include <iostream>

#include "zypp/base/Logger.h"

#include "zypp/media/MediaException.h"
#include "zypp/media/MediaAccess.h"
#include "zypp/media/MediaHandler.h"

#include "zypp/media/MediaNFS.h"
#include "zypp/media/MediaCD.h"
#include "zypp/media/MediaDIR.h"
#include "zypp/media/MediaDISK.h"
#include "zypp/media/MediaSMB.h"
#include "zypp/media/MediaCIFS.h"
#include "zypp/media/MediaCurl.h"
#include "zypp/media/MediaISO.h"

using namespace std;

namespace zypp {
  namespace media {

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : MediaAccess
//
///////////////////////////////////////////////////////////////////

const Pathname MediaAccess::_noPath; // empty path

///////////////////////////////////////////////////////////////////
// constructor
MediaAccess::MediaAccess ()
    : _handler (0)
{
}

// destructor
MediaAccess::~MediaAccess()
{
  try
    {
      close(); // !!! make sure handler gets properly deleted.
    }
  catch(...) {}
}

AttachedMedia
MediaAccess::attachedMedia() const
{
	return _handler ? _handler->attachedMedia()
	                : AttachedMedia();
}

bool
MediaAccess::isSharedMedia() const
{
	return _handler ? _handler->isSharedMedia()
	                : false;
}

void
MediaAccess::resetParentId()
{
	if( _handler) _handler->resetParentId();
}

bool
MediaAccess::dependsOnParent() const
{
	return _handler ? _handler->dependsOnParent() : false;
}

bool
MediaAccess::dependsOnParent(MediaAccessId parentId,
                             bool exactIdMatch) const
{
	return _handler ? _handler->dependsOnParent(parentId, exactIdMatch)
	                : false;
}

// open URL
void
MediaAccess::open (const Url& url, const Pathname & preferred_attach_point)
{
    if(!url.isValid()) {
	MIL << "Url is not valid" << endl;
        ZYPP_THROW(MediaBadUrlException(url));
    }

    close();

    std::string scheme = url.getScheme();

    MIL << "Trying scheme '" << scheme << "'" << endl;
    /*
    ** WARNING: Don't forget to update MediaAccess::downloads(url)
    **          if you are adding a new url scheme / handler!
    */
    if (scheme == "cd" || scheme == "dvd")
        _handler = new MediaCD (url,preferred_attach_point);
    else if (scheme == "nfs")
        _handler = new MediaNFS (url,preferred_attach_point);
    else if (scheme == "iso")
        _handler = new MediaISO (url,preferred_attach_point);
    else if (scheme == "file" || scheme == "dir")
        _handler = new MediaDIR (url,preferred_attach_point);
    else if (scheme == "hd")
        _handler = new MediaDISK (url,preferred_attach_point);
    else if (scheme == "smb")
	_handler = new MediaSMB (url,preferred_attach_point);
    else if (scheme == "cifs")
        _handler = new MediaCIFS (url,preferred_attach_point);
    else if (scheme == "ftp" || scheme == "http" || scheme == "https")
        _handler = new MediaCurl (url,preferred_attach_point);
    else
    {
	ZYPP_THROW(MediaUnsupportedUrlSchemeException(url));
    }

    // check created handler
    if ( !_handler ){
      ERR << "Failed to create media handler" << endl;
      ZYPP_THROW(MediaSystemException(url, "Failed to create media handler"));
    }

    MIL << "Opened: " << *this << endl;
}

// STATIC
bool
MediaAccess::downloads(const Url &url)
{
    std::string scheme( url.getScheme());
    return (scheme == "ftp" || scheme == "http" || scheme == "https");
}

// STATIC
bool
MediaAccess::canBeVolatile(const Url &url)
{
    std::string scheme( url.getScheme());
    return ! (scheme == "cd" || scheme == "dvd");
}

// Type of media if open, otherwise NONE.
std::string
MediaAccess::protocol() const
{
  if ( !_handler )
    return "unknown";

  return _handler->protocol();
}

bool
MediaAccess::downloads() const
{
	return _handler ? _handler->downloads() : false;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : MediaAccess::url
//	METHOD TYPE : Url
//
Url MediaAccess::url() const
{
  if ( !_handler )
    return Url();

  return _handler->url();
}

// close handler
void
MediaAccess::close ()
{
  ///////////////////////////////////////////////////////////////////
  // !!! make shure handler gets properly deleted.
  // I.e. release attached media before deleting the handler.
  ///////////////////////////////////////////////////////////////////
  if ( _handler ) {
    try {
      _handler->release();
    }
    catch (const MediaException & excpt_r)
    {
      ZYPP_CAUGHT(excpt_r);
      WAR << "Close: " << *this << " (" << excpt_r << ")" << endl;
      ZYPP_RETHROW(excpt_r);
    }
    MIL << "Close: " << *this << " (OK)" << endl;
    delete _handler;
    _handler = 0;
  }
}


// attach media
void MediaAccess::attach (bool next)
{
  if ( !_handler ) {
    ZYPP_THROW(MediaNotOpenException("attach"));
  }
  _handler->attach(next);
}

// True if media is open and attached.
bool
MediaAccess::isAttached() const
{
  return( _handler && _handler->isAttached() );
}


bool MediaAccess::hasMoreDevices() const
{
  return _handler && _handler->hasMoreDevices();
}


// local directory that corresponds to medias url
// If media is not open an empty pathname.
Pathname
MediaAccess::localRoot() const
{
  if ( !_handler )
    return _noPath;

  return _handler->localRoot();
}

// Short for 'localRoot() + pathname', but returns an empty
// * pathname if media is not open.
Pathname
MediaAccess::localPath( const Pathname & pathname ) const
{
  if ( !_handler )
    return _noPath;

  return _handler->localPath( pathname );
}

void
MediaAccess::disconnect()
{
  if ( !_handler )
    ZYPP_THROW(MediaNotOpenException("disconnect"));

  _handler->disconnect();
}


void
MediaAccess::release( const std::string & ejectDev )
{
  if ( !_handler )
    return;

  _handler->release( ejectDev );
}

// provide file denoted by path to attach dir
//
// filename is interpreted relative to the attached url
// and a path prefix is preserved to destination
void
MediaAccess::provideFile( const Pathname & filename, bool cached, bool checkonly) const
{
  if ( cached ) {
    PathInfo pi( localPath( filename ) );
    if ( pi.isExist() )
      return;
  }

  if(checkonly)
    ZYPP_THROW(MediaFileNotFoundException(url(), filename));

  if ( !_handler ) {
    ZYPP_THROW(MediaNotOpenException("provideFile(" + filename.asString() + ")"));
  }

  _handler->provideFile( filename );
}

void
MediaAccess::releaseFile( const Pathname & filename ) const
{
  if ( !_handler )
    return;

  _handler->releaseFile( filename );
}

// provide directory tree denoted by path to attach dir
//
// dirname is interpreted relative to the attached url
// and a path prefix is preserved to destination
void
MediaAccess::provideDir( const Pathname & dirname ) const
{
  if ( !_handler ) {
    ZYPP_THROW(MediaNotOpenException("provideDir(" + dirname.asString() + ")"));
  }

  _handler->provideDir( dirname );
}

void
MediaAccess::provideDirTree( const Pathname & dirname ) const
{
  if ( !_handler ) {
    ZYPP_THROW(MediaNotOpenException("provideDirTree(" + dirname.asString() + ")"));
  }

  _handler->provideDirTree( dirname );
}

void
MediaAccess::releaseDir( const Pathname & dirname ) const
{
  if ( !_handler )
    return;

  _handler->releaseDir( dirname );
}

void
MediaAccess::releasePath( const Pathname & pathname ) const
{
  if ( !_handler )
    return;

  _handler->releasePath( pathname );
}

// Return content of directory on media
void
MediaAccess::dirInfo( std::list<std::string> & retlist, const Pathname & dirname, bool dots ) const
{
  retlist.clear();

  if ( !_handler ) {
    ZYPP_THROW(MediaNotOpenException("dirInfo(" + dirname.asString() + ")"));
  }

  _handler->dirInfo( retlist, dirname, dots );
}

// Return content of directory on media
void
MediaAccess::dirInfo( filesystem::DirContent & retlist, const Pathname & dirname, bool dots ) const
{
  retlist.clear();

  if ( !_handler ) {
    ZYPP_THROW(MediaNotOpenException("dirInfo(" + dirname.asString() + ")"));
  }

  _handler->dirInfo( retlist, dirname, dots );
}

// return if a file exists
bool
MediaAccess::doesFileExist( const Pathname & filename ) const
{
  if ( !_handler ) {
    ZYPP_THROW(MediaNotOpenException("doesFileExist(" + filename.asString() + ")"));
  }

  return _handler->doesFileExist( filename );
}

std::ostream &
MediaAccess::dumpOn( std::ostream & str ) const
{
  if ( !_handler )
    return str << "MediaAccess( closed )";

  str << _handler->protocol() << "(" << *_handler << ")";
  return str;
}

void MediaAccess::getFile( const Url &from, const Pathname &to )
{
  DBG << "From: " << from << endl << "To: " << to << endl;

  Pathname path = from.getPathData();
  Pathname dir = path.dirname();
  string base = path.basename();

  Url u = from;
  u.setPathData( dir.asString() );

  MediaAccess media;

  try {
    media.open( u );
    media.attach();
    media._handler->provideFileCopy( base, to );
    media.release();
  }
  catch (const MediaException & excpt_r)
  {
    ZYPP_RETHROW(excpt_r);
  }
}

    std::ostream & operator<<( std::ostream & str, const MediaAccess & obj )
    { return obj.dumpOn( str ); }

///////////////////////////////////////////////////////////////////
  } // namespace media
} // namespace zypp
