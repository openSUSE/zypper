/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaSMB.h
 *
*/
#ifndef ZYPP_MEDIA_MEDIASMB_H
#define ZYPP_MEDIA_MEDIASMB_H

#include "zypp/media/MediaHandler.h"

namespace zypp {
  namespace media {

    class AuthData;

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : MediaSMB
    /**
     * @short Implementation class for SMB MediaHandler
     *
     * NOTE: The implementation currently serves both, "smb"
     * and "cifs" URL's, but passes "cifs" to the mount command
     * in any case.
     * @see MediaHandler
     **/
    class MediaSMB : public MediaHandler {

    private:

      /**
       * vfstype for mount. This is either "smbfs"
       * or "cifs" (rewritten by MediaCIFS).
       * Obsolete: vfstype is allways "cifs".
       **/
      const char* _vfstype;

    protected:

      virtual void attachTo (bool next = false);
      /** \deprecated in favor of releaseFrom(string&) */
      virtual void releaseFrom( bool eject ) ZYPP_DEPRECATED;
      virtual void releaseFrom( const std::string & ejectDev );
      virtual void getFile( const Pathname & filename ) const;
      virtual void getDir( const Pathname & dirname, bool recurse_r ) const;
      virtual void getDirInfo( std::list<std::string> & retlist,
                               const Pathname & dirname, bool dots = true ) const;
      virtual void getDirInfo( filesystem::DirContent & retlist,
                               const Pathname & dirname, bool dots = true ) const;
      virtual bool getDoesFileExist( const Pathname & filename ) const;

      /**
       * MediaCIFS rewrites the vfstype to "cifs"
       * within it's constructor.
       **/
      void mountAsCIFS() { _vfstype = "cifs"; }

    public:
      MediaSMB( const Url&       url_r,
		const Pathname & attach_point_hint_r );

      virtual ~MediaSMB() { try { release(); } catch(...) {} }

      virtual bool isAttached() const;

    private:
      bool authenticate( AuthData & authdata, bool firstTry ) const;
    };

///////////////////////////////////////////////////////////////////A
  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_MEDIASMB_H
