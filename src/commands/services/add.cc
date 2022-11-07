/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "add.h"
#include "main.h"
#include "repos.h"
#include "Zypper.h"
#include "commands/conditions.h"
#include "utils/messages.h"
#include "utils/misc.h"
#include "utils/flags/flagtypes.h"

#include <zypp/repo/RepoException.h>
#include <zypp/Url.h>

using namespace zypp;

namespace {
ZyppFlags::Value CheckIsServiceTypeVal( bool &target )
{
  return ZyppFlags::Value (
        []() -> boost::optional<std::string>{
          return  boost::optional<std::string>();
        },

        [ &target ]( const ZyppFlags::CommandOption &opt, const boost::optional<std::string> &in ) {
          if (!in)
            ZYPP_THROW(ZyppFlags::MissingArgumentException(opt.name));

          target = false;
          try {
            repo::ServiceType t ( *in );
            target = true;
          } catch ( const repo::RepoUnknownTypeException & e )
          { }
        }
  );
}

void add_service( Zypper & zypper, const ServiceInfo & service )
{
  RepoManager manager( zypper.config().rm_options );

  try {
    ServiceInfo exists { manager.getService( service.alias() ) };
    if ( exists != zypp::ServiceInfo::noService ) {
      if ( service.url() == exists.url() ) {
        // bsc#1203715: Support (re)adding a service with the same URL.
        MIL << "Service '" << service.alias() << "' exists with same URL '" << service.url() << "'" << endl;
        zypper.out().info( str::Format(_("Service '%1%' with URL '%2%' already exists. Just updating the settings.")) % service.alias() % service.url() );
        manager.modifyService( service.alias(), service );
      }
      else {
        ERR << "Service '" << service.alias() << "' exists with different URL '" << service.url() << "'" << endl;
        zypper.out().error( str::Format(_("Service '%1%' already exists but uses a different URL '%2%'. Please use another alias.")) % service.alias() % service.url() );
        zypper.setExitCode( ZYPPER_EXIT_ERR_ZYPP );
        return;
      }
    }
    else {
      manager.addService( service );
    }
  }
  catch ( const Exception & e )
  {
    ZYPP_CAUGHT( e );
    zypper.out().error( str::Format(_("Error occurred while adding service '%s'.")) % service.alias() );
    zypper.setExitCode( ZYPPER_EXIT_ERR_ZYPP );
    return;
  }

  MIL << "Service '" << service.alias() << "' has been added." << endl;
  std::ostringstream s;
  s << str::Format(_("Service '%s' has been successfully added.")) % service.asUserString() << endl;
  s << endl;

  {
    PropertyTable p;
    // translators: property name; short; used like "Name: value"
    p.add( _("URI"),		service.url() );
    // translators: property name; short; used like "Name: value"
    p.add( _("Enabled"),	service.enabled() );
    // translators: property name; short; used like "Name: value"
    p.add( _("Autorefresh"),	service.autorefresh() );
    s << p;
  }
  zypper.out().info( s.str() );
}

// ---------------------------------------------------------------------------

void add_service_by_url( Zypper & zypper,
                         const Url & url,
                         const std::string & alias,
                         const RepoServiceCommonOptions &props )
{
  MIL << "going to add service by url (alias=" << alias << ", url=" << url << ")" << endl;

  ServiceInfo service;

  service.setAlias( alias.empty() ? timestamp() : alias );
  if ( !props._name.empty() )
    service.setName( props._name );
  service.setUrl( url );

  zypper.out().info( str::Str() << "Adding service '" << alias << "'...");

  service.setEnabled( indeterminate( props._enable ) ? true : bool(props._enable) );
  service.setAutorefresh( indeterminate( props._enableAutoRefresh ) ? true : bool(props._enableAutoRefresh) );

  add_service( zypper, service );
}


}

AddServiceCmd::AddServiceCmd(std::vector<std::string> &&commandAliases_r ) :
  ZypperBaseCommand(
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate lowercase words
    _("addservice (as) [OPTIONS] <URI> <ALIAS>"),
    _("Add a new service."),
    _("Add a repository index service to the system."),
    ResetRepoManager
  )
{

}

zypp::ZyppFlags::CommandGroup AddServiceCmd::cmdOptions() const
{
  return {{
      { "type",
        't',
        ZyppFlags::RequiredArgument | ZyppFlags::Deprecated,
        ZyppFlags::WarnOptionVal( Zypper::instance().out(), legacyCLIStr( "type", "", LegacyCLIMsgType::Ignored ), Out::NORMAL, CheckIsServiceTypeVal(const_cast<bool&>(_isService))),
        _("The type of service is always autodetected. This option is ignored.")
      }
    }};
}

void AddServiceCmd::doReset()
{
  _isService = true;
}

int AddServiceCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs_r )
{
  // too many arguments
  if ( positionalArgs_r.size() > 2 )
  {
    report_too_many_arguments( help() );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  // missing arguments
  if ( positionalArgs_r.size() < 2 )
  {
    report_required_arg_missing( zypper.out(), help() );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  Url url = make_url( positionalArgs_r[0] );
  if ( !url.isValid() ) {
    return( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  if ( _isService )
    add_service_by_url( zypper, url, positionalArgs_r[1], _commonProps );
  else {
    //legacy behaviour
    add_repo_by_url( zypper, url, positionalArgs_r[1], _commonProps, RepoProperties(), false);
  }

  return zypper.exitCode();
}


std::vector<BaseCommandConditionPtr> AddServiceCmd::conditions() const
{
  return {
    std::make_shared<NeedsRootCondition>()
  };
}
