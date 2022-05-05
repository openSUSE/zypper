/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#ifndef ZYPP_NG_TOOLS_DIRPROVIDER_H_INCLUDED
#define ZYPP_NG_TOOLS_DIRPROVIDER_H_INCLUDED

#include <zypp-media/ng/worker/DeviceDriver>

class DirProvider : public zyppng::worker::DeviceDriver
{
  public:
    DirProvider( );
    ~DirProvider();

    zyppng::worker::AttachResult mountDevice ( const uint32_t id, const zypp::Url &attachUrl, const std::string &attachId, const std::string &label, const zyppng::HeaderValueMap &extras ) override;

  protected:
    void unmountDevice ( zyppng::worker::Device &dev ) override;

};

#endif
