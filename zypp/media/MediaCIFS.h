/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaCIFS.h
 *
*/
#ifndef ZYPP_MEDIA_MEDIACIFS_H
#define ZYPP_MEDIA_MEDIACIFS_H

#include <zypp/media/MediaHandler.h>

namespace zypp {
  namespace media {

    class AuthData;

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : MediaCIFS
    /**
     * @short Implementation class for CIFS MediaHandler
     *
     * NOTE: The implementation serves both, "smb" and "cifs" URLs,
     * but passes "cifs" to the mount command in any case.
     * @see MediaHandler
     **/
    class MediaCIFS : public MediaHandler {

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
      MediaCIFS( const Url&       url_r,
		const Pathname & attach_point_hint_r );

      virtual ~MediaCIFS() override { try { release(); } catch(...) {} }

      virtual bool isAttached() const override;

    private:
      bool authenticate( AuthData & authdata, bool firstTry ) const;
    };

///////////////////////////////////////////////////////////////////A
  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_MEDIACIFS_H
