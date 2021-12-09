/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_REPO_SERVICE_REPOS
#define ZYPP_REPO_SERVICE_REPOS

#include <zypp/base/NonCopyable.h>
#include <zypp-core/ui/ProgressData>
#include <zypp/ServiceInfo.h>
#include <zypp/RepoInfo.h>

namespace zypp
{
  namespace repo
  {
    /**
     * Retrieval of repository list for a service.
     */
    class ServiceRepos : private base::NonCopyable
    {
    public:
     /**
      * Return false from the callback to get a \ref AbortRequestException
      * to be thrown and the processing to be canceled.
      */
      typedef function< bool( const RepoInfo & )> ProcessRepo;

      /**
       * bsc#1080693: Explicitly pass the RemoManagers rootDir until it can be queried from the ServiceInfo.
       * Required to execute plugin services chrooted.
       */
      ServiceRepos( const Pathname & root_r,
                    const ServiceInfo & service,
                    const ProcessRepo & callback,
                    const ProgressData::ReceiverFnc &progress = ProgressData::ReceiverFnc() );
      ~ServiceRepos();

    public:
      struct Impl;	//!< Expose type only
    private:
      RW_pointer<Impl> _impl;
    };
  } // ns repo
} // ns zypp

#endif
