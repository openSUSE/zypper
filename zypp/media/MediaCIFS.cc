/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaCIFS.cc
 *
*/

#include <iostream>
#include <fstream>

#include "zypp/base/Logger.h"
#include "zypp/base/Gettext.h"
#include "zypp/TmpPath.h"
#include "zypp/KVMap.h"
#include "zypp/media/Mount.h"
#include "zypp/media/MediaUserAuth.h"
#include "zypp/media/CredentialManager.h"
#include "zypp/ZYppCallbacks.h"
#include "zypp/ZConfig.h"

#include "zypp/media/MediaCIFS.h"

#include <sys/types.h>
#include <sys/mount.h>
#include <errno.h>
#include <dirent.h>

using namespace std;

namespace zypp {
  namespace media {

    /******************************************************************
    **
    **
    **	FUNCTION NAME : getShare
    **	FUNCTION TYPE : inline Pathname
    **
    ** Get the 1st path component (CIFS share name).
    */
    inline string getShare( Pathname spath_r )
    {
      if ( spath_r.empty() )
        return string();

      string share( spath_r.absolutename().asString() );
      string::size_type sep = share.find( "/", 1 );
      if ( sep == string::npos )
        share = share.erase( 0, 1 ); // nothing but the share name in spath_r
      else
        share = share.substr( 1, sep-1 );

      // deescape %2f in sharename
      while ( (sep = share.find( "%2f" )) != string::npos ) {
        share.replace( sep, 3, "/" );
      }

      return share;
    }

