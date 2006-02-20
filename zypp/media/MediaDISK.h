/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaDISK.h
 *
*/
#ifndef ZYPP_MEDIA_MEDIADISK_H
#define ZYPP_MEDIA_MEDIADISK_H

#include "zypp/media/MediaHandler.h"

namespace zypp {
  namespace media {

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : MediaDISK
    /**
     * @short Implementation class for DISK MediaHandler
     * @see MediaHandler
     **/
    class MediaDISK : public MediaHandler {

      private:

        unsigned long _mountflags;

        std::string _device;
        std::string _filesystem;

      protected:

        MEDIA_HANDLER_API;

      public:

        MediaDISK( const Url &      url_r,
		   const Pathname & attach_point_hint_r );

        virtual ~MediaDISK() { try { release(); } catch(...) {} }

        virtual bool isAttached() const;
    };

///////////////////////////////////////////////////////////////////

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_MEDIADISK_H
