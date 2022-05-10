/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#ifndef ZYPP_NG_TOOLS_ISOPROVIDER_H_INCLUDED
#define ZYPP_NG_TOOLS_ISOPROVIDER_H_INCLUDED

#include <zypp-media/ng/worker/DeviceDriver>
#include "dirprovider.h"
#include "smbprovider.h"
#include "diskprovider.h"
#include "nfsprovider.h"


struct RaiiHelper
{
  ~RaiiHelper();
  zyppng::worker::DeviceDriverRef _backingDriver;
  std::string id;
};

class IsoProvider : public zyppng::worker::DeviceDriver
{
  public:
    IsoProvider( );
    ~IsoProvider();

    zyppng::worker::AttachResult mountDevice ( const uint32_t id, const zypp::Url &attachUrl, const std::string &attachId, const std::string &label, const zyppng::HeaderValueMap &extras ) override;

  private:
    std::shared_ptr<DirProvider>  _dirWorker;
    std::shared_ptr<DiskProvider> _diskWorker;
    std::shared_ptr<NfsProvider>  _nfsWorker;
    std::shared_ptr<SmbProvider>  _smbWorker;

};

#endif
