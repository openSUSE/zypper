/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------*/

#if ENABLE_ZCHUNK_COMPRESSION

#include <zypp-curl/ng/network/private/downloader_p.h>
#include <zypp-curl/ng/network/private/mediadebug_p.h>
#include <zypp-curl/ng/network/private/networkrequesterror_p.h>
#include <zypp-core/AutoDispose.h>

#include "zck_p.h"

#include <iostream>
#include <fstream>
#include <fcntl.h>

extern "C" {
#include <zck.h>
}

namespace zyppng {

  bool isZchunkFile ( const zypp::Pathname &file ) {
    std::ifstream dFile( file.c_str() );
    if ( !dFile.is_open() )
      return false;

    constexpr std::string_view magic("\0ZCK1", 5);

    std::array< char, magic.size() > lead;
    lead.fill('\0');
    dFile.read( lead.data(), lead.size() );
    return ( magic == std::string_view( lead.data(), lead.size()) );
  }

  DLZckHeadState::DLZckHeadState( std::vector<Url> &&mirrors, std::shared_ptr<Request> &&oldReq, DownloadPrivate &parent )
    : BasicDownloaderStateBase( std::move(oldReq), parent )
  {
    _fileMirrors = std::move(mirrors);
    MIL_MEDIA << "About to enter DlZckHeadState for url " << parent._spec.url() << std::endl;
  }

  DLZckHeadState::DLZckHeadState( std::vector<Url> &&mirrors, DownloadPrivate &parent )
    : BasicDownloaderStateBase( parent )
  {
    _fileMirrors = std::move(mirrors);
    MIL_MEDIA << "About to enter DlZckHeadState for url " << parent._spec.url() << std::endl;
  }


  bool DLZckHeadState::initializeRequest(std::shared_ptr<Request> &r )
  {
    BasicDownloaderStateBase::initializeRequest( r );

    const auto &s = stateMachine()._spec;
    if ( s.headerSize() == 0 ) {
      ERR_MEDIA << "Downloading the zck header was requested, but headersize is zero." << std::endl;
      return false;
    }

    std::shared_ptr<zypp::Digest> digest;
    NetworkRequest::CheckSumBytes sum;

    const auto &headerSum = s.headerChecksum();
    if ( headerSum ) {
      digest = std::make_shared<zypp::Digest>();
      if ( !digest->create( headerSum->type() ) ) {
        ERR << "Unknown header checksum type " << headerSum->type() << std::endl;
        return false;
      }
      sum = zypp::Digest::hexStringToUByteArray( headerSum->checksum() );
    }

    r->addRequestRange( 0, s.headerSize(), digest, sum );
    return true;
  }

  void DLZckHeadState::gotFinished()
  {
    if ( isZchunkFile( stateMachine()._spec.targetPath() ) )
      return BasicDownloaderStateBase::gotFinished();
    failed ( "Downloaded header is not a zchunk header");
  }

  std::shared_ptr<DLZckState> DLZckHeadState::transitionToDlZckState()
  {
    MIL_MEDIA << "Downloaded the header of size: " << _request->downloadedByteCount() << std::endl;
    return std::make_shared<DLZckState>( std::move(_fileMirrors), stateMachine() );
  }

  DLZckState::DLZckState(std::vector<Url> &&mirrors, DownloadPrivate &parent)
    : RangeDownloaderBaseState( std::move(mirrors), parent )
  {
    MIL_MEDIA << "About to enter DLZckState for url " << parent._spec.url() << std::endl;
  }

