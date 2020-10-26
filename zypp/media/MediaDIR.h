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

#include <zypp/media/MediaHandler.h>

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

        MediaDIR( const Url &      url_r,
		  const Pathname & attach_point_hint_r );

        virtual ~MediaDIR() override { try { release(); } catch(...) {} }
    };

    ///////////////////////////////////////////////////////////////////

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_MEDIADIR_H
