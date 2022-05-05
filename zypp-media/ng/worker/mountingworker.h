/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_MEDIA_NG_WORKER_MOUNTINGWORKER_H_INCLUDED
#define ZYPP_MEDIA_NG_WORKER_MOUNTINGWORKER_H_INCLUDED

#include <zypp-media/ng/worker/ProvideWorker>
#include <zypp-media/ng/worker/DeviceDriver>
#include <zypp-core/zyppng/base/Signals>
#include <any>
#include <unordered_map>

namespace zyppng::worker
{
  class MountingWorker : public zyppng::worker::ProvideWorker
  {
    public:
      MountingWorker( std::string_view workerName, DeviceDriverRef driver );
      ~MountingWorker();

      void immediateShutdown() override;

    protected:
      // ProvideWorker interface
      zyppng::expected<zyppng::worker::WorkerCaps> initialize(const zyppng::worker::Configuration &conf) override;
      void provide() override;
      void cancel( const std::deque<zyppng::worker::ProvideWorkerItemRef>::iterator &i ) override;

    private:
      DeviceDriverRef _driver;
      bool _devicesDetected = false; //< We delay device detection to the first attach request, to avoid doing it without needing it
  };
}

#endif
