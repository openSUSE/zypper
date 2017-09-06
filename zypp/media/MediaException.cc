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

#include <iostream>

#include "zypp/base/String.h"
#include "zypp/base/Gettext.h"

#include "zypp/media/MediaException.h"

using std::endl;
using zypp::str::form;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  namespace media {
  /////////////////////////////////////////////////////////////////

    std::ostream & MediaMountException::dumpOn( std::ostream & str ) const
    {
      str << form(_("Failed to mount %s on %s"), _source.c_str(), _target.c_str() );
      if( !_cmdout.empty())
	str << ": " << _error << " (" << _cmdout << ")";
      else
	str << ": " << _error;
      return str;
    }

    std::ostream & MediaUnmountException::dumpOn( std::ostream & str ) const
    {
      return str << form(_("Failed to unmount %s"), _path.c_str() ) << " : " << _error;
    }

    std::ostream & MediaBadFilenameException::dumpOn( std::ostream & str ) const
    {
      return str << form(_("Bad file name: %s"), _filename.c_str() );
    }

    std::ostream & MediaNotOpenException::dumpOn( std::ostream & str ) const
    {
      return str << form(_("Medium not opened when trying to perform action '%s'."), _action.c_str() );
    }

    std::ostream & MediaFileNotFoundException::dumpOn( std::ostream & str) const
    {
      return str << form( _("File '%s' not found on medium '%s'"), _filename.c_str(), _url.c_str() );
    }

    std::ostream & MediaWriteException::dumpOn( std::ostream & str) const
    {
      return str << form(_("Cannot write file '%s'."), _filename.c_str() );
    }

    std::ostream & MediaNotAttachedException::dumpOn( std::ostream & str) const
    {
      return str << _("Medium not attached") << ": " << _url;
    }

    std::ostream & MediaBadAttachPointException::dumpOn( std::ostream & str) const
    {
      return str << _("Bad media attach point") << ": " << _url;
    }

    std::ostream & MediaCurlInitException::dumpOn( std::ostream & str) const
    {
      // TranslatorExplanation: curl is the name of a library, don't translate
      return str << form(_("Download (curl) initialization failed for '%s'"), _url.c_str() );
    }

    std::ostream & MediaSystemException::dumpOn( std::ostream & str) const
    {
      return str << form(_("System exception '%s' on medium '%s'."), _message.c_str(), _url.c_str() );
    }

    std::ostream & MediaNotAFileException::dumpOn( std::ostream & str) const
    {
      return str << form(_("Path '%s' on medium '%s' is not a file."), _path.c_str(), _url.c_str() );
    }

    std::ostream & MediaNotADirException::dumpOn( std::ostream & str) const
    {
      return str << form(_("Path '%s' on medium '%s' is not a directory."), _path.c_str(), _url.c_str() );
    }

    std::ostream & MediaBadUrlException::dumpOn( std::ostream & str) const
    {
      if( _msg.empty())
      {
	return str << _("Malformed URI") << ": " << _url;
      }
      else
      {
	return str << _msg << ": " << _url;
      }
    }

    std::ostream & MediaBadUrlEmptyHostException::dumpOn( std::ostream & str) const
    {
      return str << _("Empty host name in URI") << ": " << _url;
    }

    std::ostream & MediaBadUrlEmptyFilesystemException::dumpOn( std::ostream & str) const
    {
      return str << _("Empty filesystem in URI") << ": " << _url;
    }

    std::ostream & MediaBadUrlEmptyDestinationException::dumpOn( std::ostream & str) const
    {
      return str << _("Empty destination in URI") << ": " << _url;
    }

    std::ostream & MediaUnsupportedUrlSchemeException::dumpOn( std::ostream & str) const
    {
      return str << form(_("Unsupported URI scheme in '%s'."), _url.c_str() );
    }

    std::ostream & MediaNotSupportedException::dumpOn( std::ostream & str) const
    {
      return str << _("Operation not supported by medium") << ": " << _url;
    }

    std::ostream & MediaCurlException::dumpOn( std::ostream & str) const
    {
      // TranslatorExplanation: curl is the name of a library, don't translate
      return str << form(_(
        "Download (curl) error for '%s':\n"
        "Error code: %s\n"
        "Error message: %s\n"), _url.c_str(), _err.c_str(), _msg.c_str());
    }

    std::ostream & MediaCurlSetOptException::dumpOn( std::ostream & str) const
    {
      // TranslatorExplanation: curl is the name of a library, don't translate
      return str << form(_("Error occurred while setting download (curl) options for '%s':"), _url.c_str() );
      if ( !_msg.empty() )
        str << endl << _msg;
    }

    std::ostream & MediaNotDesiredException::dumpOn( std::ostream & str ) const
    {
      return str << form(_("Media source '%s' does not contain the desired medium"), _url.c_str() );
    }

    std::ostream & MediaIsSharedException::dumpOn( std::ostream & str ) const
    {
      return str << form(_("Medium '%s' is in use by another instance"), _name.c_str() );
    }

    std::ostream & MediaNotEjectedException::dumpOn( std::ostream & str ) const
    {
      if( _name.empty() )
	return str << _("Cannot eject any media");
      else
	return str << form(_("Cannot eject media '%s'"), _name.c_str());
    }

    std::ostream & MediaUnauthorizedException::dumpOn( std::ostream & str ) const
    {
      str << msg();
      if( !_url.asString().empty())
        str << " (" << _url << ")";
      if( !_err.empty())
        str << ": " << _err;
      return str;
    }

    std::ostream & MediaForbiddenException::dumpOn( std::ostream & str ) const
    {
      str << form(_("Permission to access '%s' denied."), _url.c_str());
      if ( !_msg.empty() )
        str << endl << _msg;
      return str;
    }

    std::ostream & MediaTimeoutException::dumpOn( std::ostream & str ) const
    {
      str << form(_("Timeout exceeded when accessing '%s'."), _url.c_str() );
      if ( !_msg.empty() )
        str << endl << _msg;
      return str;
    }

    std::ostream & MediaTemporaryProblemException::dumpOn( std::ostream & str ) const
    {
      str << form(_("Location '%s' is temporarily unaccessible."), _url.c_str() );
      if ( !_msg.empty() )
        str << endl << _msg;
      return str;
    }

    std::ostream & MediaBadCAException::dumpOn( std::ostream & str ) const
    {
      str << form(_(" SSL certificate problem, verify that the CA cert is OK for '%s'."), _url.c_str() );
      if ( !_msg.empty() )
        str << endl << _msg;
      return str;
    }

  /////////////////////////////////////////////////////////////////
  } // namespace media
} // namespace zypp
///////////////////////////////////////////////////////////////////
