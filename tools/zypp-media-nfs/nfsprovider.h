/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
#ifndef ZYPP_NG_TOOLS_NFSPROVIDER_H_INCLUDED
#define ZYPP_NG_TOOLS_NFSPROVIDER_H_INCLUDED

#include <zypp-media/ng/worker/MountingWorker>

class NfsProvider : public zyppng::worker::MountingWorker
{
  public:
    NfsProvider( std::string_view workerName );
    ~NfsProvider();
  protected:
    void handleMountRequest ( zyppng::worker::ProvideWorkerItem &req ) override;
};

#endif
