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

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : MediaSMB
    /**
     * @short Implementation class for SMB MediaHandler
     *
     * NOTE: The implementation currently serves both, "smbfs"
     * and "cifs". The only difference is the vfstype passed to
     * the mount command.
     * @see MediaHandler
     **/
    class MediaSMB : public MediaHandler {

    private:

      /**
       * vfstype for mount. This is either "smbfs"
       * or "cifs" (rewritten by MediaCIFS).
       **/
      const char* _vfstype;

    protected:

      MEDIA_HANDLER_API;

      /**
       * MediaCIFS rewrites the vfstype to "cifs"
       * within it's constructor.
       **/
      void mountAsCIFS() { _vfstype = "cifs"; }

    public:

      MediaSMB( const Url&       url_r,
		const Pathname & attach_point_hint_r );

      virtual ~MediaSMB() { try { release(); } catch(...) {} }
    };

///////////////////////////////////////////////////////////////////A
  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_MEDIASMB_H
