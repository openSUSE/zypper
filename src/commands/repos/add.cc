/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "add.h"
#include "repos.h"

#include "commands/conditions.h"
#include "utils/flags/flagtypes.h"
#include "utils/messages.h"
#include "utils/misc.h"
#include "Zypper.h"

#include <zypp/repo/RepoException.h>

using namespace zypp;

AddRepoCmd::AddRepoCmd(const std::vector<std::string> &commandAliases_r) :
  ZypperBaseCommand(
    commandAliases_r,
    std::vector<std::string>{ _("addrepo (ar) [OPTIONS] <URI> <ALIAS>"), _("addrepo (ar) [OPTIONS] <FILE.repo>") },
    _("Add a new repository."),
    _("Add a repository to the system. The repository can be specified by its URI or can be read from specified .repo file (even remote)."),
    ResetRepoManager )
{ }

std::vector<BaseCommandConditionPtr> AddRepoCmd::conditions() const
{
  return {
    std::make_shared<NeedsRootCondition>()
  };
}

zypp::ZyppFlags::CommandGroup AddRepoCmd::cmdOptions() const
{
  auto that = const_cast<AddRepoCmd *>(this);
  return {{
    { "repo", 'r', ZyppFlags::RequiredArgument, ZyppFlags::StringType( &that->_repoFile, boost::optional<const char *>(), ARG_FILE_repo), _("Just another means to specify a .repo file to read.") },
    { "check", 'c', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that->_enableCheck, ZyppFlags::StoreTrue ), _("Probe URI.") },
    { "no-check", 'C', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that->_disableCheck, ZyppFlags::StoreTrue ), _("Don't probe URI, probe later during refresh.") },
    { "type", 't',
            ZyppFlags::RequiredArgument | ZyppFlags::Deprecated,
            ZyppFlags::WarnOptionVal( Zypper::instance().out(), legacyCLIStr( "type", "", LegacyCLIMsgType::Ignored ), Out::NORMAL, boost::optional<ZyppFlags::Value>() ),
            _("The repository type is always autodetected. This option is ignored.") }
  }};
}

void AddRepoCmd::doReset()
{
  _repoFile.clear();
  _enableCheck  = false;
  _disableCheck = false;
}

int AddRepoCmd::execute(Zypper &zypp_r, const std::vector<std::string> &positionalArgs_r)
{

  // too many arguments
  if ( positionalArgs_r.size() > 2 )
  {
    report_too_many_arguments( zypp_r.out(), help() );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  if ( _enableCheck && _disableCheck )
  {
    zypp_r.out().warning(str::form(
      _("Cannot use %s together with %s. Using the %s setting."),
      "--check", "--no-check", "zypp.conf")
        ,Out::QUIET );
  }

  try
  {
    // add repository specified in .repo file
    if ( ! _repoFile.empty() )
    {
      add_repo_from_file( zypp_r, _repoFile, _commonProperties, _repoProperties, _disableCheck );
      return zypp_r.exitCode();
    }

    switch ( positionalArgs_r.size() )
    {
    // display help message if insufficient info was given
    case 0:
        report_too_few_arguments( zypp_r.out(), help() );
        return( ZYPPER_EXIT_ERR_INVALID_ARGS );
    case 1:
      if( !isRepoFile( positionalArgs_r[0] ) )
      {
        zypp_r.out().error(_("If only one argument is used, it must be a URI pointing to a .repo file."));
        ERR << "Not a repo file." << endl;
        zypp_r.out().info( help() );
        return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
      }
      else
      {
        add_repo_from_file( zypp_r, positionalArgs_r[0], _commonProperties, _repoProperties, _disableCheck );
        break;
      }
    case 2:
      Url url;
      if ( positionalArgs_r[0].find("obs") == 0 )
        url = make_obs_url( positionalArgs_r[0], zypp_r.config().obs_baseUrl, zypp_r.config().obs_platform );
      else
        url = make_url( positionalArgs_r[0] );
      if ( !url.isValid() )
      {
        return (ZYPPER_EXIT_ERR_INVALID_ARGS);
      }

      if ( _enableCheck )
        zypp_r.globalOptsNoConst().rm_options.probe = true;
      else if ( _disableCheck )
        zypp_r.globalOptsNoConst().rm_options.probe = false;

      // load gpg keys
      int code = defaultSystemSetup( zypp_r, InitTarget  );
      if ( code != ZYPPER_EXIT_OK )
        return code;

      add_repo_by_url( zypp_r, url, positionalArgs_r[1]/*alias*/, _commonProperties, _repoProperties, _disableCheck );
    }
  }
  catch ( const repo::RepoUnknownTypeException & e )
  {
    ZYPP_CAUGHT( e );
    zypp_r.out().error( e, _("Specified type is not a valid repository type:"),
                 str::form( _("See '%s' or '%s' to get a list of known repository types."),
                            "zypper help addrepo", "man zypper" ) );
    return ZYPPER_EXIT_ERR_INVALID_ARGS;
  }


  return zypp_r.exitCode();;
}
