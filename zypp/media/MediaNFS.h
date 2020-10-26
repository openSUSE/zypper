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

#include <zypp/media/MediaHandler.h>

/**
 * Value of NFS mount minor timeout (passed to <code>timeo</code> option
 * of the NFS mount) in tenths of a second.
 *
 * The value of 300 should give a major timeout after 3.5 minutes
 * for UDP and 1.5 minutes for TCP. (#350309)
 */
#define NFS_MOUNT_TIMEOUT 300

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

	virtual void attachTo (bool next = false) override;

        virtual void releaseFrom( const std::string & ejectDev ) override;
	virtual void getFile( const OnMediaLocation & file ) const override;
	virtual void getDir( const Pathname & dirname, bool recurse_r ) const override;
        virtual void getDirInfo( std::list<std::string> & retlist,
                                 const Pathname & dirname, bool dots = true ) const override;
        virtual void getDirInfo( filesystem::DirContent & retlist,
                                 const Pathname & dirname, bool dots = true ) const override;
        virtual bool getDoesFileExist( const Pathname & filename ) const override;

      public:

        MediaNFS( const Url&       url_r,
		  const Pathname & attach_point_hint_r );

        virtual ~MediaNFS() override { try { release(); } catch(...) {} }

    	virtual bool isAttached() const override;
    };

    ///////////////////////////////////////////////////////////////////
  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_MEDIANFS_H
