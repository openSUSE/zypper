#include "modify.h"
#include "Zypper.h"
#include "utils/flags/flagtypes.h"
#include "utils/messages.h"

#include "commands/commandhelpformatter.h"
#include "commands/conditions.h"
#include "commands/services/common.h"

#include "repos.h"


ModifyServiceCmd::ModifyServiceCmd( const std::vector<std::string> &commandAliases_r )
  : ZypperBaseCommand(
    commandAliases_r,
      {
        // translators: command synopsis; do not translate lowercase words
        _("modifyservice (ms) <OPTIONS> <ALIAS|#|URI>"),
        // translators: command synopsis; do not translate lowercase words
        str::Format( _("modifyservice (ms) <OPTIONS> <%1%>") ) % "--all|--remote|--local|--medium-type"
      },
      _("Modify specified service."),
      str::Format(_("Modify properties of services specified by alias, number, or URI, or by the '%1%' aggregate options.") ) % "--all, --remote, --local, --medium-type",
      ResetRepoManager
   )
{

}

std::vector<BaseCommandConditionPtr> ModifyServiceCmd::conditions() const
{
  return {
    std::make_shared<NeedsRootCondition>()
  };
}

zypp::ZyppFlags::CommandGroup ModifyServiceCmd::cmdOptions() const
{
  auto that = const_cast<ModifyServiceCmd *>(this);
  return {
    _("Advanced:"),
    {
      { "ar-to-enable", 'i', ZyppFlags::RequiredArgument | ZyppFlags::Repeatable, ZyppFlags::StringVectorType( &that->_arToEnable, "ALIAS"), _("Add a RIS service repository to enable.")},
      { "ar-to-disable", 'I', ZyppFlags::RequiredArgument | ZyppFlags::Repeatable, ZyppFlags::StringVectorType( &that->_arToDisable, "ALIAS"), _("Add a RIS service repository to disable.")},
      { "rr-to-enable", 'j', ZyppFlags::RequiredArgument | ZyppFlags::Repeatable, ZyppFlags::StringVectorType( &that->_rrToEnable, "ALIAS"), _("Remove a RIS service repository to enable.")},
      { "rr-to-disable", 'J', ZyppFlags::RequiredArgument | ZyppFlags::Repeatable, ZyppFlags::StringVectorType( &that->_rrToDisable, "ALIAS"), _("Remove a RIS service repository to disable.")},
      { "cl-to-enable", 'k', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that->_clearToEnable, ZyppFlags::StoreTrue), _("Clear the list of RIS repositories to enable.")},
      { "cl-to-disable", 'K', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that->_clearToDisable, ZyppFlags::StoreTrue), _("Clear the list of RIS repositories to disable.")},
      //some legacy options
      {"legacy-refresh", 'r', ZyppFlags::NoArgument | ZyppFlags::Hidden, ZyppFlags::TriBoolType( that->_commonProperties._enableAutoRefresh, ZyppFlags::StoreTrue ), "" },
      {"legacy-no-refresh", 'R', ZyppFlags::NoArgument | ZyppFlags::Hidden, ZyppFlags::TriBoolType( that->_commonProperties._enableAutoRefresh, ZyppFlags::StoreFalse), "" }
    }};
}

void ModifyServiceCmd::doReset()
{
  _arToEnable.clear();
  _arToDisable.clear();
  _rrToEnable.clear();
  _rrToDisable.clear();
  _clearToEnable  = false;
  _clearToDisable = false;
}

int ModifyServiceCmd::execute(Zypper &zypper, const std::vector<std::string> &positionalArgs_r)
{
  bool non_alias = _selectOptions._all || _selectOptions._local || _selectOptions._remote || (!_selectOptions._mediumTypes.empty());

  if ( positionalArgs_r.size() < 1 && !non_alias )
  {
    // translators: aggregate option is e.g. "--all". This message will be
    // followed by ms command help text which will explain it
    zypper.out().error(_("Alias or an aggregate option is required."));
    ERR << "No alias argument given." << endl;
    zypper.out().info( help() );
    return ZYPPER_EXIT_ERR_INVALID_ARGS;
  }
  // too many arguments
  if ( positionalArgs_r.size() > 1 || ( positionalArgs_r.size() > 0 && non_alias ) )
  {
    report_too_many_arguments( help() );
    return ZYPPER_EXIT_ERR_INVALID_ARGS;
  }

  if ( non_alias )
  {
    modifyServicesByOption( zypper );
  }
  else
  {
    repo::RepoInfoBase_Ptr srv;
    if ( match_service( zypper, positionalArgs_r[0], srv, false, false ) )
    {
      if ( dynamic_pointer_cast<ServiceInfo>(srv) )
        modifyService( zypper, srv->alias() );
      else {
        RepoProperties rProps;
        rProps.reset();
        modify_repo( zypper, srv->alias(), _commonProperties, rProps );
      }
    }
    else
    {
      zypper.out().error( str::Format(_("Service '%s' not found.")) % positionalArgs_r[0] );
      ERR << "Service " << positionalArgs_r[0] << " not found" << endl;
    }
  }
  return zypper.exitCode();
}


