/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#ifndef ZYPP_NG_TOOLS_NETWORKPROVIDER_H_INCLUDED
#define ZYPP_NG_TOOLS_NETWORKPROVIDER_H_INCLUDED

#include <zypp-media/ng/worker/ProvideWorker>
#include <zypp-core/zyppng/base/Signals>
#include <zypp-core/zyppng/base/AutoDisconnect>
#include <zypp-curl/ng/network/AuthData>
#include <chrono>

namespace zyppng {
  class Downloader;
  class Download;
}

class NetworkProvider;

struct NetworkProvideItem : public zyppng::worker::ProvideWorkerItem
{
public:
  NetworkProvideItem( NetworkProvider &parent, zyppng::ProvideMessage &&spec );
  ~NetworkProvideItem();

  void startDownload( std::shared_ptr<zyppng::Download> &&dl );
  void cancelDownload ();

  std::shared_ptr<zyppng::Download> _dl;
  zypp::Pathname _targetFileName;
  zypp::Pathname _stagingFileName;

  std::chrono::steady_clock::time_point _scheduleAfter = std::chrono::steady_clock::time_point::min();

private:
  void clearConnections ();
  void onStarted      ( zyppng::Download & );
  void onFinished     ( zyppng::Download & );
  void onAuthRequired ( zyppng::Download &,  zyppng::NetworkAuthData &auth, const std::string &availAuth );

private:
  std::vector<zyppng::connection> _connections;
  NetworkProvider &_parent;
};

using NetworkProvideItemRef = std::shared_ptr<NetworkProvideItem>;

class NetworkProvider : public zyppng::worker::ProvideWorker
{
public:
  NetworkProvider( std::string_view workerName );
  void immediateShutdown() override;

protected:
  // ProvideWorker interface
  zyppng::expected<zyppng::worker::WorkerCaps> initialize(const zyppng::worker::Configuration &conf) override;
  void provide() override;
  void cancel(const std::deque<zyppng::worker::ProvideWorkerItemRef>::iterator &i ) override;
  zyppng::worker::ProvideWorkerItemRef makeItem(zyppng::ProvideMessage &&spec) override;

  friend struct NetworkProvideItem;
  void itemStarted  ( NetworkProvideItemRef item );
  void itemFinished ( NetworkProvideItemRef item );
  void itemAuthRequired (NetworkProvideItemRef item, zyppng::NetworkAuthData &auth, const std::string &);

private:
  std::shared_ptr<zyppng::Downloader> _dlManager;
  zypp::Pathname _attachPoint;
};





#endif