  void DLZckState::enter()
  {
    auto &sm = stateMachine();
    const auto &spec = sm._spec;

    // setup the base downloader
    _error = {};
    _ranges.clear();
    _failedRanges.clear();

    // @TODO get this from zchunk file?
    _fileSize = spec.expectedFileSize();

    zypp::AutoFD src_fd = open( spec.deltaFile().asString().c_str(), O_RDONLY);
    if(src_fd < 0)
      return setFailed ( zypp::str::Format("Unable to open %1%") % spec.deltaFile() );

    zypp::AutoDispose<zckCtx *> zck_src ( zck_create(), []( auto ptr ) { if ( ptr ) zck_free( &ptr ); } );
    if( !zck_src )
      return setFailed ( zypp::str::Format("%1%") % zck_get_error(NULL) );

    if(!zck_init_read(zck_src, src_fd))
      return setFailed ( zypp::str::Format( "Unable to open %1%: %2%") %  spec.deltaFile() % zck_get_error(zck_src) );

    zypp::AutoFD target_fd = open( spec.targetPath().asString().c_str(), O_RDWR);
    if(target_fd < 0)
      return setFailed ( zypp::str::Format("Unable to open %1%") % spec.targetPath() );

    zypp::AutoDispose<zckCtx *> zckTarget ( zck_create(), []( auto ptr ) { if ( ptr ) zck_free( &ptr ); } );
    if( !zckTarget )
      return setFailed ( zypp::str::Format("%1%") % zck_get_error(NULL) );

    if(!zck_init_read(zckTarget, target_fd))
      return setFailed ( zypp::str::Format( "Unable to open %1%: %2%") %  spec.targetPath() % zck_get_error(zckTarget) );

    // Returns 0 for error, -1 for invalid checksum and 1 for valid checksum
    switch ( zck_find_valid_chunks(zckTarget) ) {
      case 0: // Returns 0 if there was a error
        return setFailed ( zypp::str::Format( "Unable to open %1%: %2%") %  spec.targetPath() % zck_get_error(zckTarget) );
      case 1: // getting a 1 would mean the file is already complete, basically impossible but lets handle it anyway
        return setFinished();
    }

    const auto srcHashType = zck_get_chunk_hash_type( zckTarget );
    const auto targetHashType = zck_get_chunk_hash_type( zckTarget );

    const size_t fLen = zck_get_length( zckTarget );
    if ( _fileSize > 0 ) {
      // check if the file size as reported by zchunk is equal to the one we expect
      if ( _fileSize != fLen ) {
        return setFailed( NetworkRequestErrorPrivate::customError(
          NetworkRequestError::ExceededMaxLen,
          zypp::str::Format("Zchunk header reports a different filesize than what was expected ( Zck: %1% != Exp: %2%).") % fLen % _fileSize )
          );
      }
    } else {
      _fileSize = fLen;
    }

    const auto maxConns = sm._requestDispatcher->maximumConcurrentConnections();
    if ( sm._spec.preferredChunkSize() == 0 ) {
      const auto autoBlkSize = makeBlksize( _fileSize );
      if ( maxConns == -1 ) {
        _preferredChunkSize = autoBlkSize;
      } else {
        _preferredChunkSize = _fileSize / maxConns;
        if ( _preferredChunkSize < autoBlkSize )
          _preferredChunkSize = autoBlkSize;
        else if ( _preferredChunkSize > zypp::ByteCount( 100, zypp::ByteCount::M ) )
          _preferredChunkSize = zypp::ByteCount( 100, zypp::ByteCount::M );
      }
    } else {
      // spec chunk size overrules the automatic one
      _preferredChunkSize = sm._spec.preferredChunkSize();
    }

    if( srcHashType != targetHashType )
      return setFailed ( zypp::str::Format( "ERROR: Chunk hash types don't match. Source Hash: %1% vs Target Hash: %2%")
                        % zck_hash_name_from_type ( srcHashType )
                        % zck_hash_name_from_type ( targetHashType ) );

    if(!zck_copy_chunks( zck_src, zckTarget ))
      return setFailed ( zypp::str::Format( "Unable to copy chunks from deltafile.") );

    // will reset all chunks that are marked as failed back to missing
    zck_reset_failed_chunks( zckTarget );


    // we calculate what is already downloaded by substracting the block sizes we still need to download from the full file size
    _downloadedMultiByteCount = _fileSize;

    auto chunk = zck_get_first_chunk( zckTarget );
    do {
      // Get validity of current chunk: 1 = valid, 0 = missing, -1 = invalid
      if ( zck_get_chunk_valid( chunk ) == 1 )
        continue;

      zypp::AutoFREE<char> zckDigest( zck_get_chunk_digest( chunk ) );
      UByteArray chksumVec = zypp::Digest::hexStringToUByteArray( std::string_view( zckDigest.value() ) );
      std::string chksumName;
      std::optional<size_t> chksumCompareLen;

      switch ( targetHashType ) {
        case ZCK_HASH_SHA1: {
          chksumName = zypp::Digest::sha1();
          break;
        }
        case ZCK_HASH_SHA256: {
          chksumName = zypp::Digest::sha256();
          break;
        }
        case ZCK_HASH_SHA512: {
          chksumName = zypp::Digest::sha512();
          break;
        }
        case ZCK_HASH_SHA512_128: {
          // defined in zchunk as
          // SHA-512/128 (first 128 bits of SHA-512 checksum)
          chksumName = zypp::Digest::sha512();
          chksumCompareLen = chksumVec.size();
          break;
        }
        default: {
          return setFailed ( zypp::str::Format( "Unsupported chunk hash type: %1%.") % zck_hash_name_from_type( targetHashType ) );
        }
      }

      const auto s = static_cast<off_t>( zck_get_chunk_start( chunk ) );
      const auto l = static_cast<size_t>( zck_get_chunk_comp_size ( chunk ) );

      MIL_MEDIA << "Downloading block " << s << " with length " << l << " checksum " << zckDigest.value() << " type " << chksumName << std::endl;

      _ranges.push_back( Block{
        .start = s,
        .len   = l,
        .chksumtype = chksumName,
        .chksumVec  = std::move( chksumVec ),
        .chksumCompareLen = std::move(chksumCompareLen)
      } );

      // substract the block length from the already downloaded bytes size
      _downloadedMultiByteCount -= l;

    } while ( (chunk = zck_get_next_chunk( chunk )) );

    ensureDownloadsRunning();
  }

