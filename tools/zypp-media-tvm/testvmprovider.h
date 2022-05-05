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

/*!
 * \file Contains a testprovider to simulate volatile mounting devices.
 *       This is only used in the zypp testsuite.
 */

class TestVMProvider : public zyppng::worker::DeviceDriver
{
public:

  TestVMProvider();
  ~TestVMProvider();

  // DeviceDriver interface
  zyppng::expected<zyppng::worker::WorkerCaps> initialize(const zyppng::worker::Configuration &conf) override;
  zyppng::worker::AttachResult mountDevice ( const uint32_t id, const zypp::Url &attachUrl, const std::string &attachId, const std::string &label, const zyppng::HeaderValueMap &extras ) override;
  void detectDevices();

protected:
  void unmountDevice ( zyppng::worker::Device &dev ) override;

private:
  zypp::Pathname _provRoot;
};

#endif
