/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaCD.h
 *
*/
#ifndef ZYPP_MEDIA_MEDIACD_H
#define ZYPP_MEDIA_MEDIACD_H

#include "zypp/media/MediaHandler.h"
#include "zypp/media/MediaManager.h"

namespace zypp {
  namespace media {

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : MediaCD
    /**
     * @short Implementation class for CD/DVD MediaHandler
     * @see MediaHandler
     **/
    class MediaCD : public MediaHandler {

      private:

        typedef std::list<MediaSource> DeviceList;
        /** list of devices to try to mount */
        DeviceList _devices;

        /** number of last successful mounted device in list */
        int        _lastdev;

        static bool openTray( const std::string & device_r );
        static bool closeTray( const std::string & device_r );

	DeviceList  detectDevices(bool supportingDVD);

      protected:

        MEDIA_HANDLER_API;

        virtual void forceEject();

      public:

        MediaCD( const Url &      url_r,
		 const Pathname & attach_point_hint_r );

        virtual ~MediaCD() { try { release(); } catch(...) {} }
    };

///////////////////////////////////////////////////////////////////
  } // namespace media
} // namespace zypp
#endif // ZYPP_MEDIA_MEDIACD_H
