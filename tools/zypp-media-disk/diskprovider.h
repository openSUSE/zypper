/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#ifndef ZYPP_NG_TOOLS_DISKPROVIDER_H_INCLUDED
#define ZYPP_NG_TOOLS_DISKPROVIDER_H_INCLUDED

#include <zypp-media/ng/worker/MountingWorker>

class DiskProvider : public zyppng::worker::MountingWorker
{
  public:
    DiskProvider( std::string_view workerName );
    ~DiskProvider();
  protected:
    void handleMountRequest ( zyppng::worker::ProvideWorkerItem &req ) override;

};

#endif
