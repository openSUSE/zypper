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
#include <zypp-core/Digest.h>
#include <zypp-core/zyppng/base/Signals>

class ChksumProvider : public zyppng::worker::ProvideWorker
{
  public:
    ChksumProvider( std::string_view workerName ) : ProvideWorker( workerName ) { }

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

      std::string chksumType;
      try {
        chksumType = req->_spec.value( std::string_view("chksumType") ).asString();
      } catch ( const zypp::Exception &excp ) {
        ZYPP_CAUGHT(excp);

        std::string err = zypp::str::Str() << "No or invalid chksumType in request";
        ERR << err << std::endl;

        req->_state = zyppng::worker::ProvideWorkerItem::Finished;
        provideFailed( req->_spec.requestId()
          , zyppng::ProvideMessage::Code::BadRequest
          , err
          , false
          , {} );

        return;
      }

      zypp::Pathname file = url.getPathName();
      if ( ! zypp::PathInfo( file ).isFile() ) {
        req->_state = zyppng::worker::ProvideWorkerItem::Finished;
        provideFailed( req->_spec.requestId()
          , zyppng::ProvideMessage::Code::NotFound
          , zypp::str::Str() << "File " << file << " not found."
          , false
          , {} );
        return;
      }

      std::string filesum = zypp::filesystem::checksum( file, chksumType );
      DBG << "Calculated checksum for : " << file << " with type: " << chksumType << " is " << filesum << std::endl;
      provideSuccess( req->_spec.requestId(), false, file, {{chksumType, {filesum}}} );
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

  auto provider = std::make_shared<ChksumProvider>("zypp-media-chksum");
  auto res = provider->run (STDIN_FILENO, STDOUT_FILENO);
  provider->immediateShutdown();
  if ( res )
    return 0;

  //@TODO print error
  return 1;
}