    /******************************************************************
    **
    **
    **	FUNCTION NAME : stripShare
    **	FUNCTION TYPE : inline Pathname
    **
    ** Strip off the 1st path component (CIFS share name).
    */
    inline Pathname stripShare( Pathname spath_r )
    {
      if ( spath_r.empty() )
        return Pathname();

      string striped( spath_r.absolutename().asString() );
      string::size_type sep = striped.find( "/", 1 );
      if ( sep == string::npos )
        return "/"; // nothing but the share name in spath_r

      return striped.substr( sep );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : MediaCIFS
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaCIFS::MediaCIFS
    //	METHOD TYPE : Constructor
    //
    //	DESCRIPTION :
    //
    MediaCIFS::MediaCIFS( const Url &      url_r,
			const Pathname & attach_point_hint_r )
        : MediaHandler( url_r, attach_point_hint_r,
    		    stripShare( url_r.getPathName() ), // urlpath WITHOUT share name at attachpoint
    		    false )       // does_download
    {
	MIL << "MediaCIFS::MediaCIFS(" << url_r << ", " << attach_point_hint_r << ")" << endl;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaCIFS::attachTo
    //	METHOD TYPE : PMError
    /**
     * Asserted that not already attached, and attachPoint is a directory.
     *
     * Authentication: credentials can be specified in the following few ways
     * (the first has the highest preference).
     * - URL username:password
     * - mountoptions URL query parameter (see man mount.cifs)
     * - CredentialManager - either previously saved credentials will be used
     *   or the user will be promted for them via AuthenticationReport callback.
     *
     * \note The implementation currently serves both, "smb" and
     *      and "cifs" URLs, but passes "cifs" to the mount command
     *      in any case.
     */
    void MediaCIFS::attachTo(bool next)
    {
      if(_url.getHost().empty())
    	ZYPP_THROW(MediaBadUrlEmptyHostException(_url));
      if(next)
	ZYPP_THROW(MediaNotSupportedException(_url));

      string path = "//";
      path += _url.getHost() + "/" + getShare( _url.getPathName() );

      MediaSourceRef media( new MediaSource( "cifs", path));
      AttachedMedia  ret( findAttachedMedia( media));

      if( ret.mediaSource &&
	  ret.attachPoint &&
	  !ret.attachPoint->empty())
      {
	DBG << "Using a shared media "
	    << ret.mediaSource->name
	    << " attached on "
	    << ret.attachPoint->path
	    << endl;

	removeAttachPoint();
	setAttachPoint(ret.attachPoint);
	setMediaSource(ret.mediaSource);
	return;
      }

      std::string mountpoint = attachPoint().asString();
      if( !isUseableAttachPoint(attachPoint()))
      {
	mountpoint = createAttachPoint().asString();
	if( mountpoint.empty())
	  ZYPP_THROW( MediaBadAttachPointException(url()));
	setAttachPoint( mountpoint, true);
      }

      Mount mount;
      CredentialManager cm;

      Mount::Options options( _url.getQueryParam("mountoptions") );
      string username = _url.getUsername();
      string password = _url.getPassword();

      if ( ! options.has( "rw" ) ) {
        options["ro"];
      }

      // look for a workgroup
      string workgroup = _url.getQueryParam("workgroup");
      if ( workgroup.empty() )
        workgroup = _url.getQueryParam("domain");
      if ( !workgroup.empty() )
        options["domain"] = workgroup;

      // extract 'username', do not overwrite any _url.username

      Mount::Options::iterator toEnv;
      toEnv = options.find("username");
      if ( toEnv != options.end() ) {
        if ( username.empty() )
          username = toEnv->second;
        options.erase( toEnv );
      }

      toEnv = options.find("user"); // actually cifs specific
      if ( toEnv != options.end() ) {
        if ( username.empty() )
          username = toEnv->second;
        options.erase( toEnv );
      }

      // extract 'password', do not overwrite any _url.password

      toEnv = options.find("password");
      if ( toEnv != options.end() ) {
        if ( password.empty() )
          password = toEnv->second;
        options.erase( toEnv );
      }

      toEnv = options.find("pass"); // actually cifs specific
      if ( toEnv != options.end() ) {
        if ( password.empty() )
          password = toEnv->second;
        options.erase( toEnv );
      }

      if ( username.empty() || password.empty() )
      {
        AuthData_Ptr c = cm.getCred(_url);
        if (c)
        {
          username = c->username();
          password = c->password();
        }
      }

      bool firstTry = true;
      bool authRequired = false;
      AuthData authdata;
      do // repeat this while the mount returns "Permission denied" error
      {
        // get credentials from authenicate()
        if ( !firstTry )
        {
          username = authdata.username();
          password = authdata.password();
        }

        // pass 'username' and 'password' via environment
        Mount::Environment environment;
        if ( !username.empty() )
          environment["USER"] = username;
        if ( !password.empty() )
          environment["PASSWD"] = password;

        //////////////////////////////////////////////////////
        // In case we need a tmpfile, credentials will remove
        // it in it's destructor after the mout call below.
        filesystem::TmpPath credentials;
        if ( !username.empty() || !password.empty() )
        {
          filesystem::TmpFile tmp;
          ofstream outs( tmp.path().asString().c_str() );
          outs << "username=" <<  username << endl;
          outs << "password=" <<  password << endl;
          outs.close();

          credentials = tmp;
          options["credentials"] = credentials.path().asString();
        }
        else
        {
          // Use 'guest' option unless explicitly disabled (bnc #547354)
          if ( options.has( "noguest" ) )
            options.erase( "noguest" );
          else
            // prevent smbmount from asking for password
            // only add this option if 'credentials' is not used (bnc #560496)
            options["guest"];
        }

        //
        //////////////////////////////////////////////////////

        try
        {
          mount.mount( path, mountpoint, "cifs",
                       options.asString(), environment );
          setMediaSource(media);
          break;
        }
        catch (const MediaMountException & e)
        {
          ZYPP_CAUGHT( e );

          if ( e.mountError() == "Permission denied" )
            authRequired = authenticate( authdata, firstTry );
          else
            ZYPP_RETHROW( e );
        }

        firstTry = false;
      }
      while ( authRequired );

      // wait for /etc/mtab update ...
      // (shouldn't be needed)
      int limit = 3;
      bool mountsucceeded;
      while( !(mountsucceeded=isAttached()) && --limit)
        sleep(1);

      if ( !mountsucceeded )
      {
        setMediaSource(MediaSourceRef());
        try
        {
          mount.umount(attachPoint().asString());
        }
        catch (const MediaException & excpt_r)
        {
          ZYPP_CAUGHT(excpt_r);
        }
        ZYPP_THROW(MediaMountException(
          "Unable to verify that the media was mounted",
          path, mountpoint
        ));
      }
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : MediaCIFS::isAttached
    //	METHOD TYPE : bool
    //
    //	DESCRIPTION : Override check if media is attached.
    //
    bool
    MediaCIFS::isAttached() const
    {
      return checkAttached(true);
    }

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaCIFS::releaseFrom
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached.
    //
    void MediaCIFS::releaseFrom( const std::string & ejectDev )
    {
      Mount mount;
      mount.umount(attachPoint().asString());
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : MediaCIFS::getFile
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached.
    //
    void MediaCIFS::getFile (const Pathname & filename) const
    {
      MediaHandler::getFile( filename );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : MediaCIFS::getDir
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached.
    //
    void MediaCIFS::getDir( const Pathname & dirname, bool recurse_r ) const
    {
      MediaHandler::getDir( dirname, recurse_r );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaCIFS::getDirInfo
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached and retlist is empty.
    //
    void MediaCIFS::getDirInfo( std::list<std::string> & retlist,
			       const Pathname & dirname, bool dots ) const
    {
      MediaHandler::getDirInfo( retlist, dirname, dots );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaCIFS::getDirInfo
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached and retlist is empty.
    //
    void MediaCIFS::getDirInfo( filesystem::DirContent & retlist,
			       const Pathname & dirname, bool dots ) const
    {
      MediaHandler::getDirInfo( retlist, dirname, dots );
    }

    bool MediaCIFS::getDoesFileExist( const Pathname & filename ) const
    {
      return MediaHandler::getDoesFileExist( filename );
    }

    bool MediaCIFS::authenticate(AuthData & authdata, bool firstTry) const
    {
      //! \todo need a way to pass different CredManagerOptions here
      CredentialManager cm(CredManagerOptions(ZConfig::instance().systemRoot()));

      // get stored credentials
      AuthData_Ptr cmcred = cm.getCred(_url);

      AuthData_Ptr smbcred;
      smbcred.reset(new AuthData());
      callback::SendReport<AuthenticationReport> auth_report;

      // preset the username if present in current url
      if (!_url.getUsername().empty() && firstTry)
        smbcred->setUsername(_url.getUsername());
      // if CM has found some credentials, preset the username from there
      else if (cmcred)
        smbcred->setUsername(cmcred->username());

      // indicate we have no good credentials from CM
      cmcred.reset();

      string prompt_msg = str::form(
        //!\todo add comma to the message for the next release
        _("Authentication required for '%s'"), _url.asString().c_str());

      // ask user
      if (auth_report->prompt(_url, prompt_msg, *smbcred))
      {
        DBG << "callback answer: retry" << endl
            << "AuthData: " << *smbcred << endl;

        if (smbcred->valid())
        {
          cmcred = smbcred;
            // if (credentials->username() != _url.getUsername())
            //   _url.setUsername(credentials->username());
            /**
             *  \todo find a way to save the url with changed username
             *  back to repoinfo or dont store urls with username
             *  (and either forbid more repos with the same url and different
             *  user, or return a set of credentials from CM and try them one
             *  by one)
             */
        }
      }
      else
        DBG << "callback answer: cancel" << endl;

      // set username and password
      if (cmcred)
      {
        authdata.setUsername(cmcred->username());
        authdata.setPassword(cmcred->password());

        // save the credentials
        cmcred->setUrl(_url);
        cm.addCred(*cmcred);
        cm.save();

        return true;
      }

      return false;
    }


  } // namespace media
} // namespace zypp
