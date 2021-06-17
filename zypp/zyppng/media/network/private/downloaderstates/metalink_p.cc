/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------*/

#include <zypp/zyppng/media/network/private/downloader_p.h>
#include <zypp/zyppng/media/network/private/mediadebug_p.h>
#include <zypp/zyppng/media/network/private/networkrequesterror_p.h>
#include <zypp/PathInfo.h>

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
      }
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
