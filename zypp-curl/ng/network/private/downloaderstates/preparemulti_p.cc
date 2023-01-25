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
#include <zypp-curl/private/curlhelper_p.h>
#include <zypp-curl/parser/ZsyncParser>
#include <zypp-core/fs/PathInfo.h>

#include "preparemulti_p.h"

#if ENABLE_ZCHUNK_COMPRESSION
#include "zck_p.h"
#endif

namespace zyppng {

  PrepareMultiState::PrepareMultiState( std::shared_ptr<Request> oldReq, Mode m, DownloadPrivate &parent )
    : SimpleState( parent )
    , _mode(m)
    , _oldRequest( oldReq )
  {
    MIL << "About to enter PrepareMultiState for URL: " << parent._spec.url() << std::endl;
  }

  void PrepareMultiState::enter( )
  {
    auto &sm = stateMachine();
    const auto &spec = sm._spec;
    const auto &url = spec.url();
    const auto &targetPath = spec.targetPath();
#if ENABLE_ZCHUNK_COMPRESSION
    _haveZckData = (isZchunkFile( spec.deltaFile() )  && spec.headerSize() > 0);
    MIL << " Upgrading request for URL: "<< url << " to multipart download , which zckunk=" << _haveZckData << std::endl;
#else
    MIL << " Upgrading request for URL: "<< url << " to multipart download , which zckunk=false" << std::endl;
#endif


    //we have a metalink download, lets parse it and see what we got
    _mirrors.clear();

    std::vector<zypp::media::MetalinkMirror> mirrs;

    try {

      const auto &parseMetadata = [&]( auto &&parser ) {
        using T = std::decay_t<decltype (parser)>;
        constexpr auto metalinkMode = std::is_same< T, zypp::media::MetaLinkParser>();

        parser.parse( targetPath );

        // we only care about the metalink chunks if we have no zchunk data
  #if ENABLE_ZCHUNK_COMPRESSION
        if ( !_haveZckData ) {
  #else
        if ( true ) {
  #endif
          auto bl = parser.getBlockList();
          if ( !bl.haveBlocks() )
            MIL << "Got no blocks for URL " << spec.url() << " but got filesize? " << bl.getFilesize() << std::endl;
          if ( bl.haveBlocks() || bl.haveFilesize() )
            _blockList = std::move(bl);
        }

        //migrate some settings from the base url to the mirror
        if constexpr ( !metalinkMode ) {
          const auto &urlList = parser.getUrls();
          std::for_each( urlList.begin(), urlList.end(), [&]( const auto &url ) {
            mirrs.push_back( { 0, -1, url } );
          });
        } else {
          mirrs = parser.getMirrors();
        }

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
      };

      switch( _mode ) {
        case Zsync: {
          parseMetadata( zypp::media::ZsyncParser() );
          break;
        }
        case Metalink: {
          parseMetadata( zypp::media::MetaLinkParser() );
          break;
        }
      }
    } catch ( const zypp::Exception &ex ) {
      const auto &err = zypp::str::Format("Failed to parse metalink information.(%1%)" ) % ex.asUserString();
      WAR << err << std::endl;
      _error = NetworkRequestErrorPrivate::customError( NetworkRequestError::InternalError, err);
      _sigFailed.emit();
      return;
    }

    if ( mirrs.size() == 0 ) {
      const auto &err =  zypp::str::Format("Invalid metalink information.( No mirrors in metalink file)" );
      WAR << err << std::endl;
      _error = NetworkRequestErrorPrivate::customError( NetworkRequestError::InternalError, err );
      _sigFailed.emit();
      return;
    }

    //remove the metalink file
    zypp::filesystem::unlink( targetPath );
    _mirrorControlReadyConn = sm._mirrorControl->connect( &MirrorControl::sigNewMirrorsReady, *this, &PrepareMultiState::onMirrorsReady );

    // this will emit a mirrorsReady signal once some connection tests have been done
    sm._mirrorControl->registerMirrors( mirrs );
  }

  void PrepareMultiState::exit()
  {
    // if we did not pass on the existing request to the next state we destroy it here
    if ( _oldRequest )
      _oldRequest.reset();
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

        MIL << "Generate blocklist, since there was none in the metalink file." << url  << std::endl;

        off_t currOff = 0;
        off_t filesize = _blockList.getFilesize();
        const auto &prefSize = std::max<zypp::ByteCount>(  sm._spec.preferredChunkSize(), zypp::ByteCount(4, zypp::ByteCount::K) );

        while ( currOff <  filesize )  {

          auto blksize = filesize - currOff ;
          if ( blksize > prefSize )
            blksize = prefSize;

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
    MIL << "No blocklist and no filesize, falling back to normal download for URL " << stateMachine()._spec.url() << std::endl;
    std::shared_ptr<DlNormalFileState> ptr;
    if ( _oldRequest ) {
      ptr = std::make_shared<DlNormalFileState>( std::move(_oldRequest), stateMachine() );
    } else {
      ptr = std::make_shared<DlNormalFileState>( stateMachine() );
    }

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
    if ( _oldRequest )
      return std::make_shared<DLZckHeadState>( std::move(_mirrors), std::move(_oldRequest), stateMachine() );
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
