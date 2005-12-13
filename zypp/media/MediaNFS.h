/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaNFS.h
 *
*/
#ifndef ZYPP_MEDIA_MEDIANFS_H
#define ZYPP_MEDIA_MEDIANFS_H

#include "zypp/media/MediaHandler.h"

namespace zypp {
  namespace media {

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : MediaNFS
    /**
     * @short Implementation class for NFS MediaHandler
     * @see MediaHandler
     **/
    class MediaNFS : public MediaHandler {
    
      protected:
    
        MEDIA_HANDLER_API;
    
      public:
    
        MediaNFS( const Url&       url_r,
		  const Pathname & attach_point_hint_r );
    
        virtual ~MediaNFS() { release(); }
    };
    
    ///////////////////////////////////////////////////////////////////
  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_MEDIANFS_H
