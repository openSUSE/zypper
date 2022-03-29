/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#ifndef ZYPP_NG_TOOLS_DISCPROVIDER_H_INCLUDED
#define ZYPP_NG_TOOLS_DISCPROVIDER_H_INCLUDED

#include <zypp-media/ng/worker/ProvideWorker>
#include <zypp-core/zyppng/base/Signals>
#include <any>
#include <unordered_map>

struct Device
{
  std::string  _path;           //!< Path of the device node
  unsigned int _maj_nr    = 0;  //!< Major number of the device
  unsigned int _min_nr    = 0;  //!< Minor number of the device
  zypp::Pathname _mountPoint = {}; //!< Mountpoint of the device, if empty dev is not mounted
  std::unordered_map<std::string, std::any> _properties = {};
};

struct AttachedMedia
{
  std::shared_ptr<Device> _dev;
  zypp::Pathname _attachRoot;
};

class DiscProvider : public zyppng::worker::ProvideWorker
{
public:
  DiscProvider( std::string_view workerName );
  ~DiscProvider();

  void detectDevices();
  void immediateShutdown() override;

protected:
  // ProvideWorker interface
  zyppng::expected<zyppng::worker::WorkerCaps> initialize(const zyppng::worker::Configuration &conf) override;
  void provide() override;
  void cancel(const std::deque<zyppng::worker::ProvideWorkerItemRef>::iterator &i ) override;
  void unmountDevice ( Device &dev );

private:
  zypp::Pathname _attachRoot;
  std::vector<std::shared_ptr<Device>> _sysDevs;
  std::unordered_map<std::string, AttachedMedia> _attachedMedia;
  bool _devicesDetected = false; //< We delay device detection to the first attach request, to avoid doing it without needing it
};

#endif