  void DLZckState::exit()
  {
    cancelAll( NetworkRequestError() );
  }

  std::shared_ptr<FinishedState> DLZckState::transitionToFinished()
  {
    return std::make_shared<FinishedState>( std::move(_error), stateMachine() );
  }

  void DLZckState::setFinished()
  {
    const auto &spec = stateMachine()._spec;

    zypp::AutoFD target_fd = open( spec.targetPath().asString().c_str(), O_RDONLY );
    if( target_fd < 0 )
      return setFailed ( zypp::str::Format("Unable to open %1%") % spec.targetPath() );

    zypp::AutoDispose<zckCtx *> zckTarget ( zck_create(), []( auto ptr ) { if ( ptr ) zck_free( &ptr ); } );
    if( !zckTarget )
      return setFailed ( zypp::str::Format("%1%") % zck_get_error(nullptr) );

    if(!zck_init_read(zckTarget, target_fd))
      return setFailed ( zypp::str::Format( "Unable to open %1%: %2%") %  spec.targetPath() % zck_get_error(zckTarget) );

    /* Validate the chunk and data checksums for the current file.
     * Returns 0 for error, -1 for invalid checksum and 1 for valid checksum */
    const auto res = zck_validate_checksums( zckTarget );
    if ( res == 0 || res == -1 ) {
      if( zck_is_error(nullptr) ) {
        std::string err = zck_get_error(NULL);
        zck_clear_error(NULL);
        return setFailed( std::move(err) );
      }
      if( zck_is_error(zckTarget) )
        return setFailed( zck_get_error(zckTarget) );
      return setFailed( "zck_validate_checksums returned a unknown error." );
    }

    // everything is valid
    RangeDownloaderBaseState::setFinished();
  }

}

#endif
