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

#include <zypp-media/ng/worker/DeviceDriver>
#include <zypp-core/zyppng/base/Signals>
#include <any>
#include <unordered_map>

class DiscProvider : public zyppng::worker::DeviceDriver
{
public:
  DiscProvider();
  ~DiscProvider();

  // DeviceDriver interface
  zyppng::worker::AttachResult mountDevice ( const uint32_t id, const zypp::Url &attachUrl, const std::string &attachId, const std::string &label, const zyppng::HeaderValueMap &extras ) override;
  void detectDevices() override;

private:
  bool _devicesDetected = false; //< We delay device detection to the first attach request, to avoid doing it without needing it
};

#endif
