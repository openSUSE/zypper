/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaDIR.h
 *
*/
#ifndef ZYPP_MEDIA_MEDIADIR_H
#define ZYPP_MEDIA_MEDIADIR_H

#include "zypp/media/MediaHandler.h"

namespace zypp {
  namespace media {

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : MediaDIR

    /**
     * @short Implementation class for DIR MediaHandler
     * @see MediaHandler
     **/
    class MediaDIR : public MediaHandler {

      protected:

	MEDIA_HANDLER_API;

      public:

        MediaDIR( const Url &      url_r,
		  const Pathname & attach_point_hint_r );

        virtual ~MediaDIR() { try { release(); } catch(...) {} }
    };

    ///////////////////////////////////////////////////////////////////

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_MEDIADIR_H
