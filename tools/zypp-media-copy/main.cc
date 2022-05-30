/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <csignal>
#include <zypp-core/zyppng/base/private/linuxhelpers_p.h>
#include <zypp-core/fs/PathInfo.h>
#include <zypp-media/ng/worker/ProvideWorker>
#include <zypp-media/ng/private/providemessage_p.h>
#include <zypp-core/zyppng/base/Signals>

class CopyProvider : public zyppng::worker::ProvideWorker
{
  public:
    CopyProvider( std::string_view workerName ) : ProvideWorker( workerName ) { }

    void immediateShutdown() override { }

  protected:
    // ProvideWorker interface
    zyppng::expected<zyppng::worker::WorkerCaps> initialize(const zyppng::worker::Configuration &conf) override {

      zyppng::worker::WorkerCaps caps;
      caps.set_worker_type ( zyppng::worker::WorkerCaps::CPUBound );
      caps.set_cfg_flags(
        zyppng::worker::WorkerCaps::Flags (
          zyppng::worker::WorkerCaps::Pipeline
          | zyppng::worker::WorkerCaps::ZyppLogFormat
          | zyppng::worker::WorkerCaps::FileArtifacts
          )
        );

      return zyppng::expected<zyppng::worker::WorkerCaps>::success(caps);
    }

    void provide() override {
      auto &queue = requestQueue();

      if ( !queue.size() )
        return;


      auto req = queue.front();
      queue.pop_front();

      // here we only receive request codes, we only support Provide messages, all others are rejected
      // Cancel is never to be received here
      if ( req->_spec.code() != zyppng::ProvideMessage::Code::Provide ) {
        req->_state = zyppng::worker::ProvideWorkerItem::Finished;
        provideFailed( req->_spec.requestId()
          , zyppng::ProvideMessage::Code::BadRequest
          , "Request type not implemented"
          , false
          , {} );
        return;
      }

      zypp::Url url;
      const auto &urlVal = req->_spec.value( zyppng::ProvideMsgFields::Url );
      try {
        url = zypp::Url( urlVal.asString() );
      }  catch ( const zypp::Exception &excp ) {
        ZYPP_CAUGHT(excp);

        std::string err = zypp::str::Str() << "Invalid URL in request: " << urlVal.asString();
        ERR << err << std::endl;

        req->_state = zyppng::worker::ProvideWorkerItem::Finished;
        provideFailed( req->_spec.requestId()
          , zyppng::ProvideMessage::Code::BadRequest
          , err
          , false
          , {} );

        return;
      }

      auto targetFileName = req->_spec.value( zyppng::ProvideMsgFields::Filename );
      if ( !targetFileName.valid() ) {
        req->_state = zyppng::worker::ProvideWorkerItem::Finished;
        provideFailed( req->_spec.requestId()
          , zyppng::ProvideMessage::Code::BadRequest
          , "Copy worker requires a target filename hint"
          , false
          , {} );
        return;
      }
      zypp::Pathname targetFilePath( targetFileName.asString() );

      zypp::PathInfo pi( url.getPathName() );
      if ( !pi.isExist() ) {
        req->_state = zyppng::worker::ProvideWorkerItem::Finished;
        provideFailed( req->_spec.requestId()
          , zyppng::ProvideMessage::Code::NotFound
          , zypp::str::Str() << "File " << pi.path() << " not found."
          , false
          , {} );
        return;
      }

      if ( !pi.isFile() )  {
        provideFailed( req->_spec.requestId()
          , zyppng::ProvideMessage::Code::NotAFile
          , zypp::str::Str() << "Path " << pi.path() << " exists, but its not a file"
          , false
          , {} );
        return;
      }

      auto res = zypp::filesystem::hardlinkCopy( pi.path(), targetFileName.asString() );
      if ( res == 0 ) {
        provideSuccess( req->_spec.requestId(), false, targetFilePath );
      } else {
        provideFailed( req->_spec.requestId()
          , zyppng::ProvideMessage::Code::BadRequest
          , zypp::str::Str() << "Failed to create file " << targetFilePath
          , false
          , {} );
      }
    }

    void cancel(const std::deque<zyppng::worker::ProvideWorkerItemRef>::iterator &i ) override {
      ERR << "Bug, cancel should never be called for running items" << std::endl;
    }

};

int main( int argc, char *argv[] )
{
  // lets ignore those, SIGPIPE is better handled via the EPIPE error, and we do not want anyone
  // to CTRL+C us
  zyppng::blockSignalsForCurrentThread( { SIGPIPE, SIGINT } );

  auto provider = std::make_shared<CopyProvider>("zypp-media-copy");
  auto res = provider->run (STDIN_FILENO, STDOUT_FILENO);
  provider->immediateShutdown();
  if ( res )
    return 0;

  //@TODO print error
  return 1;
}
