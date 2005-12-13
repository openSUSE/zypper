/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaCIFS.h
 *
*/
#ifndef ZYPP_MEDIA_MEDIACIFS_H
#define ZYPP_MEDIA_MEDIACIFS_H

#include "zypp/media/MediaSMB.h"

namespace zypp {
  namespace media {

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : MediaCIFS
    /**
     * @short Implementation class for CIFS MediaHandler
     *
     * NOTE: It's actually MediaSMB, but using "cifs"
     * as vfstype for mount.
     * @see MediaHandler
     **/
    class MediaCIFS : public MediaSMB {
    
    public:
    
      MediaCIFS( const Url&       url_r,
	         const Pathname & attach_point_hint_r )
	: MediaSMB( url_r, attach_point_hint_r )
      {
	mountAsCIFS();
      }
    };

  } // namespace media
} // namespace zypp

///////////////////////////////////////////////////////////////////

#endif // ZYPP_MEDIA_MEDIACIFS_H
