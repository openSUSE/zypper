/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "listupdates.h"
#include "commonflags.h"
#include "utils/messages.h"
#include "src/update.h"

ListUpdatesCmd::ListUpdatesCmd( std::vector<std::string> &&commandAliases_r) :
  ZypperBaseCommand (
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate lowercase words
    _("list-updates (lu) [OPTIONS]"),
    // translators: command summary: list-updates, lu
    _("List available updates."),
    // translators: command description
    _("List all available updates."),
    ResetRepoManager
  )
{
  _initReposOpts.setCompatibilityMode( CompatModeBits::EnableNewOpt | CompatModeBits::EnableRugOpt );
}

zypp::ZyppFlags::CommandGroup ListUpdatesCmd::cmdOptions() const
{
  auto &that = *const_cast<ListUpdatesCmd *>(this);
  return {{
    CommonFlags::resKindSetFlag( that._kinds ),
    CommonFlags::bestEffortUpdateFlag( that._bestEffort ),
    { "all", 'a', ZyppFlags::NoArgument, ZyppFlags::BoolType( &that._all, ZyppFlags::StoreTrue, _all ),
          // translators: -a, --all
          _("List all packages for which newer versions are available, regardless whether they are installable or not.")
    },
    { "filter-version-change", '\0', ZyppFlags::RequiredArgument,
      ZyppFlags::Value(
        []() -> boost::optional<std::string> { return std::string("none"); },
        [&that]( const ZyppFlags::CommandOption &opt, const boost::optional<std::string> &in ) {
          if ( !in )
            ZYPP_THROW( ZyppFlags::MissingArgumentException( opt.name ) );
          if ( *in == "none" )         that._vcFilter = VCF_None;
          else if ( *in == "rebuild" ) that._vcFilter = VCF_Rebuild;
          else if ( *in == "package" ) that._vcFilter = VCF_Package;
          else
            ZYPP_THROW( ZyppFlags::InvalidValueException( opt.name, *in,
              str::Format(_("Valid values: %1%")) % "none, rebuild, package" ) );
        },
        _("LEVEL")
      ),
      // translators: --filter-version-change
      _("Filter updates by version change significance. LEVEL: 'none' (default, show all), 'rebuild' (hide rebuild-only changes), 'package' (hide packaging-only changes, i.e. same upstream version).")
    }
  }};
}

void ListUpdatesCmd::doReset()
{
  _kinds.clear();
  _all = false;
  _bestEffort = false;
  _vcFilter = VCF_None;
}

int ListUpdatesCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs_r )
{
  // too many arguments
  if ( positionalArgs_r.size() > 0 ) {
    report_too_many_arguments( help() );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  if ( _kinds.empty() )
    _kinds.insert( ResKind::package );

  int code = defaultSystemSetup( zypper, InitTarget | InitRepos | LoadResolvables | Resolve );
  if ( code != ZYPPER_EXIT_OK )
    return code;

  list_updates( zypper, _kinds, _bestEffort, _all, PatchSelector(), _vcFilter );
  return zypper.exitCode();
}
