/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* You have been warned!
*
*/
#ifndef ZYPPNG_MEDIA_NETWORK_DOWNLOADSPEC_H
#define ZYPPNG_MEDIA_NETWORK_DOWNLOADSPEC_H

#include <zypp/zyppng/base/zyppglobal.h>
#include <zypp/zyppng/core/Url>
#include <zypp/Pathname.h>
#include <zypp/base/PtrTypes.h>
#include <zypp/ByteCount.h>
#include <zypp/CheckSum.h>
#include <zypp/media/TransferSettings.h>

namespace zypp::proto {
  class DownloadSpec;
}

namespace zyppng {


  class DownloadSpecPrivate;
  using TransferSettings = zypp::media::TransferSettings;

  /*!
   * Specifies all aspects of a download. Used together with the
   * \ref zyppng::Download and \ref zyppng::Downloader classes
   */
  class DownloadSpec
  {
    ZYPP_FWD_DECLARE_PRIVATE( DownloadSpec )

  public:

    DownloadSpec( Url file, zypp::filesystem::Pathname targetPath, zypp::ByteCount expectedFileSize = zypp::ByteCount() );
    DownloadSpec( const zypp::proto::DownloadSpec &spec );

    DownloadSpec( const DownloadSpec &other );
    DownloadSpec &operator= ( const DownloadSpec &other );

    /*!
     * Returns the source URL of the download
     */
    Url url () const;
    DownloadSpec &setUrl ( const Url &url );

    /*!
     * Returns the target file path, this is where the downloaded data is stored
     */
    zypp::filesystem::Pathname targetPath() const;
    DownloadSpec &setTargetPath ( const zypp::Pathname &path );

    /*!
     * Enabled or disabled metalink handling. Enabled by default.
     * \note if Metalink is enabled the Download tells the server that it accepts metalink files by adding a specific header
     *       to the request.
     */
    DownloadSpec &setMetalinkEnabled ( bool enable = true );
    bool metalinkEnabled (  ) const;

    /*!
     * Enables a special mode, in this case only the existance of the file is checked but no data is actually downloaded
     */
    DownloadSpec &setCheckExistsOnly ( bool set = true );
    bool checkExistsOnly    ( ) const;

    /*!
     * Set a already existing local file to be used for partial downloading, in case of a multichunk download all chunks from the
     * file that have the expected checksum will be reused instead of downloaded
     */
    DownloadSpec &setDeltaFile ( const zypp::Pathname &file );
    zypp::filesystem::Pathname deltaFile() const;

    /*!
     * Sets the prefered amount of bytes the downloader tries to request from a single server per metalink chunk request.
     * If the metalink description has smaller chunks those are coalesced to match the preferred size.
     */
    DownloadSpec &setPreferredChunkSize ( const zypp::ByteCount &bc );
    zypp::ByteCount preferredChunkSize() const;

    /*!
     * Returns the \sa zyppng::TransferSettings for the download. The settings are reused for
     * possible sub downloads, however authentication data is stripped if the subdownload uses a different host to
     * fetch the data from. If there is no auth data known \sa sigAuthRequired is emitted.
     */
    TransferSettings settings () const;
    DownloadSpec &setTransferSettings( TransferSettings &&set );
    DownloadSpec &setTransferSettings( const TransferSettings &set );

    DownloadSpec &setExpectedFileSize ( const zypp::ByteCount &bc );
    zypp::ByteCount expectedFileSize() const;

    DownloadSpec &setHeaderSize ( const zypp::ByteCount &bc );
    zypp::ByteCount headerSize() const;

    std::optional<zypp::CheckSum> headerChecksum () const;
    DownloadSpec &setHeaderChecksum ( const zypp::CheckSum &sum );

    const zypp::proto::DownloadSpec &protoData() const;
    zypp::proto::DownloadSpec &protoData();

  private:
    zypp::RWCOW_pointer<DownloadSpecPrivate> d_ptr;
  };

}




#endif // ZYPPNG_MEDIA_NETWORK_DOWNLOADSPEC_H
