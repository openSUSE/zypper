/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
----------------------------------------------------------------------/
*
* This file contains private API, this might break at any time between releases.
* You have been warned!
*
*/
#ifndef ZYPPNG_MEDIA_MEDIANETWORKSERVER_H_INCLUDED
#define ZYPPNG_MEDIA_MEDIANETWORKSERVER_H_INCLUDED

#include <zypp/zyppng/base/zyppglobal.h>
#include <zypp/zyppng/base/signals.h>
#include <zypp/base/String.h>
#include <zypp/zyppng/base/Base>
#include <zypp/zyppng/io/Socket>
#include <zypp/TmpPath.h>
#include <zypp/zyppng/thread/Wakeup>

#include <sys/types.h>
#include <unistd.h>
#include <memory>
#include <optional>
#include <list>
#include <thread>

namespace google::protobuf {
  class Message;
}

namespace zyppng {

  class Downloader;
  class MediaNetworkConn;

  using RequestId = uint32_t;

  class LIBZYPP_NG_NO_EXPORT MediaNetworkServer : public Base
  {
  public:
    using ConnectionList = std::list< std::shared_ptr<MediaNetworkConn> >;

    MediaNetworkServer();

    void listen ( const std::string &sockPath );

    const zypp::filesystem::TmpDir &workingDir ();
    std::shared_ptr<Downloader> downloader ();

  private:
    void onIncomingConnection ();

  private:
    std::shared_ptr<Socket> _serverSocket;
    std::shared_ptr<Downloader> _downloadManager;
    ConnectionList _clients;
    zypp::filesystem::TmpDir _workingDir;
  };

  class LIBZYPP_NG_NO_EXPORT MediaNetworkConn : public Base
  {
    struct Request;
    using ReqPtr  = std::shared_ptr<Request>;
    using ReqList = std::list< ReqPtr >;
  public:
    MediaNetworkConn ( MediaNetworkServer &server, std::shared_ptr<Socket> &&socket );
    ~MediaNetworkConn ( );

    SignalProxy<void()> sigDisconnected ();

  private:
    void onDisconnected ();
    void onReadyRead ();
    void onError ( Socket::SocketError err );
    void trackRequest ( Request &r );
    void signalRequestStarted ( Request &r );
    void trackedDlFinished ( Request &r );

    template <typename T>
    void sendMessage ( T &m );

  private:
    MediaNetworkServer &_server;
    ReqList _requests;
    std::shared_ptr<Socket> _connection;
    std::optional<int32_t> _pendingMessageSize;
    Signal<void()> _disconnected;
  };

  class LIBZYPP_NG_NO_EXPORT MediaNetworkThread : public Base
  {
  public:
    ~MediaNetworkThread();
    static MediaNetworkThread &instance ();

    static std::string sockPath () {
      static std::string path = zypp::str::Format("zypp-mediasocket-%1%") % getpid();
      return path;
    }

  private:
    MediaNetworkThread();
    void threadMain ();
    std::thread _t;
    zyppng::Wakeup _shutdownSignal;
  };

}



#endif // MEDIANETWORKSERVER_H
