/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include "common.h"
#include "repos.h"

#include <zypp/media/MediaException.h>

ServiceList get_all_services( Zypper & zypper )
{
  RepoManager & manager( zypper.repoManager() );
  ServiceList services;

  try
  {
    // RIS type services
    for_( it, manager.serviceBegin(), manager.serviceEnd() )
    {
      services.insert( services.end(), ServiceInfo_Ptr( new ServiceInfo( *it ) ) );	// copy needed?
    }

    // non-services repos
    for_( it, manager.repoBegin(), manager.repoEnd() )
    {
      if ( !it->service().empty() )
        continue;
      services.insert( services.end(), RepoInfo_Ptr( new RepoInfo( *it ) ) );	// copy needed?
    }
  }
  catch ( const Exception &e )
  {
    ZYPP_CAUGHT(e);
    zypper.out().error( e, _("Error reading services:") );
    exit( ZYPPER_EXIT_ERR_ZYPP );
  }

  return services;
}

bool match_service( Zypper & zypper, std::string str, repo::RepoInfoBase_Ptr & service_ptr, bool looseAuth, bool looseQuery  )
{
  ServiceList known = get_all_services( zypper );
  bool found = false;

  unsigned number = 0;	// service number start with 1
  for_( known_it, known.begin(), known.end() )
  {
    ++number;
    unsigned tmp = 0;
    safe_lexical_cast( str, tmp ); // try to make an int out of the string

    try
    {
      // match by alias or number
      found = (*known_it)->alias() == str || tmp == number;

      // match by URL
      if ( !found )
      {
        url::ViewOption urlview = url::ViewOption::DEFAULTS + url::ViewOption::WITH_PASSWORD;
        if ( looseAuth )//zypper.cOpts().count("loose-auth") )
        {
          urlview = urlview - url::ViewOptions::WITH_PASSWORD - url::ViewOptions::WITH_USERNAME;
        }
        if ( looseQuery ) //zypper.cOpts().count("loose-query") )
          urlview = urlview - url::ViewOptions::WITH_QUERY_STR;

        ServiceInfo_Ptr s_ptr = dynamic_pointer_cast<ServiceInfo>(*known_it);

        if ( !( urlview.has(url::ViewOptions::WITH_PASSWORD) && urlview.has(url::ViewOptions::WITH_QUERY_STR) ) )
        {
          if ( s_ptr )
            found = Url(str).asString(urlview) == s_ptr->url().asString(urlview);
          else
          {
            RepoInfo_Ptr r_ptr = dynamic_pointer_cast<RepoInfo>(*known_it);
            if ( !r_ptr->baseUrlsEmpty() )
            {
              for_( urlit, r_ptr->baseUrlsBegin(), r_ptr->baseUrlsEnd() )
                if ( urlit->asString(urlview) == Url(str).asString(urlview) )
                {
                  found = true;
                  break;
                }
            }
          }
        }
        else
        {
          if ( s_ptr )
            found = ( Url(str) == s_ptr->url() );
          else
          {
            RepoInfo_Ptr r_ptr = dynamic_pointer_cast<RepoInfo>(*known_it);
            if ( !r_ptr->baseUrlsEmpty() )
            {
              found = find( r_ptr->baseUrlsBegin(), r_ptr->baseUrlsEnd(), Url(str) ) != r_ptr->baseUrlsEnd();
            }
          }
        }
      }
      if ( found )
      {
        service_ptr = *known_it;
        break;
      }
    }
    catch( const url::UrlException & )
    {}

  } // END for all known services

  return found;
}

bool refresh_service(Zypper & zypper, const ServiceInfo & service, RepoManager::RefreshServiceFlags flags_r)
{
  MIL << "going to refresh service '" << service.alias() << "'" << endl;
  init_target( zypper );	// need targetDistribution for service refresh
  RepoManager & manager( zypper.repoManager() );

  bool error = true;
  try
  {
    zypper.out().info( str::form(_("Refreshing service '%s'."), service.asUserString().c_str() ) );
    manager.refreshService( service, flags_r );
    error = false;
  }
  catch ( const repo::ServicePluginInformalException & e )
  {
    ZYPP_CAUGHT( e );
    zypper.out().error( e, str::form(_("Problem retrieving the repository index file for service '%s':"),
                                     service.asUserString().c_str()));
    zypper.out().warning( str::form( _("Skipping service '%s' because of the above error."),
                                     service.asUserString().c_str()));
    // this is just an informal note. The service will be used as is (usually empty)
    error = false;
  }
  catch ( const media::MediaException & e )
  {
    ZYPP_CAUGHT( e );
    zypper.out().error( e, str::form(_("Problem retrieving the repository index file for service '%s':"),
                                     service.asUserString().c_str() ),
                        _("Check if the URI is valid and accessible.") );
    zypper.setExitCode( ZYPPER_EXIT_ERR_ZYPP );
  }

  return error;
}

void remove_service( Zypper & zypper, const ServiceInfo & service )
{
  RepoManager & manager( zypper.repoManager() );

  zypper.out().info( str::Format(_("Removing service '%s':")) % service.asUserString() );
  manager.removeService( service );
  MIL << "Service '" << service.alias() << "' has been removed." << endl;
  zypper.out().info( str::Format(_("Service '%s' has been removed.")) % service.asUserString() );
}


