#include <iostream>
#include <sstream>
#include "zypp/base/Logger.h"
#include "zypp/repo/ServiceRepos.h"
#include "zypp/parser/RepoFileReader.h"
#include "zypp/media/MediaManager.h"
#include "zypp/parser/RepoindexFileReader.h"
#include "zypp/ExternalProgram.h"

using std::stringstream;

namespace zypp
{
namespace repo
{

class ServiceRepos::Impl
{
public:
    Impl()
    {
    }
    
    virtual ~Impl()
    {
    }
};

class RIMServiceRepos : public ServiceRepos::Impl
{
public:
    ServiceRepos::ProcessRepo _callback;
    
    RIMServiceRepos(const ServiceInfo &service,
                    const ServiceRepos::ProcessRepo & callback,
                    const ProgressData::ReceiverFnc &progress = ProgressData::ReceiverFnc() )
        : _callback(callback)
    {
      // repoindex.xml must be fetched always without using cookies (bnc #573897)
      Url serviceUrl( service.url() );
      serviceUrl.setQueryParam( "cookies", "0" );
      
      // download the repo index file
      media::MediaManager mediamanager;
      media::MediaAccessId mid = mediamanager.open( serviceUrl );
      mediamanager.attach( mid );
      mediamanager.provideFile( mid, "repo/repoindex.xml" );
      Pathname path = mediamanager.localPath(mid, "repo/repoindex.xml" );
      parser::RepoindexFileReader reader(path, _callback);
      mediamanager.release( mid );
      mediamanager.close( mid );
    }
    
    ~RIMServiceRepos()
    {

    }
};

class PluginServiceRepos : public ServiceRepos::Impl
{
public:
    ServiceRepos::ProcessRepo _callback;
    
    PluginServiceRepos(const ServiceInfo &service,
                      const ServiceRepos::ProcessRepo & callback,
                      const ProgressData::ReceiverFnc &progress = ProgressData::ReceiverFnc() )
        : _callback(callback)
    {
      Url serviceUrl( service.url() );
      stringstream buffer;
     
      ExternalProgram prog(serviceUrl.getPathName(), ExternalProgram::Discard_Stderr, false, -1, true);
      std::string line;
      for(line = prog.receiveLine(); !line.empty(); line = prog.receiveLine() )
          buffer << line;
      prog.close();
      parser::RepoFileReader parser(buffer, _callback);
    }
    
    ~PluginServiceRepos()
    {

    }
};

    
ServiceRepos::ServiceRepos(const ServiceInfo &service,
                           const ServiceRepos::ProcessRepo & callback,
                           const ProgressData::ReceiverFnc &progress)
    : _impl( (service.type() == ServiceType::PLUGIN) ? (ServiceRepos::Impl *)(new PluginServiceRepos(service, callback, progress)) : (ServiceRepos::Impl *)(new RIMServiceRepos(service, callback, progress)))
{
}
    
ServiceRepos::~ServiceRepos()
{
}
    

}
}
