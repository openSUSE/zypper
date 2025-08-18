/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include "common.h"
#include "repos.h"

#include <zypp-media/MediaException>

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

void get_services_helper( Zypper & zypper, const std::string & spec, ServiceList & services, std::list<std::string> & not_found )
{
  repo::RepoInfoBase_Ptr service;

  if ( !match_service( zypper, spec, service, false, false ) )
  {
    not_found.push_back( spec );
    return;
  }

  // service found
  // is it a duplicate? compare by alias and URIs
  //! \todo operator== in RepoInfo?
  bool duplicate = false;
  for_( serv_it, services.begin(), services.end() )
  {
    ServiceInfo_Ptr s_ptr = dynamic_pointer_cast<ServiceInfo>(*serv_it);
    ServiceInfo_Ptr current_service_ptr = dynamic_pointer_cast<ServiceInfo>(service);

    // one is a service, the other is a repo
    if ( s_ptr && !current_service_ptr )
      continue;

    // service
    if ( s_ptr )
    {
      if ( s_ptr->alias() == current_service_ptr->alias()
        && s_ptr->url() == current_service_ptr->url() )
      {
        duplicate = true;
        break;
      }
    }
    // repo
    else if ( repo_cmp_alias_urls( *dynamic_pointer_cast<RepoInfo>(service),
      *dynamic_pointer_cast<RepoInfo>(*serv_it) ) )
    {
      duplicate = true;
      break;
    }
  } // END for all found so far

  if ( !duplicate )
    services.push_back( service );
}

void report_unknown_services( Out & out, const std::list<std::string> & not_found )
{
  if ( not_found.empty() )
    return;

  for_( it, not_found.begin(), not_found.end() )
    out.error( str::Format(_("Service '%s' not found by its alias, number, or URI.")) % *it );

  out.info( str::Format(_("Use '%s' to get the list of defined services.")) % "zypper services" );
}

bool match_service( Zypper & zypper, const std::string & spec, repo::RepoInfoBase_Ptr & service_ptr, bool looseAuth, bool looseQuery  )
{
  ServiceList known = get_all_services( zypper );
  bool found = false;

  unsigned number = 0;	// service number start with 1
  for_( known_it, known.begin(), known.end() )
  {
    ++number;
    unsigned tmp = 0;
    safe_lexical_cast( spec, tmp ); // try to make an int out of the string

    try
    {
      // match by alias or number
      found = (*known_it)->alias() == spec || tmp == number;

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
            found = Url(spec).asString(urlview) == s_ptr->url().asString(urlview);
          else
          {
            RepoInfo_Ptr r_ptr = dynamic_pointer_cast<RepoInfo>(*known_it);
            if ( !r_ptr->baseUrlsEmpty() )
            {
              for_( urlit, r_ptr->baseUrlsBegin(), r_ptr->baseUrlsEnd() )
                if ( urlit->asString(urlview) == Url(spec).asString(urlview) )
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
            found = ( Url(spec) == s_ptr->url() );
          else
          {
            RepoInfo_Ptr r_ptr = dynamic_pointer_cast<RepoInfo>(*known_it);
            if ( !r_ptr->baseUrlsEmpty() )
            {
              found = find( r_ptr->baseUrlsBegin(), r_ptr->baseUrlsEnd(), Url(spec) ) != r_ptr->baseUrlsEnd();
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


