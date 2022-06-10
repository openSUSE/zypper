/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------*/
#include "downloadspec.h"
#include <string>

namespace zyppng {

  class DownloadSpecPrivate {
  public:
    DownloadSpecPrivate() = default;
    DownloadSpecPrivate( const DownloadSpecPrivate &other ) = default;
    DownloadSpecPrivate( DownloadSpecPrivate &&other ) = default;

    DownloadSpecPrivate *clone () const {
      return new DownloadSpecPrivate(*this);
    }

    zypp::Url _url;
    TransferSettings _settings;
    zypp::Pathname  _delta;
    zypp::ByteCount _expectedFileSize;
    zypp::Pathname  _targetPath;
    bool _checkExistanceOnly = false; //< this will NOT download the file, but only query the server if it exists
    bool _metalink_enabled   = true;  //< should the download try to use metalinks
    zypp::ByteCount _headerSize;     //< Optional file header size for things like zchunk
    std::optional<zypp::CheckSum> _headerChecksum; //< Optional file header checksum
    zypp::ByteCount _preferred_chunk_size = zypp::ByteCount( 4096, zypp::ByteCount::K );
  };

  ZYPP_IMPL_PRIVATE( DownloadSpec )

  DownloadSpec::DownloadSpec( Url file , zypp::filesystem::Pathname targetPath, zypp::ByteCount expectedFileSize ) : d_ptr( new DownloadSpecPrivate() )
  {
    // default settings
    d_ptr->_url = std::move(file);
    d_ptr->_targetPath = std::move(targetPath);
    d_ptr->_expectedFileSize = std::move( expectedFileSize );
  }

  DownloadSpec::DownloadSpec( const DownloadSpec &other ) = default;

  DownloadSpec &DownloadSpec::operator=(const DownloadSpec &other) = default;

  const Url &DownloadSpec::url() const
  {
    return d_ptr->_url;
  }

  DownloadSpec &DownloadSpec::setUrl(const Url &url)
  {
    d_ptr->_url = url;
    return *this;
  }

  const zypp::Pathname &DownloadSpec::targetPath() const
  {
    return d_ptr->_targetPath;
  }

  DownloadSpec &DownloadSpec::setTargetPath(const zypp::filesystem::Pathname &path)
  {
    d_ptr->_targetPath = path;
    return *this;
  }

  DownloadSpec &DownloadSpec::setMetalinkEnabled(bool enable)
  {
    d_ptr->_metalink_enabled = enable;
    return *this;
  }

  bool DownloadSpec::metalinkEnabled() const
  {
    return d_ptr->_metalink_enabled;
  }

  DownloadSpec &DownloadSpec::setCheckExistsOnly(bool set)
  {
    d_ptr->_checkExistanceOnly = ( set );
    return *this;
  }

  bool DownloadSpec::checkExistsOnly() const
  {
    return d_ptr->_checkExistanceOnly;
  }

  DownloadSpec &DownloadSpec::setDeltaFile(const zypp::Pathname &file)
  {
    d_ptr->_delta = file;
    return *this;
  }

  zypp::Pathname DownloadSpec::deltaFile() const
  {
    return d_ptr->_delta;
  }

  DownloadSpec &DownloadSpec::setPreferredChunkSize(const zypp::ByteCount &bc)
  {
    d_ptr->_preferred_chunk_size = bc;
    return *this;
  }

  zypp::ByteCount DownloadSpec::preferredChunkSize() const
  {
    return d_ptr->_preferred_chunk_size;
  }

  const TransferSettings &DownloadSpec::settings() const
  {
    return d_ptr->_settings;
  }

  DownloadSpec & DownloadSpec::setTransferSettings(TransferSettings &&set)
  {
    d_ptr->_settings = std::move( set );
    return *this;
  }

  DownloadSpec & DownloadSpec::setTransferSettings(const TransferSettings &set)
  {
    d_ptr->_settings = set;
    return *this;
  }

  DownloadSpec &DownloadSpec::setExpectedFileSize(const zypp::ByteCount &bc)
  {
    d_ptr->_expectedFileSize = bc;
    return *this;
  }

  zypp::ByteCount DownloadSpec::expectedFileSize() const
  {
    return d_ptr->_expectedFileSize;
  }

  DownloadSpec &DownloadSpec::setHeaderSize(const zypp::ByteCount &bc)
  {
    d_ptr->_headerSize = bc;
    return *this;
  }

  zypp::ByteCount DownloadSpec::headerSize() const
  {
    return d_ptr->_headerSize;
  }

  const std::optional<zypp::CheckSum> &DownloadSpec::headerChecksum() const
  {
    Z_D();
    return d->_headerChecksum;
  }

  DownloadSpec &DownloadSpec::setHeaderChecksum(const zypp::CheckSum &sum)
  {
    Z_D();
    if ( sum.empty() )
      d->_headerChecksum.reset();
    else {
      d->_headerChecksum = sum;
    }
    return *this;
  }
}
