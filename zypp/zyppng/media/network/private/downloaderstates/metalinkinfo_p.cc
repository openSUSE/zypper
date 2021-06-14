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
#include <zypp/zyppng/media/network/private/request_p.h>
#include <zypp/zyppng/media/network/private/networkrequesterror_p.h>

#include "metalinkinfo_p.h"

namespace zyppng {

  namespace  {

    constexpr auto minMetalinkProbeSize = 256; //< The maximum probe size we download before we decide we really got no metalink file

    bool looks_like_metalink_data( const std::vector<char> &data )
    {
      if ( data.empty() )
        return false;

      const char *p = data.data();
      while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
        p++;

      if (!strncasecmp(p, "<?xml", 5))
      {
        while (*p && *p != '>')
          p++;
        if (*p == '>')
          p++;
        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
          p++;
      }
      bool ret = !strncasecmp( p, "<metalink", 9 ) ? true : false;
      return ret;
    }

    bool looks_like_metalink_file( const zypp::Pathname &file )
    {
      std::unique_ptr<FILE, decltype(&fclose)> fd( fopen( file.c_str(), "r" ), &fclose );
      if ( !fd )
        return false;
      return looks_like_metalink_data( zyppng::peek_data_fd( fd.get(), 0, minMetalinkProbeSize ) );
    }
  }

  DlMetaLinkInfoState::DlMetaLinkInfoState(DownloadPrivate &parent)
    : BasicDownloaderStateBase( parent )
  {
    MIL_MEDIA << "Downloading metalink on " << parent._spec.url() << std::endl;
  }

  std::shared_ptr<FinishedState> DlMetaLinkInfoState::transitionToFinished()
  {
    MIL_MEDIA << "Downloading on " << stateMachine()._spec.url() << " transition to final state. " << std::endl;
    return std::make_shared<FinishedState>( std::move(_error), stateMachine() );
  }

  bool DlMetaLinkInfoState::initializeRequest( std::shared_ptr<Request> r )
  {
    r->transferSettings().addHeader("Accept: */*, application/metalink+xml, application/metalink4+xml");
    return BasicDownloaderStateBase::initializeRequest(r);
  }

  void DlMetaLinkInfoState::gotFinished()
  {
    // some proxies do not store the content type, so also look at the file to find
    // out if we received a metalink (bnc#649925)
    if ( !_isMetalink )
      _isMetalink = looks_like_metalink_file( _request->targetFilePath() );
    if ( !_isMetalink ) {
      // Move to finished state
      MIL << "Downloading on " << stateMachine()._spec.url() << " was successful, no metalink data. " << std::endl;
      return BasicDownloaderStateBase::gotFinished();
    }

    // Move to Prepare Multi state
    MIL << "Downloading on " << stateMachine()._spec.url() << " returned a Metalink " << std::endl;
    _sigGotMetalink.emit();
  }

  void DlMetaLinkInfoState::handleRequestProgress(NetworkRequest &req, off_t dltotal, off_t dlnow)
  {
    auto &sm = stateMachine();

    if ( !_isMetalink && dlnow < minMetalinkProbeSize ) {
      // can't tell yet, ...
      return sm._sigAlive.emit( *sm.z_func(), dlnow );
    }

    if ( !_isMetalink ) {
      std::string cType = req.contentType();
      _isMetalink = ( cType.find("application/metalink+xml") == 0 || cType.find("application/metalink4+xml") == 0 );
    }

    if ( !_isMetalink ) {
      _isMetalink = looks_like_metalink_data( req.peekData( 0, minMetalinkProbeSize ) );
    }

    if ( _isMetalink ) {
      // this is a metalink file change the expected filesize
      if ( zypp::ByteCount( 2, zypp::ByteCount::MB) < static_cast<zypp::ByteCount::SizeType>( dlnow ) ) {
        WAR << "Metalink file exceeds 2MB in filesize, aborting."<<std::endl;
        sm._requestDispatcher->cancel( req, NetworkRequestErrorPrivate::customError( NetworkRequestError::ExceededMaxLen ) );
        return;
      }
    } else {
      // still no metalink, we assume a normal download, not perfect though
      return BasicDownloaderStateBase::handleRequestProgress( req, dltotal, dlnow );
    }
  }


}
