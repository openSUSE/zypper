/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "refresh.h"
#include "main.h"
#include "Zypper.h"
#include "repos.h"

#include "common.h"
#include "utils/flags/flagtypes.h"
#include "commands/repos/refresh.h"
#include "commands/conditions.h"

#include <zypp/repo/RepoInfoBase.h>
#include <zypp/base/Iterator.h>
#include <zypp/media/MediaException.h>

using namespace zypp;

/**
 * Try to find ServiceInfo or RepoInfo counterparts among known services by alias, number,
 * or URI, based on the list of strings given as the iterator range \a begin and
 * \a end. Matching objects will be added to \a services (as RepoInfoBase_Ptr) and those
 * with no match will be added to \a not_found.
 */
template<typename T>
void get_services( Zypper & zypper, const T & begin, const T & end,
                   ServiceList & services, std::list<std::string> & not_found )
{
  for_( it, begin, end )
  {
    repo::RepoInfoBase_Ptr service;

    if ( !match_service( zypper, *it, service, false, false ) )
    {
      not_found.push_back( *it );
      continue;
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
}

/**
 * Say "Service %s not found" for all strings in \a not_found list.
 */
static void report_unknown_services( Out & out, std::list<std::string> not_found )
{
  for_( it, not_found.begin(), not_found.end() )
    out.error( str::Format(_("Service '%s' not found by its alias, number, or URI.")) % *it );

  if ( !not_found.empty() )
    out.info( str::Format(_("Use '%s' to get the list of defined services.")) % "zypper repos" );
}

RefreshServicesCmd::RefreshServicesCmd(std::vector<std::string> &&commandAliases_r ) :
  ZypperBaseCommand(
    std::move( commandAliases_r ),
    _("refresh-services (refs) [OPTIONS]"),
    _("Refresh all services."),
    _("Refresh defined repository index services."))
{

}

std::vector<BaseCommandConditionPtr> RefreshServicesCmd::conditions() const
{
  return {
    std::make_shared<NeedsRootCondition>()
  };
}

ZyppFlags::CommandGroup RefreshServicesCmd::cmdOptions() const
{
  auto that  = const_cast<RefreshServicesCmd *>(this);
  return {{
    { "force", 'f', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that->_force, ZyppFlags::StoreTrue, _force), _("Force a complete refresh.") },
    { "with-repos", 'r', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that->_withRepos, ZyppFlags::StoreTrue, _withRepos), _("Refresh also the service repositories.") },
    { "restore-status", 'R', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that->_restoreStatus, ZyppFlags::StoreTrue, _restoreStatus), _("Also restore service repositories enabled/disabled state.") }
  }};
}

void RefreshServicesCmd::doReset()
{
  _force = false;
  _withRepos = false;
  _restoreStatus = false;
}

int RefreshServicesCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs )
{
  return refreshServices( zypper, positionalArgs );
}

