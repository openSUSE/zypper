/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------*/

#include <zypp-curl/ng/network/private/downloader_p.h>
#include <zypp-curl/ng/network/private/mediadebug_p.h>
#include <zypp-curl/ng/network/private/networkrequesterror_p.h>
#include <zypp-core/fs/PathInfo.h>

#include "metalink_p.h"
#include "final_p.h"

 #include <iostream>
#include <fstream>

namespace zyppng {

  DlMetalinkState::DlMetalinkState(zypp::media::MediaBlockList &&blockList, std::vector<Url> &&mirrors, DownloadPrivate &parent)
    : RangeDownloaderBaseState( std::move(mirrors), parent )
    , _blockList( std::move(blockList) )
  {
    MIL_MEDIA << "About to enter DlMetalinkState for url " << parent._spec.url() << std::endl;
  }

  void DlMetalinkState::enter()
  {
    auto &sm = stateMachine();
    const auto &spec = sm._spec;

    _fileSize = sm._spec.expectedFileSize();

    //first we try to reuse blocks from the deltafile , if we have one
    if ( !spec.deltaFile().empty() ) {
      zypp::PathInfo dFileInfo ( spec.deltaFile() );
      if ( dFileInfo.isFile() && dFileInfo.isR() ) {
        FILE *f = fopen( spec.targetPath().asString().c_str(), "w+b" );
        if ( !f ) {
          setFailed( NetworkRequestErrorPrivate::customError( NetworkRequestError::InternalError, zypp::str::Format("Failed to open target file.(errno %1%)" ) % errno ) );
          return;
        }

        try {
          _blockList.reuseBlocks ( f, spec.deltaFile().asString() );
        } catch ( ... ) { }

        fclose( f );
      } else {
        DBG << "Delta XFER: Delta file: " << spec.deltaFile() << " does not exist or is not readable." << std::endl;
      }
    } else {
      DBG << "Delta XFER: No delta file given, can not reuse blocks." << std::endl;
    }

    // setup the base downloader
    _error = {};
    _ranges.clear();
    _failedRanges.clear();
    _downloadedMultiByteCount = 0;

    if ( _blockList.haveFileChecksum() ) {
      _fileChecksumType = _blockList.fileChecksumType();
      _fileChksumVec    = _blockList.getFileChecksum();
    }

    const size_t fLen = _blockList.getFilesize();
    if ( _fileSize > 0 ) {
      // check if the file size as reported by zchunk is equal to the one we expect
      if ( _fileSize != fLen ) {
        return setFailed( NetworkRequestErrorPrivate::customError(
          NetworkRequestError::ExceededMaxLen,
          zypp::str::Format("Metalink file reports a different filesize than what was expected ( Meta: %1% != Exp: %2%).") % fLen % _fileSize )
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

    DBG << "Downloading " << sm._spec.url() << " with " << _preferredChunkSize << " chunk size over " << maxConns << std::endl;

    // remember how many bytes we need to download
    size_t bytesToDl = 0;
    for ( size_t i = 0; i < _blockList.numBlocks(); i++ ) {
      const auto &mediaBlock = _blockList.getBlock( i );
      _ranges.push_back(
        Block{
          .start = mediaBlock.off,
          .len   = mediaBlock.size,
          .chksumtype = _blockList.getChecksumType(),
          .chksumVec  = _blockList.getChecksum ( i )
        } );

      bytesToDl += mediaBlock.size;
    }
    // substract the length of the blocks we have to download from the overall file size
    _downloadedMultiByteCount = fLen - bytesToDl;

    ensureDownloadsRunning();
  }

  void DlMetalinkState::exit()
  {
    cancelAll( NetworkRequestError() );
  }

  void DlMetalinkState::setFinished()
  {
    if ( _fileChecksumType.size() && _fileChksumVec ) {
      //TODO move this into a external application so we do not need to block on it
      //need to check file digest
      zypp::Digest dig;
      dig.create( _fileChecksumType );

      std::ifstream istrm( stateMachine()._spec.targetPath().asString(), std::ios::binary);
      if ( !istrm.is_open() ) {
        setFailed( "Failed to verify file digest (Could not open target file)." );
        return;
      }
      if ( !dig.update( istrm ) ) {
        setFailed( "Failed to verify file digest (Could not read target file)." );
        return;
      }

      const auto &calculatedChksum = dig.digestVector();
      if ( *_fileChksumVec != calculatedChksum ) {
        setFailed( "Failed to verify file digest (Checksum did not match)." );
        return;
      }
    }
    RangeDownloaderBaseState::setFinished();
  }

  std::shared_ptr<FinishedState> DlMetalinkState::transitionToFinished()
  {
    return std::make_shared<FinishedState>( std::move(_error), stateMachine() );
  }

}
