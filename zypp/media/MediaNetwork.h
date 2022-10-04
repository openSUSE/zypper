/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaNetwork.h
 *
*/
#ifndef ZYPP_MEDIA_MEDIANETWORK_H
#define ZYPP_MEDIA_MEDIANETWORK_H

#include <zypp/base/Flags.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/media/MediaNetworkCommonHandler.h>

namespace internal {
  struct SharedData;
}

namespace zyppng {
  class DownloadSpec;
}

namespace zypp {
  namespace media {
    //
    //	CLASS NAME : MediaNetwork
    /**
     * @short Implementation class for FTP, HTTP and HTTPS MediaHandler
     * @see MediaHandler
     **/
    class MediaNetwork : public MediaNetworkCommonHandler
    {
      protected:

        void attachTo (bool next = false) override;
        void releaseFrom( const std::string & ejectDev ) override;
        void getFile( const OnMediaLocation & file ) const override;
        void getFileCopy( const OnMediaLocation & file, const Pathname & targetFilename ) const override;
        void getDir( const Pathname & dirname, bool recurse_r ) const override;
        void getDirInfo( std::list<std::string> & retlist,
                         const Pathname & dirname, bool dots = true ) const override;
        void getDirInfo( filesystem::DirContent & retlist,
                         const Pathname & dirname, bool dots = true ) const override;
        /**
         *
         * \throws MediaException
         *
         */
        void disconnectFrom() override;

        /**
         *
         * \throws MediaException
         *
         */
        bool getDoesFileExist( const Pathname & filename ) const override;

        bool checkAttachPoint(const Pathname &apoint) const override;


      public:

        MediaNetwork( const Url &      url_r,
                  const Pathname & attach_point_hint_r );

        ~MediaNetwork() override { try { release(); } catch(...) {} }

      private:

        void runRequest ( const zyppng::DownloadSpec &spec, callback::SendReport<DownloadProgressReport> *report = nullptr ) const;

      private:
        mutable std::shared_ptr<::internal::SharedData> _shared;
    };

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_MEDIANETWORK_H