int RefreshServicesCmd::refreshServices(Zypper &zypper, const std::vector<std::string> &services_r )
{
  MIL << "going to refresh services" << endl;

  ServiceList services = get_all_services( zypper );

  // get the list of repos specified on the command line ...
  ServiceList specified;
  std::list<std::string> not_found;
  // ...as command arguments
  get_services( zypper, services_r.begin(), services_r.end(), specified, not_found );
  report_unknown_services( zypper.out(), not_found ) ;

  unsigned error_count = 0;
  unsigned enabled_service_count = services.size();

  if ( !specified.empty() || not_found.empty() )
  {
    unsigned number = 0;
    for_( sit, services.begin(), services.end() )
    {
      ++number;
      repo::RepoInfoBase_Ptr service_ptr( *sit );

      // skip services not specified on the command line
      if ( !specified.empty() )
      {
        bool found = false;
        for_( it, specified.begin(), specified.end() )
          if ( (*it)->alias() == service_ptr->alias() )
          {
            found = true;
            break;
          }

        if ( !found )
        {
          DBG << service_ptr->alias() << "(#" << number << ") not specified," << " skipping." << endl;
          --enabled_service_count;
          continue;
        }
      }

      // skip disabled services
      if ( !service_ptr->enabled() )
      {
        DBG << "skipping disabled service '" << service_ptr->alias() << "'" << endl;

        std::string msg = str::Format(_("Skipping disabled service '%s'")) % service_ptr->asUserString();
        if ( specified.empty() )
          zypper.out().info( msg, Out::HIGH );
        else
          zypper.out().error( msg );

        --enabled_service_count;
        continue;
      }

      // do the refresh
      bool error = false;
      ServiceInfo_Ptr s = dynamic_pointer_cast<ServiceInfo>(service_ptr);
      if ( s )
      {
        RepoManager::RefreshServiceOptions opts;
        if ( _restoreStatus )
          opts |= RepoManager::RefreshService_restoreStatus;
        if ( _force )
          opts |= RepoManager::RefreshService_forceRefresh;

        error = refresh_service( zypper, *s, opts );

        // refresh also service's repos
        if ( _withRepos )
        {
          RepoCollector collector;
          RepoManager & rm = zypper.repoManager();
          rm.getRepositoriesInService( s->alias(),
                                       make_function_output_iterator( bind( &RepoCollector::collect, &collector, _1 ) ) );
          for_( repoit, collector.repos.begin(), collector.repos.end() )
              RefreshRepoCmd::refreshRepository( zypper, *repoit, _force ? RefreshRepoCmd::Force : RefreshRepoCmd::Default );
        }
      }
      else
      {
        if ( !_withRepos )
        {
          DBG << "Skipping non-index service '" << service_ptr->asUserString() << "' because '--no-repos' is used.";
          continue;
        }
        error = RefreshRepoCmd::refreshRepository( zypper, *dynamic_pointer_cast<RepoInfo>(service_ptr), _force ? RefreshRepoCmd::Force : RefreshRepoCmd::Default );
      }

      if ( error )
      {
        ERR << "Skipping service '" << service_ptr->alias() << "' because of the above error." << endl;
        zypper.out().error( str::Format(_("Skipping service '%s' because of the above error.")) % service_ptr->asUserString().c_str() );
        ++error_count;
      }
    }
  }
  else
    enabled_service_count = 0;

  // print the result message
  if ( enabled_service_count == 0 )
  {
    std::string hint = str::form(_("Use '%s' or '%s' commands to add or enable services."),
                                 "zypper addservice", "zypper modifyservice" );
    if ( !specified.empty() || !not_found.empty() )
      zypper.out().error(_("Specified services are not enabled or defined."), hint);
    else
      zypper.out().error(_("There are no enabled services defined."), hint);
  }
  else if ( error_count == enabled_service_count )
  {
    zypper.out().error(_("Could not refresh the services because of errors.") );
    return ZYPPER_EXIT_ERR_ZYPP;
  }
  else if ( error_count )
  {
    zypper.out().error(_("Some of the services have not been refreshed because of an error.") );
    return ZYPPER_EXIT_ERR_ZYPP;
  }
  else if ( !specified.empty() )
    zypper.out().info(_("Specified services have been refreshed.") );
  else
    zypper.out().info(_("All services have been refreshed.") );

  MIL << "DONE";
  return ZYPPER_EXIT_OK;
}


int RefreshServicesCmd::systemSetup(Zypper &zypper)
{
  int code = defaultSystemSetup( zypper, InitTarget );
  if ( code != ZYPPER_EXIT_OK )
    return code;

  zypper.configNoConst().rm_options.servicesTargetDistro =
      zyppApi()->target()->targetDistribution();

  return defaultSystemSetup( zypper, ResetRepoManager );
}

void RefreshServicesCmd::setRestoreStatus(bool restoreStatus)
{
  _restoreStatus = restoreStatus;
}

void RefreshServicesCmd::setWithRepos(bool withRepos)
{
  _withRepos = withRepos;
}

void RefreshServicesCmd::setForce(bool force)
{
  _force = force;
}
