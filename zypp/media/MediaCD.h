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
        int        _lastdev_tried;

        static bool openTray( const std::string & device_r );
        static bool closeTray( const std::string & device_r );

	DeviceList  detectDevices(bool supportingDVD);

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

        virtual void forceEject(const std::string & ejectDev);

	virtual bool isAutoMountedMedia(const AttachedMedia &media);
        
        virtual bool hasMoreDevices();

      public:

        MediaCD( const Url &      url_r,
		 const Pathname & attach_point_hint_r );

        virtual ~MediaCD() { try { release(); } catch(...) {} }

	virtual bool isAttached() const;
    };

///////////////////////////////////////////////////////////////////
  } // namespace media
} // namespace zypp
#endif // ZYPP_MEDIA_MEDIACD_H
