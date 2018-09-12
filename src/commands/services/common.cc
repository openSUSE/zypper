/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include "common.h"

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
