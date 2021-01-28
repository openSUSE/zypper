#include "downloadspec.h"
#include <string>
#include <zypp-proto/download.pb.h>

namespace zyppng {

  class DownloadSpecPrivate {
  public:
    DownloadSpecPrivate() = default;
    DownloadSpecPrivate( const DownloadSpecPrivate &other ) = default;
    DownloadSpecPrivate( DownloadSpecPrivate &&other ) = default;
    DownloadSpecPrivate( zypp::proto::DownloadSpec spec ) :_protoData( std::move(spec) ) {}

    DownloadSpecPrivate *clone () const {
      return new DownloadSpecPrivate(*this);
    }

    zypp::proto::DownloadSpec _protoData;
  };

  ZYPP_IMPL_PRIVATE( DownloadSpec )

  DownloadSpec::DownloadSpec( Url file , zypp::filesystem::Pathname targetPath, zypp::ByteCount expectedFileSize ) : d_ptr( new DownloadSpecPrivate() )
  {
    // default settings
    *d_ptr->_protoData.mutable_settings() = std::move( TransferSettings().protoData() );

    setUrl( file );
    setTargetPath( targetPath );
    setExpectedFileSize( expectedFileSize );
    setPreferredChunkSize( zypp::ByteCount( 4096, zypp::ByteCount::K ) );
    d_ptr->_protoData.set_checkexistanceonly( false );
    d_ptr->_protoData.set_metalink_enabled( true );
  }

  DownloadSpec::DownloadSpec( const zypp::proto::DownloadSpec &spec ) : d_ptr( new DownloadSpecPrivate( spec ) )
  { }

  DownloadSpec::DownloadSpec( const DownloadSpec &other ) = default;

  DownloadSpec &DownloadSpec::operator=(const DownloadSpec &other) = default;

  Url DownloadSpec::url() const
  {
    return d_ptr->_protoData.url();
  }

  DownloadSpec &DownloadSpec::setUrl(const Url &url)
  {
    d_ptr->_protoData.set_url( url.asCompleteString() );
    return *this;
  }

  zypp::Pathname DownloadSpec::targetPath() const
  {
    return zypp::Pathname(d_ptr->_protoData.targetpath());
  }

  DownloadSpec &DownloadSpec::setTargetPath(const zypp::filesystem::Pathname &path)
  {
    d_ptr->_protoData.set_targetpath( path.asString() );
    return *this;
  }

  DownloadSpec &DownloadSpec::setMetalinkEnabled(bool enable)
  {
    d_ptr->_protoData.set_metalink_enabled( enable );
    return *this;
  }

  bool DownloadSpec::metalinkEnabled() const
  {
    return d_ptr->_protoData.metalink_enabled();
  }

  DownloadSpec &DownloadSpec::setCheckExistsOnly(bool set)
  {
    d_ptr->_protoData.set_checkexistanceonly( set );
    return *this;
  }

  bool DownloadSpec::checkExistsOnly() const
  {
    return d_ptr->_protoData.checkexistanceonly();
  }

  DownloadSpec &DownloadSpec::setDeltaFile(const zypp::filesystem::Pathname &file)
  {
    if ( !file.empty() )
      d_ptr->_protoData.set_delta( file.asString() );
    else
      d_ptr->_protoData.clear_delta();
    return *this;
  }

  zypp::filesystem::Pathname DownloadSpec::deltaFile() const
  {
    return d_ptr->_protoData.delta();
  }

  DownloadSpec &DownloadSpec::setPreferredChunkSize(const zypp::ByteCount &bc)
  {
    d_ptr->_protoData.set_preferred_chunk_size( bc.operator long long() );
    return *this;
  }

  zypp::ByteCount DownloadSpec::preferredChunkSize() const
  {
    return d_ptr->_protoData.preferred_chunk_size();
  }

  TransferSettings DownloadSpec::settings() const
  {
    return TransferSettings( d_ptr->_protoData.settings() );
  }

  DownloadSpec & DownloadSpec::setTransferSettings(TransferSettings &&set)
  {
    (*d_ptr->_protoData.mutable_settings()) = std::move( set.protoData() );
    return *this;
  }

  DownloadSpec & DownloadSpec::setTransferSettings(const TransferSettings &set)
  {
    (*d_ptr->_protoData.mutable_settings()) = set.protoData();
    return *this;
  }

  DownloadSpec &DownloadSpec::setExpectedFileSize(const zypp::ByteCount &bc)
  {
    d_ptr->_protoData.set_expectedfilesize( bc.operator long long() );
    return *this;
  }

  zypp::ByteCount DownloadSpec::expectedFileSize() const
  {
    return d_ptr->_protoData.expectedfilesize();
  }

  DownloadSpec &DownloadSpec::setHeaderSize(const zypp::ByteCount &bc)
  {
    d_ptr->_protoData.set_headersize( bc.operator long long() );
    return *this;
  }

  zypp::ByteCount DownloadSpec::headerSize() const
  {
    return d_ptr->_protoData.headersize();
  }

  std::optional<zypp::CheckSum> DownloadSpec::headerChecksum() const
  {
    Z_D();
    if ( !d->_protoData.has_headerchecksum() )
      return {};

    return zypp::CheckSum( d->_protoData.headerchecksum().type(), d->_protoData.headerchecksum().sum() );

  }

  DownloadSpec &DownloadSpec::setHeaderChecksum(const zypp::CheckSum &sum)
  {
    if ( sum.empty() )
      d_func()->_protoData.clear_headerchecksum();
    else {
      auto csum = d_func()->_protoData.mutable_headerchecksum();
      csum->set_type( sum.type() );
      csum->set_sum( sum.checksum() );
    }
    return *this;
  }

  const zypp::proto::DownloadSpec &DownloadSpec::protoData() const
  {
    return d_ptr->_protoData;
  }

  zypp::proto::DownloadSpec &DownloadSpec::protoData()
  {
    return d_ptr->_protoData;
  }
}

