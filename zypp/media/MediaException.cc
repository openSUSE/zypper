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
      return str << "Failed to mount " << _source << " on " << _target
	<< " : " << _error << endl;
    }

    std::ostream & MediaUnmountException::dumpOn( std::ostream & str ) const
    {
      return str << "Failed to unmount " << _path
	<< " : " << _error << endl;
    }

    std::ostream & MediaBadFilenameException::dumpOn( std::ostream & str ) const
    {
      return str << "Bad file name " << _filename << endl;
    }

    std::ostream & MediaNotOpenException::dumpOn( std::ostream & str ) const
    {
      return str << "Media not opened while performing action " << _action << endl;
    }

    std::ostream & MediaFileNotFoundException::dumpOn( std::ostream & str) const
    {
      return str << "File " << _filename
	<< " not found on media: " << _url << endl;
    }

    std::ostream & MediaWriteException::dumpOn( std::ostream & str) const
    {
      return str << "Cannot write file " << _filename << endl;
    }

    std::ostream & MediaNotAttachedException::dumpOn( std::ostream & str) const
    {
      return str << "Media not attached: " << _url << endl;
    }

    std::ostream & MediaBadAttachPointException::dumpOn( std::ostream & str) const
    {
      return str << "Bad media attach point: " << _url << endl;
    }

    std::ostream & MediaCurlInitException::dumpOn( std::ostream & str) const
    {
      return str << "Curl init failed for: " << _url << endl;
    }

    std::ostream & MediaSystemException::dumpOn( std::ostream & str) const
    {
      return str << "System exception: " << _message
	<< " on media: " << _url << endl;
    }

    std::ostream & MediaNotAFileException::dumpOn( std::ostream & str) const
    {
      return str << "Path " << _path
	<< " on media: " << _url
        << " is not a file." << endl;
    }

    std::ostream & MediaNotADirException::dumpOn( std::ostream & str) const
    {
      return str << "Path " << _path
	<< " on media: " << _url
        << " is not a directory." << endl;
    }

    std::ostream & MediaBadUrlException::dumpOn( std::ostream & str) const
    {
      if( _msg.empty())
      {
      	return str << "Malformed URL: " << _url << endl;
      }
      else
      {
      	return str << _msg << ": " << _url << endl;
      }
    }

    std::ostream & MediaBadUrlEmptyHostException::dumpOn( std::ostream & str) const
    {
      return str << "Empty host name in URL: " << _url << endl;
    }

    std::ostream & MediaBadUrlEmptyFilesystemException::dumpOn( std::ostream & str) const
    {
      return str << "Empty filesystem in URL: " << _url << endl;
    }

    std::ostream & MediaBadUrlEmptyDestinationException::dumpOn( std::ostream & str) const
    {
      return str << "Empty destination in URL: " << _url << endl;
    }

    std::ostream & MediaUnsupportedUrlSchemeException::dumpOn( std::ostream & str) const
    {
      return str << "Unsupported URL scheme in URL: " << _url << endl;
    }

    std::ostream & MediaNotSupportedException::dumpOn( std::ostream & str) const
    {
      return str << "Operation not supported by media: " << _url << endl;
    }

    std::ostream & MediaCurlException::dumpOn( std::ostream & str) const
    {
      return str << "Curl error for: " << _url
	<< ": Error code: " << _err
	<< " Error message: " << _msg << endl;
    }

    std::ostream & MediaCurlSetOptException::dumpOn( std::ostream & str) const
    {
      return str << "Error occurred while setting CURL options for " << _url
	<< ": " << _msg << endl;
    }

    std::ostream & MediaNotDesiredException::dumpOn( std::ostream & str ) const
    {
      return str << "Media source " << _url << " does not contain the desired media" << endl;
    }

    std::ostream & MediaIsSharedException::dumpOn( std::ostream & str ) const
    {
      return str << "Media " << _name << " is in use by another instance" << endl;
    }

  /////////////////////////////////////////////////////////////////
  } // namespace media
} // namespace zypp
///////////////////////////////////////////////////////////////////
