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

/**
 * Value of nfs mount minor timeout in tenths of a second.
 */
#define NFS_MOUNT_TIMEOUT 10

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

	virtual void attachTo (bool next = false);
	virtual void releaseFrom( bool eject );
	virtual void getFile( const Pathname & filename ) const;
	virtual void getDir( const Pathname & dirname, bool recurse_r ) const;
        virtual void getDirInfo( std::list<std::string> & retlist,
                                 const Pathname & dirname, bool dots = true ) const;
        virtual void getDirInfo( filesystem::DirContent & retlist,
                                 const Pathname & dirname, bool dots = true ) const;
        virtual bool getDoesFileExist( const Pathname & filename ) const;

      public:

        MediaNFS( const Url&       url_r,
		  const Pathname & attach_point_hint_r );

        virtual ~MediaNFS() { try { release(); } catch(...) {} }

    	virtual bool isAttached() const;
    };

    ///////////////////////////////////////////////////////////////////
  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_MEDIANFS_H