std::string ModifyServiceCmd::help()
{
  CommandHelpFormater formatter;

  formatter <<  ZypperBaseCommand::help();

  formatter
      .legacyOptionSection()
      .legacyOption( "-r", "-f" )
      .legacyOption( "-R", "-F" );

  return formatter;
}


int ModifyServiceCmd::modifyService( Zypper & zypper, const std::string & alias )
{
  // enable/disable repo
  const TriBool &enable = _commonProperties._enable;
  DBG << "enable = " << enable << endl;

  // autorefresh
  const TriBool &autoref = _commonProperties._enableAutoRefresh;
  DBG << "autoref = " << autoref << endl;

  try
  {
    RepoManager & manager = zypper.repoManager();
    ServiceInfo srv( manager.getService( alias ) );

    bool changed_enabled = false;
    bool changed_autoref = false;

    if ( !indeterminate(enable) )
    {
      if ( enable != srv.enabled() )
        changed_enabled = true;
      srv.setEnabled( enable );
    }

    if ( !indeterminate(autoref) )
    {
      if ( autoref != srv.autorefresh() )
        changed_autoref = true;
      srv.setAutorefresh( autoref );
    }

    const std::string &name = _commonProperties._name;
    if ( ! name.empty() )
      srv.setName( name );

    std::set<std::string> artoenable;
    std::set<std::string> artodisable;
    std::set<std::string> rrtoenable;
    std::set<std::string> rrtodisable;

    // RIS repos to enable
    if ( _clearToEnable )
    {
      rrtoenable.insert( srv.reposToEnableBegin(), srv.reposToEnableEnd() );
      srv.clearReposToEnable();
    }
    else
    {
      for_( rit, _arToEnable.begin(), _arToEnable.end() )
      {
        if ( !srv.repoToEnableFind( *rit ) )
        {
          srv.addRepoToEnable( *rit );
          artoenable.insert( *rit );
        }
      }

      for_( rit, _rrToEnable.begin(), _rrToEnable.end() )
      {
        if ( srv.repoToEnableFind( *rit ) )
        {
          srv.delRepoToEnable( *rit );
          rrtoenable.insert( *rit );
        }
      }
    }

    // RIS repos to disable
    if ( _clearToDisable )
    {
      rrtodisable.insert( srv.reposToDisableBegin(), srv.reposToDisableEnd() );
      srv.clearReposToDisable();
    }
    else
    {
      for_( rit, _arToDisable.begin(), _arToDisable.end() )
      {
        if ( !srv.repoToDisableFind( *rit ) )
        {
          srv.addRepoToDisable( *rit );
          artodisable.insert( *rit );
        }
      }

      for_( rit, _rrToDisable.begin(), _rrToDisable.end() )
      {
        if  (srv.repoToDisableFind( *rit ) )
        {
          srv.delRepoToDisable( *rit );
          rrtodisable.insert( *rit );
        }
      }
    }

    if ( changed_enabled
      || changed_autoref
      || !name.empty()
      || !artoenable.empty()
      || !artodisable.empty()
      || !rrtoenable.empty()
      || !rrtodisable.empty() )
    {
      manager.modifyService( alias, srv );

      if ( changed_enabled )
      {
        if ( srv.enabled() )
          zypper.out().info( str::Format(_("Service '%s' has been successfully enabled.")) % alias );
        else
          zypper.out().info( str::Format(_("Service '%s' has been successfully disabled.")) % alias );
      }

      if ( changed_autoref )
      {
        if ( srv.autorefresh() )
          zypper.out().info( str::Format(_("Autorefresh has been enabled for service '%s'.")) % alias );
        else
          zypper.out().info( str::Format(_("Autorefresh has been disabled for service '%s'.")) % alias );
      }

      if ( !name.empty() )
      {
        zypper.out().info( str::Format(_("Name of service '%s' has been set to '%s'.")) % alias % name );
      }

      if ( !artoenable.empty() )
      {
        zypper.out().info( str::Format(PL_("Repository '%s' has been added to enabled repositories of service '%s'",
					   "Repositories '%s' have been added to enabled repositories of service '%s'",
					   artoenable.size()))
	                   % str::join( artoenable.begin(), artoenable.end(), ", " ) % alias );
      }
      if ( !artodisable.empty() )
      {
        zypper.out().info( str::Format(PL_("Repository '%s' has been added to disabled repositories of service '%s'",
					   "Repositories '%s' have been added to disabled repositories of service '%s'",
					   artodisable.size()))
			   % str::join( artodisable.begin(), artodisable.end(), ", " ) % alias );
      }
      if ( !rrtoenable.empty() )
      {
        zypper.out().info( str::Format(PL_("Repository '%s' has been removed from enabled repositories of service '%s'",
					   "Repositories '%s' have been removed from enabled repositories of service '%s'",
					   rrtoenable.size()))
			   % str::join( rrtoenable.begin(), rrtoenable.end(), ", " ) % alias );
      }
      if ( !rrtodisable.empty())
      {
        zypper.out().info( str::Format(PL_("Repository '%s' has been removed from disabled repositories of service '%s'",
					   "Repositories '%s' have been removed from disabled repositories of service '%s'",
					   rrtodisable.size()))
			   % str::join( rrtodisable.begin(), rrtodisable.end(), ", " ) % alias );
      }
    }
    else
    {
      MIL << "Nothing to modify in '" << alias << "':" << srv << endl;
      zypper.out().info( str::Format(_("Nothing to change for service '%s'.")) % alias );
    }
  }
  catch ( const Exception & ex )
  {
    ERR << "Error while modifying the service:" << ex.asUserString() << endl;
    zypper.out().error( ex, _("Error while modifying the service:"),
			str::Format(_("Leaving service %s unchanged.")) % alias );
    return ZYPPER_EXIT_ERR_ZYPP;
  }

  return ZYPPER_EXIT_OK;
}

