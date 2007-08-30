/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaException.cc
 *
*/

#include <string>
#include <iostream>

#include "zypp/media/MediaException.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  namespace media {
  /////////////////////////////////////////////////////////////////

    std::ostream & MediaMountException::dumpOn( std::ostream & str ) const
    {
      str << "Failed to mount " << _source << " on " << _target;
      if( !_cmdout.empty())
	str << ": " << _error << " (" << _cmdout << ")";
      else
	str << ": " << _error;
      return str;
    }

    std::ostream & MediaUnmountException::dumpOn( std::ostream & str ) const
    {
      return str << "Failed to unmount " << _path
	<< " : " << _error;
    }

    std::ostream & MediaBadFilenameException::dumpOn( std::ostream & str ) const
    {
      return str << "Bad file name " << _filename;
    }

    std::ostream & MediaNotOpenException::dumpOn( std::ostream & str ) const
    {
      return str << "Media not opened while performing action " << _action;
    }

    std::ostream & MediaFileNotFoundException::dumpOn( std::ostream & str) const
    {
      return str << "File " << _filename
	<< " not found on media: " << _url;
    }

    std::ostream & MediaWriteException::dumpOn( std::ostream & str) const
    {
      return str << "Cannot write file " << _filename;
    }

    std::ostream & MediaNotAttachedException::dumpOn( std::ostream & str) const
    {
      return str << "Media not attached: " << _url;
    }

    std::ostream & MediaBadAttachPointException::dumpOn( std::ostream & str) const
    {
      return str << "Bad media attach point: " << _url;
    }

    std::ostream & MediaCurlInitException::dumpOn( std::ostream & str) const
    {
      return str << "Curl init failed for: " << _url;
    }

    std::ostream & MediaSystemException::dumpOn( std::ostream & str) const
    {
      return str << "System exception: " << _message
	<< " on media: " << _url;
    }

    std::ostream & MediaNotAFileException::dumpOn( std::ostream & str) const
    {
      return str << "Path " << _path
	<< " on media: " << _url
        << " is not a file.";
    }

    std::ostream & MediaNotADirException::dumpOn( std::ostream & str) const
    {
      return str << "Path " << _path
	<< " on media: " << _url
        << " is not a directory.";
    }

    std::ostream & MediaBadUrlException::dumpOn( std::ostream & str) const
    {
      if( _msg.empty())
      {
      	return str << "Malformed URL: " << _url;
      }
      else
      {
      	return str << _msg << ": " << _url;
      }
    }

    std::ostream & MediaBadUrlEmptyHostException::dumpOn( std::ostream & str) const
    {
      return str << "Empty host name in URL: " << _url;
    }

    std::ostream & MediaBadUrlEmptyFilesystemException::dumpOn( std::ostream & str) const
    {
      return str << "Empty filesystem in URL: " << _url;
    }

    std::ostream & MediaBadUrlEmptyDestinationException::dumpOn( std::ostream & str) const
    {
      return str << "Empty destination in URL: " << _url;
    }

    std::ostream & MediaUnsupportedUrlSchemeException::dumpOn( std::ostream & str) const
    {
      return str << "Unsupported URL scheme in URL: " << _url;
    }

    std::ostream & MediaNotSupportedException::dumpOn( std::ostream & str) const
    {
      return str << "Operation not supported by media: " << _url;
    }

    std::ostream & MediaCurlException::dumpOn( std::ostream & str) const
    {
      return str << "Curl error for '" << _url << "':" << endl
        << "Error code: " << _err << endl
        << "Error message: " << _msg;
    }

    std::ostream & MediaCurlSetOptException::dumpOn( std::ostream & str) const
    {
      return str << "Error occurred while setting CURL options for " << _url
	<< ": " << _msg;
    }

    std::ostream & MediaNotDesiredException::dumpOn( std::ostream & str ) const
    {
      return str << "Media source " << _url << " does not contain the desired media";
    }

    std::ostream & MediaIsSharedException::dumpOn( std::ostream & str ) const
    {
      return str << "Media " << _name << " is in use by another instance";
    }

    std::ostream & MediaNotEjectedException::dumpOn( std::ostream & str ) const
    {
      if( _name.empty())
	return str << "Can't eject any media";
      else
	return str << "Can't eject media " << _name;
    }

    std::ostream & MediaUnauthorizedException::dumpOn( std::ostream & str ) const
    {
      str << msg();
      if( !_url.asString().empty())
      {
        str << " (" << _url << ")";
      }
      if( !_err.empty())
      {
        str << ": " << _err;
      }
      return str;
    }

  /////////////////////////////////////////////////////////////////
  } // namespace media
} // namespace zypp
///////////////////////////////////////////////////////////////////
