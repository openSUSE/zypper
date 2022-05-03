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

#include <zypp-media/ng/worker/MountingWorker>

class DirProvider : public zyppng::worker::MountingWorker
{
  public:
    DirProvider( std::string_view workerName );
    ~DirProvider();
  protected:
    void handleMountRequest ( zyppng::worker::ProvideWorkerItem &req ) override;
    void unmountDevice ( zyppng::worker::Device &dev ) override;

};

#endif