// ---------------------------------------------------------------------------

void ModifyServiceCmd::modifyServicesByOption( Zypper & zypper )
{
  ServiceList known = get_all_services( zypper );
  std::set<std::string> repos_to_modify;
  std::set<std::string> services_to_modify;

  if ( _selectOptions._all )
  {
    for_( it, known.begin(), known.end() )
    {
      ServiceInfo_Ptr sptr = dynamic_pointer_cast<ServiceInfo>(*it);
      if ( sptr )
        modifyService( zypper, sptr->alias() );
      else {
        RepoProperties rProps;
        rProps.reset();
        modify_repo( zypper, (*it)->alias(), _commonProperties, rProps );
      }
    }
    return;
  }

  bool local = _selectOptions._local;
  bool remote = _selectOptions._remote;

  std::set<std::string> schemes( _selectOptions._mediumTypes.begin(), _selectOptions._mediumTypes.end() );

  for_( it, known.begin(), known.end() )
  {
    ServiceInfo_Ptr sptr = dynamic_pointer_cast<ServiceInfo>(*it);
    Url url;
    if ( sptr )
      url = sptr->url();
    else
    {
      RepoInfo_Ptr rptr = dynamic_pointer_cast<RepoInfo>(*it);
      if ( !rptr->baseUrlsEmpty() )
        url = rptr->url();
    }

    if ( url.isValid() )
    {
      bool modify = false;
      if ( local  && ! url.schemeIsDownloading() )
        modify = true;

      if ( !modify && remote && url.schemeIsDownloading() )
        modify = true;

      if ( !modify && schemes.find(url.getScheme()) != schemes.end() )
        modify = true;

      if ( modify )
      {
        std::string alias = (*it)->alias();
        if ( sptr )
          services_to_modify.insert( alias );
        else
          repos_to_modify.insert( alias );
      }
    }
    else
      WAR << "got invalid url: " << url.asString() << endl;
  }

  for_( it, services_to_modify.begin(), services_to_modify.end() )
    modifyService( zypper, *it );

  RepoProperties rProps;
  rProps.reset();
  for_( it, repos_to_modify.begin(), repos_to_modify.end() )
    modify_repo( zypper, *it, _commonProperties, rProps );
}
