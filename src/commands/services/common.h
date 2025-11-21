/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_SERVICES_COMMON_H_INCLUDED
#define ZYPPER_COMMANDS_SERVICES_COMMON_H_INCLUDED

#include "Zypper.h"

#include <zypp/RepoInfo.h>
#include <zypp/repo/RepoInfoBase.h>
#include <zypp/RepoManager.h>

#include <list>

struct RepoCollector
{
  bool collect( const zypp::RepoInfo & repo )
  {
    repos.push_back( repo );
    return true;
  }
  RepoInfoList repos;
};

/** A RepoInfoBase_Ptr list because service lists may  include non-service repos */
typedef std::list<zypp::repo::RepoInfoBase_Ptr> ServiceList;

/** Get all services and non-service repos from RepoManager. */
ServiceList get_all_services( Zypper & zypper );

/**
 * Try to find ServiceInfo or RepoInfo counterparts among known services by alias, number,
 * or URI, based on the list of strings given as the iterator range \a begin and
 * \a end. Matching objects will be added to \a services (as RepoInfoBase_Ptr) and those
 * with no match will be added to \a not_found. (e.g evaluation of positionals args)
 *
 * \note: explicit instantiations required for other translation units
 */
template<typename TStringIter>
void get_services( Zypper & zypper, TStringIter && begin, TStringIter && end, ServiceList & services, std::list<std::string> & not_found )
{
  void get_services_helper( Zypper & zypper, const std::string & spec, ServiceList & services, std::list<std::string> & not_found );
  for ( const std::string & spec : makeIterable(std::forward<TStringIter>(begin),std::forward<TStringIter>(end)) ) {
    get_services_helper( zypper, spec, services, not_found );
  }
}

/** Say "Service %s not found" for all strings in \a not_found list. */
void report_unknown_services( Out & out, const std::list<std::string> & not_found );

/** Find a service by spec in all services and non-service repos. */
bool match_service( Zypper & zypper, const std::string & spec, repo::RepoInfoBase_Ptr & service_ptr, bool looseAuth, bool looseQuery );

bool refresh_service(Zypper & zypper, const ServiceInfo & service, RepoManager::RefreshServiceFlags flags_r = RepoManager::RefreshServiceFlags() );
void remove_service( Zypper & zypper, const ServiceInfo & service );


#endif
