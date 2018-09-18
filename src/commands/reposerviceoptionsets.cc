#include "reposerviceoptionsets.h"
#include "utils/flags/flagtypes.h"
#include "main.h"

#include "utils/getopt.h"
#include "Zypper.h"

using namespace zypp;

std::vector<ZyppFlags::CommandGroup> RepoServiceCommonOptions::options()
{
  return {
    {
      {
        {"name",   'n', ZyppFlags::RequiredArgument, ZyppFlags::StringType( &_name, boost::optional<const char *>(), "NAME"), _("Set a descriptive name for the service.") },
        {"enable", 'e', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( _enable, ZyppFlags::StoreTrue, TriBool( true ) ), _("Enable a disabled service.") },
        {"disable", 'd', ZyppFlags::NoArgument, ZyppFlags::TriBoolType(_enable, ZyppFlags::StoreFalse, TriBool( false ) ), _("Disable the service (but don't remove it).")},
        {"refresh", 'f', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( _enableAutoRefresh, ZyppFlags::StoreTrue, TriBool( true ) ), _("Enable auto-refresh of the service.") },
        {"no-refresh", 'F', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( _enableAutoRefresh, ZyppFlags::StoreFalse, TriBool( false ) ), _("Disable auto-refresh of the service.") }
      }
    }
  };
}

void RepoServiceCommonOptions::reset()
{
  _name.clear();
  _enable = indeterminate;
  _enableAutoRefresh = indeterminate;
}

void RepoServiceCommonOptions::fillFromCopts( Zypper &zypper )
{
  reset();
  parsed_opts::const_iterator it = zypper.cOpts().find("name");
  if ( it != zypper.cOpts().end() )
    _name = it->second.front();

  _enable = get_boolean_option( zypper, "enable", "disable" );
  _enableAutoRefresh = get_boolean_option( zypper, "refresh", "no-refresh" );
}
