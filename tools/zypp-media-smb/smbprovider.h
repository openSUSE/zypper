/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#ifndef ZYPP_NG_TOOLS_SMBPROVIDER_H_INCLUDED
#define ZYPP_NG_TOOLS_SMBPROVIDER_H_INCLUDED

#include <zypp-media/ng/worker/MountingWorker>

class SmbProvider : public zyppng::worker::MountingWorker
{
  public:
    SmbProvider( std::string_view workerName );
    ~SmbProvider();
  protected:
    void handleMountRequest ( zyppng::worker::ProvideWorkerItem &req ) override;
};

#endif
