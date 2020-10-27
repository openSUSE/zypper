#include <iostream>
#include <sstream>
#include <zypp/base/Logger.h>
#include <zypp/repo/ServiceRepos.h>
#include <zypp/repo/RepoException.h>
#include <zypp/media/MediaException.h>
#include <zypp/parser/RepoFileReader.h>
#include <zypp/media/MediaManager.h>
#include <zypp/parser/RepoindexFileReader.h>
#include <zypp/ExternalProgram.h>

using std::stringstream;
using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  namespace repo
  {
    struct ServiceRepos::Impl
    { virtual ~Impl() {} };

    ///////////////////////////////////////////////////////////////////

    struct RIMServiceRepos : public ServiceRepos::Impl
    {
      RIMServiceRepos( const Pathname & /*root_r*/,
		       const ServiceInfo & service,
		       const ServiceRepos::ProcessRepo & callback,
		       const ProgressData::ReceiverFnc & progress = ProgressData::ReceiverFnc() )
      {
	// repoindex.xml must be fetched always without using cookies (bnc #573897)
	Url serviceUrl( service.url() );
	serviceUrl.setQueryParam( "cookies", "0" );

	// download the repo index file
	media::MediaManager mediamanager;
	media::MediaAccessId mid = mediamanager.open( serviceUrl );
	mediamanager.attach( mid );
        mediamanager.provideFile( mid, OnMediaLocation("repo/repoindex.xml") );
	Pathname path = mediamanager.localPath(mid, "repo/repoindex.xml" );
	try {
	  parser::RepoindexFileReader reader(path, callback);
	  service.setProbedTtl( reader.ttl() );	// hack! Modifying the const Service to set parsed TTL
	  mediamanager.release( mid );
	  mediamanager.close( mid );
	} catch ( const Exception &e ) {
	  //Reader throws a bare exception, we need to translate it into something our calling
	  //code expects and handles (bnc#1116840)
	  ZYPP_CAUGHT ( e );
	  ServicePluginInformalException ex ( e.msg() );
	  ex.remember( e );
	  ZYPP_THROW( ex );
	}
      }
    };

    ///////////////////////////////////////////////////////////////////

    struct PluginServiceRepos : public ServiceRepos::Impl
    {
      PluginServiceRepos( const Pathname & root_r,
			  const ServiceInfo & service,
			  const ServiceRepos::ProcessRepo & callback,
			  const ProgressData::ReceiverFnc & progress = ProgressData::ReceiverFnc() )
      {
	// bsc#1080693: Service script needs to be executed chrooted to the RepoManagers rootDir.
	// The service is not aware of the rootDir, so it's explicitly passed and needs to be
	// stripped from the URLs path.
	stringstream buffer;

	ExternalProgram::Arguments args;
	args.reserve( 3 );
	args.push_back( "/bin/sh" );
	args.push_back( "-c" );
	args.push_back( Pathname::stripprefix( root_r, service.url().getPathName() ).asString() );
	ExternalProgramWithStderr prog( args, root_r );
	prog >> buffer;

	if ( prog.close() != 0 )
	{
	  // ServicePluginInformalException:
	  // Ignore this error but we'd like to report it somehow...
	  std::string errbuffer;
	  prog.stderrGetUpTo( errbuffer, '\0' );
	  ERR << "Capture plugin error:[" << endl << errbuffer << endl << ']' << endl;
	  ZYPP_THROW( repo::ServicePluginInformalException( service, errbuffer ) );
	}
	parser::RepoFileReader parser( buffer, callback );
      }
    };

    ///////////////////////////////////////////////////////////////////

    ServiceRepos::ServiceRepos( const Pathname & root_r,
				const ServiceInfo & service,
				const ServiceRepos::ProcessRepo & callback,
				const ProgressData::ReceiverFnc &progress )
    : _impl( ( service.type() == ServiceType::PLUGIN )
    ? static_cast<ServiceRepos::Impl*>( new PluginServiceRepos( root_r, service, callback, progress ) )
    : static_cast<ServiceRepos::Impl*>( new RIMServiceRepos( root_r, service, callback, progress ) ) )
    {}

    ServiceRepos::~ServiceRepos()
    {}

  } // namespace repo
  ///////////////////////////////////////////////////////////////////
} //namespace zypp
///////////////////////////////////////////////////////////////////
