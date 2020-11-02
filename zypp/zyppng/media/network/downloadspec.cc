#include "downloadspec.h"
#include <string>
#include <zypp/zyppng/download.pb.h>

namespace zyppng {

  class DownloadSpecPrivate : public zypp::proto::DownloadSpec {
  public:
    DownloadSpecPrivate() = default;
    DownloadSpecPrivate( const DownloadSpecPrivate &other ) = default;
    DownloadSpecPrivate( DownloadSpecPrivate &&other ) = default;
    DownloadSpecPrivate( zypp::proto::DownloadSpec spec ) : zypp::proto::DownloadSpec( std::move(spec) ) {}

    DownloadSpecPrivate *clone () const {
      return new DownloadSpecPrivate(*this);
    }

  };

  ZYPP_IMPL_PRIVATE( DownloadSpec )

  DownloadSpec::DownloadSpec( Url file , zypp::filesystem::Pathname targetPath, zypp::ByteCount expectedFileSize ) : d_ptr( new DownloadSpecPrivate() )
  {
    // default settings
    *d_ptr->mutable_settings() = std::move( TransferSettings().protoData() );

    setUrl( file );
    setTargetPath( targetPath );
    setExpectedFileSize( expectedFileSize );
    setPreferredChunkSize( zypp::ByteCount( 4096, zypp::ByteCount::K ) );
    d_ptr->set_checkexistanceonly( false );
    d_ptr->set_metalink_enabled( true );
  }

  DownloadSpec::DownloadSpec( const zypp::proto::DownloadSpec &spec ) : d_ptr( new DownloadSpecPrivate( spec ) )
  { }

  DownloadSpec::DownloadSpec( const DownloadSpec &other ) = default;

  DownloadSpec &DownloadSpec::operator=(const DownloadSpec &other) = default;

  Url DownloadSpec::url() const
  {
    return d_ptr->url();
  }

  DownloadSpec &DownloadSpec::setUrl(const Url &url)
  {
    d_ptr->set_url( url.asCompleteString() );
    return *this;
  }

  zypp::Pathname DownloadSpec::targetPath() const
  {
    return zypp::Pathname(d_ptr->targetpath());
  }

  DownloadSpec &DownloadSpec::setTargetPath(const zypp::filesystem::Pathname &path)
  {
    d_ptr->set_targetpath( path.asString() );
    return *this;
  }

  DownloadSpec &DownloadSpec::setMetalinkEnabled(bool enable)
  {
    d_ptr->set_metalink_enabled( enable );
    return *this;
  }

  bool DownloadSpec::metalinkEnabled() const
  {
    return d_ptr->metalink_enabled();
  }

  DownloadSpec &DownloadSpec::setCheckExistsOnly(bool set)
  {
    d_ptr->set_checkexistanceonly( set );
    return *this;
  }

  bool DownloadSpec::checkExistsOnly() const
  {
    return d_ptr->checkexistanceonly();
  }

  DownloadSpec &DownloadSpec::setDeltaFile(const zypp::filesystem::Pathname &file)
  {
    if ( !file.empty() )
      d_ptr->set_delta( file.asString() );
    else
      d_ptr->clear_delta();
    return *this;
  }

  zypp::filesystem::Pathname DownloadSpec::deltaFile() const
  {
    return d_ptr->delta();
  }

  DownloadSpec &DownloadSpec::setPreferredChunkSize(const zypp::ByteCount &bc)
  {
    d_ptr->set_preferred_chunk_size( bc.operator long long() );
    return *this;
  }

  zypp::ByteCount DownloadSpec::preferredChunkSize() const
  {
    return d_ptr->preferred_chunk_size();
  }

  TransferSettings DownloadSpec::settings() const
  {
    return TransferSettings( d_ptr->settings() );
  }

  DownloadSpec & DownloadSpec::setTransferSettings(TransferSettings &&set)
  {
    (*d_ptr->mutable_settings()) = std::move( set.protoData() );
    return *this;
  }

  DownloadSpec & DownloadSpec::setTransferSettings(const TransferSettings &set)
  {
    (*d_ptr->mutable_settings()) = set.protoData();
    return *this;
  }

  DownloadSpec &DownloadSpec::setExpectedFileSize(const zypp::ByteCount &bc)
  {
    d_ptr->set_expectedfilesize( bc.operator long long() );
    return *this;
  }

  zypp::ByteCount DownloadSpec::expectedFileSize() const
  {
    return d_ptr->expectedfilesize();
  }

  DownloadSpec &DownloadSpec::setHeaderSize(const zypp::ByteCount &bc)
  {
    d_ptr->set_headersize( bc.operator long long() );
    return *this;
  }

  zypp::ByteCount DownloadSpec::headerSize() const
  {
    return d_ptr->headersize();
  }

  std::optional<zypp::CheckSum> DownloadSpec::headerChecksum() const
  {
    Z_D();
    if ( !d->has_headerchecksum() )
      return {};

    return zypp::CheckSum( d->headerchecksum().type(), d->headerchecksum().sum() );

  }

  DownloadSpec &DownloadSpec::setHeaderChecksum(const zypp::CheckSum &sum)
  {
    if ( sum.empty() )
      d_func()->clear_headerchecksum();
    else {
      auto csum = d_func()->mutable_headerchecksum();
      csum->set_type( sum.type() );
      csum->set_sum( sum.checksum() );
    }
    return *this;
  }

  const zypp::proto::DownloadSpec &DownloadSpec::protoData() const
  {
    return *d_ptr;
  }

  zypp::proto::DownloadSpec &DownloadSpec::protoData()
  {
    return *d_ptr;
  }
}

