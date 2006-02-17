/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaSMB.cc
 *
*/

#include <iostream>
#include <fstream>

#include "zypp/base/Logger.h"
#include "zypp/TmpPath.h"
#include "zypp/KVMap.h"
#include "zypp/media/Mount.h"
#include "zypp/media/MediaSMB.h"

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
    //	CLASS NAME : MediaSMB
    //
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaSMB::MediaSMB
    //	METHOD TYPE : Constructor
    //
    //	DESCRIPTION :
    //
    MediaSMB::MediaSMB( const Url &      url_r,
			const Pathname & attach_point_hint_r )
        : MediaHandler( url_r, attach_point_hint_r,
    		    stripShare( url_r.getPathName() ), // urlpath WITHOUT share name at attachpoint
    		    false )       // does_download
        , _vfstype( "cifs" )
    {
	MIL << "MediaSMB::MediaSMB(" << url_r << ", " << attach_point_hint_r << ")" << endl;
    }

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaSMB::attachTo
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that not already attached, and attachPoint
    //      is a directory.
    //
    //      NOTE: The implementation currently serves both, "smb" and
    //      and "cifs" URL's, but passes "cifs" to the mount command
    //      in any case.
    //
    void MediaSMB::attachTo(bool next)
    {
      if(_url.getHost().empty())
    	ZYPP_THROW(MediaBadUrlEmptyHostException(_url));
      if(next)
	ZYPP_THROW(MediaNotSupportedException(_url));
    
      string path = "//";
      path += _url.getHost() + "/" + getShare( _url.getPathName() );
   
      MediaSourceRef media( new MediaSource( _vfstype, path));
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
      if( mountpoint.empty() || mountpoint == "/")
      {
	mountpoint = createAttachPoint().asString();
	if( mountpoint.empty())
	  ZYPP_THROW( MediaBadAttachPointException(url()));
	setAttachPoint( mountpoint, true);
      }

      Mount mount;
 
      Mount::Options options( _url.getQueryParam("mountoptions") );
      string username = _url.getUsername();
      string password = _url.getPassword();
    
      options["guest"]; // prevent smbmount from asking for password
    
      if ( ! options.has( "rw" ) ) {
        options["ro"];
      }
    
      Mount::Options::iterator toEnv;
    
      // extract 'username', do not overwrite any _url.username
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
    
      // look for a workgroup
      string workgroup = _url.getQueryParam("workgroup");
      if ( workgroup.size() ) {
        options["workgroup"] = workgroup;
      }
    
      // pass 'username' and 'password' via environment
      Mount::Environment environment;
      if ( username.size() )
        environment["USER"] = username;
      if ( password.size() )
        environment["PASSWD"] = password;
    
      //////////////////////////////////////////////////////
      // In case we need a tmpfile, credentials will remove
      // it in it's destructor after the mout call below.
      filesystem::TmpPath credentials;
      if ( username.size() || password.size() )
        {
          filesystem::TmpFile tmp;
          ofstream outs( tmp.path().asString().c_str() );
          outs << "username=" <<  username << endl;
          outs << "password=" <<  password << endl;
          outs.close();
    
          credentials = tmp;
          options["credentials"] = credentials.path().asString();
        }
      //
      //////////////////////////////////////////////////////
    
      mount.mount( path, mountpoint.c_str(), _vfstype,
		   options.asString(), environment );

      setMediaSource(media);
    }

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaSMB::releaseFrom
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached.
    //
    void MediaSMB::releaseFrom( bool eject )
    {
      Mount mount;
      mount.umount(attachPoint().asString());
    }


    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : MediaSMB::getFile
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached.
    //
    void MediaSMB::getFile (const Pathname & filename) const
    {
      MediaHandler::getFile( filename );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : MediaSMB::getDir
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached.
    //
    void MediaSMB::getDir( const Pathname & dirname, bool recurse_r ) const
    {
      MediaHandler::getDir( dirname, recurse_r );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaSMB::getDirInfo
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached and retlist is empty.
    //
    void MediaSMB::getDirInfo( std::list<std::string> & retlist,
			       const Pathname & dirname, bool dots ) const
    {
      MediaHandler::getDirInfo( retlist, dirname, dots );
    }

    ///////////////////////////////////////////////////////////////////
    //
    //
    //	METHOD NAME : MediaSMB::getDirInfo
    //	METHOD TYPE : PMError
    //
    //	DESCRIPTION : Asserted that media is attached and retlist is empty.
    //
    void MediaSMB::getDirInfo( filesystem::DirContent & retlist,
			       const Pathname & dirname, bool dots ) const
    {
      MediaHandler::getDirInfo( retlist, dirname, dots );
    }

  } // namespace media
} // namespace zypp
