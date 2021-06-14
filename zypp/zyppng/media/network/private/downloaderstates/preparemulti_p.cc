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
#include <zypp/media/CurlHelper.h>

#include "preparemulti_p.h"

#if ENABLE_ZCHUNK_COMPRESSION
#include "zck_p.h"
#endif

namespace zyppng {

  PrepareMultiState::PrepareMultiState(DownloadPrivate &parent) : SimpleState( parent )
  {
    MIL_MEDIA << "About to enter PrepareMultiState" << std::endl;
  }

  void PrepareMultiState::enter( )
  {
    auto &sm = stateMachine();
    const auto &spec = sm._spec;
    const auto &url = spec.url();
    const auto &targetPath = spec.targetPath();
#if ENABLE_ZCHUNK_COMPRESSION
    _haveZckData = (isZchunkFile( spec.deltaFile() )  && spec.headerSize() > 0);
    DBG_MEDIA << " Upgrading request for URL: "<< url << " to multipart download , which zckunk=" << _haveZckData << std::endl;
#else
    DBG_MEDIA << " Upgrading request for URL: "<< url << " to multipart download , which zckunk=false" << std::endl;
#endif


    //we have a metalink download, lets parse it and see what we got
    _mirrors.clear();

    std::vector<zypp::media::MetalinkMirror> mirrs;

    try {
      zypp::media::MetaLinkParser parser;
      parser.parse( targetPath );

      // we only care about the metalink chunks if we have no zchunk data
#if ENABLE_ZCHUNK_COMPRESSION
      if ( !_haveZckData ) {
#else
      if ( true ) {
#endif
        auto bl = parser.getBlockList();
        if ( !bl.haveBlocks() )
          MIL_MEDIA << "Got no blocks for URL " << spec.url() << " but got filesize? " << bl.getFilesize() << std::endl;
        if ( bl.haveBlocks() || bl.haveFilesize() )
          _blockList = std::move(bl);
      }

      //migrate some settings from the base url to the mirror
      mirrs = parser.getMirrors();
      for ( auto urliter = mirrs.begin(); urliter != mirrs.end(); ++urliter ) {
        try {
          const std::string scheme = urliter->url.getScheme();
          if (scheme == "http" || scheme == "https" || scheme == "ftp" || scheme == "tftp") {
            if ( !sm._requestDispatcher->supportsProtocol( urliter->url )) {
              urliter = mirrs.erase( urliter );
              continue;
            }
            urliter->url = ::internal::propagateQueryParams( urliter->url, url );
            _mirrors.push_back( urliter->url );
          }
        }
        catch (...) {  }
      }

      if ( mirrs.empty() ) {
        mirrs.push_back( { 0, -1, url } );
        _mirrors.push_back( url );
      }

    } catch ( const zypp::Exception &ex ) {
      _error = NetworkRequestErrorPrivate::customError( NetworkRequestError::InternalError, zypp::str::Format("Failed to parse metalink information.(%1%)" ) % ex.asUserString() );
      _sigFailed.emit();
      return;
    }

    if ( mirrs.size() == 0 ) {
      _error = NetworkRequestErrorPrivate::customError( NetworkRequestError::InternalError, zypp::str::Format("Invalid metalink information.( No mirrors in metalink file)" ) );
      _sigFailed.emit();
      return;
    }

    //remove the metalink file
    zypp::filesystem::unlink( targetPath );
    _mirrorControlReadyConn = sm._mirrorControl->connect( &MirrorControl::sigNewMirrorsReady, *this, &PrepareMultiState::onMirrorsReady );

    // this will emit a mirrorsReady signal once some connection tests have been done
    sm._mirrorControl->registerMirrors( mirrs );
  }

  void PrepareMultiState::onMirrorsReady()
  {
    auto &sm = stateMachine();
    const auto &spec = sm._spec;
    const auto &url = spec.url();
    _mirrorControlReadyConn.disconnect();

#if ENABLE_ZCHUNK_COMPRESSION
    if ( _haveZckData  ) {
      _sigFinished.emit();
      return;
    }
#endif

    // we have no zchunk data, so for a multi download we need a blocklist
    if ( !_blockList.haveBlocks() )  {
      //if we have no filesize we can not generate a blocklist, we need to fall back to normal download
      if ( !_blockList.haveFilesize() ) {

        //fall back to normal download but use a mirror from the mirror list
        //otherwise we get HTTPS to HTTP redirect errors
        _sigFallback.emit();
        return;
      } else {
        //we generate a blocklist on the fly based on the filesize

        MIL_MEDIA << "Generate blocklist, since there was none in the metalink file." << url  << std::endl;

        off_t currOff = 0;
        off_t filesize = _blockList.getFilesize();
        while ( currOff <  filesize )  {

          auto blksize = filesize - currOff ;
          if ( blksize > sm._spec.preferredChunkSize() )
            blksize = sm._spec.preferredChunkSize();

          _blockList.addBlock( currOff, blksize );
          currOff += blksize;
        }

        MIL_MEDIA << "Generated blocklist: " << std::endl << _blockList << std::endl << " End blocklist " << std::endl;
      }
    }

    _sigFinished.emit();
  }

  std::shared_ptr<DlNormalFileState> PrepareMultiState::fallbackToNormalTransition()
  {
    MIL_MEDIA << "No blocklist and no filesize, falling back to normal download for URL " << stateMachine()._spec.url() << std::endl;
    auto ptr = std::make_shared<DlNormalFileState>( stateMachine() );
    ptr->_fileMirrors = std::move(_mirrors);

    if ( _blockList.haveFileChecksum() ) {
      ptr->_chksumtype = _blockList.fileChecksumType();
      ptr->_chksumVec  = _blockList.getFileChecksum();
    }

    return ptr;
  }

  std::shared_ptr<DlMetalinkState> PrepareMultiState::transitionToMetalinkDl()
  {
    return std::make_shared<DlMetalinkState>( std::move(_blockList), std::move(_mirrors), stateMachine() );
  }

  std::shared_ptr<FinishedState> PrepareMultiState::transitionToFinished()
  {
    return std::make_shared<FinishedState>( std::move(_error), stateMachine() );
  }

#if ENABLE_ZCHUNK_COMPRESSION
  std::shared_ptr<DLZckHeadState> PrepareMultiState::transitionToZckHeadDl()
  {
    return std::make_shared<DLZckHeadState>( std::move(_mirrors), stateMachine() );
  }

  bool PrepareMultiState::toZckHeadDownloadGuard() const
  {
    return ( stateMachine().hasZckInfo() );
  }
#endif

  bool PrepareMultiState::toMetalinkDownloadGuard() const
  {
#if ENABLE_ZCHUNK_COMPRESSION
    return (!toZckHeadDownloadGuard());
#else
    return true;
#endif
  }

}
