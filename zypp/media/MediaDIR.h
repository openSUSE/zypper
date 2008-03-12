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

        MediaDIR( const Url &      url_r,
		  const Pathname & attach_point_hint_r );

        virtual ~MediaDIR() { try { release(); } catch(...) {} }
    };

    ///////////////////////////////////////////////////////////////////

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_MEDIADIR_H
