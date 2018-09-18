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

typedef std::list<zypp::repo::RepoInfoBase_Ptr> ServiceList;

ServiceList get_all_services( Zypper & zypper );

bool match_service( Zypper & zypper, std::string str, repo::RepoInfoBase_Ptr & service_ptr );
bool refresh_service(Zypper & zypper, const ServiceInfo & service, RepoManager::RefreshServiceFlags flags_r = RepoManager::RefreshServiceFlags() );


#endif
