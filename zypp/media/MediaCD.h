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
    
        unsigned long _mountflags;
    
        typedef std::list<std::string> DeviceList;
        /** list of devices to try to mount */
        DeviceList _devices;
    
        /** which device has been mounted */
        std::string _mounteddevice;
    
        /** number of last successful mounted device in list */
        int _lastdev;
    
        static bool openTray( const std::string & device_r );
        static bool closeTray( const std::string & device_r );
    
      protected:
    
        MEDIA_HANDLER_API;
    
        virtual void forceEject();
    
      public:
    
        MediaCD( const Url &      url_r,
		 const Pathname & attach_point_hint_r );
    
        virtual ~MediaCD() { release(); }
    };

///////////////////////////////////////////////////////////////////
  } // namespace media
} // namespace zypp
#endif // ZYPP_MEDIA_MEDIACD_H
