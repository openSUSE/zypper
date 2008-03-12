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

	virtual void attachTo (bool next = false);
        virtual void releaseFrom( const std::string & ejectDev );
	virtual void getFile( const Pathname & filename ) const;
	virtual void getDir( const Pathname & dirname, bool recurse_r ) const;
        virtual void getDirInfo( std::list<std::string> & retlist,
                                 const Pathname & dirname, bool dots = true ) const;
        virtual void getDirInfo( filesystem::DirContent & retlist,
                                 const Pathname & dirname, bool dots = true ) const;
        virtual bool getDoesFileExist( const Pathname & filename ) const;

      public:

        MediaDISK( const Url &      url_r,
		   const Pathname & attach_point_hint_r );

        virtual ~MediaDISK() { try { release(); } catch(...) {} }

        virtual bool isAttached() const;

        bool    verifyIfDiskVolume(const Pathname &name);
    };

///////////////////////////////////////////////////////////////////

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_MEDIADISK_H
