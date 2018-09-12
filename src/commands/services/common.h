/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_SERVICES_COMMON_H_INCLUDED
#define ZYPPER_COMMANDS_SERVICES_COMMON_H_INCLUDED

#include "repos.h"

#include <zypp/repo/RepoInfoBase.h>

#include <list>

namespace zypp { using repo::RepoInfoBase_Ptr; }
typedef std::list<RepoInfoBase_Ptr> ServiceList;

ServiceList get_all_services( Zypper & zypper );


#endif
