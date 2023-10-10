/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include "info.h"
#include "utils/flags/flagtypes.h"
#include "utils/messages.h"
#include "commands/commandhelpformatter.h"
#include "commands/commonflags.h"


InfoCmd::InfoCmd( std::vector<std::string> &&commandAliases_r, InfoCmd::Mode cmdMode_r ) :
  ZypperBaseCommand (
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate lowercase words
    _("info (if) [OPTIONS] <NAME> ..."),
    // translators: command summary: info, if
    _("Show full information for specified packages."),
    // translators: command description
    _("Show detailed information for specified packages. By default the packages which match exactly the given names are shown. To get also packages partially matching use option '--match-substrings' or use wildcards (*?) in name."),
    DefaultSetup
  ),
  _cmdMode ( cmdMode_r )
{
  _initRepoOpts.setCompatibilityMode( cmdMode_r != Mode::Default ? CompatModeBits::EnableRugOpt : CompatModeBits::EnableNewOpt );
}


std::string InfoCmd::help()
{
  CommandHelpFormater myHelp;
  myHelp << ZypperBaseCommand::help();
  switch ( _cmdMode ) {
    case Mode::RugPatchInfo:
      myHelp.descriptionAliasCmd( "zypper info -t patch" );
      break;
    case Mode::RugPatternInfo:
      myHelp.descriptionAliasCmd( "zypper info -t pattern" );
      break;
    case Mode::RugProductInfo:
      myHelp.descriptionAliasCmd( "zypper info -t product" );
      break;
    default:
    case Mode::Default:
      break;
  }
  return myHelp;
}

zypp::ZyppFlags::CommandGroup InfoCmd::cmdOptions() const
{
  if ( _cmdMode != Mode::Default )
    return {};

  auto &that = *const_cast<InfoCmd *>( this );
  return {{
      { "match-substrings", 's',  ZyppFlags::NoArgument, ZyppFlags::BoolType( &that._options._matchSubstrings, ZyppFlags::StoreTrue, _options._matchSubstrings ),
            // translators: -s, --match-substrings
            _("Print information for packages partially matching name.")
      },
      CommonFlags::resKindSetFlag( that._options._kinds ),
      { "provides", '\0', ZyppFlags::NoArgument, ZyppFlags::BitFieldType( that._options._flags, InfoBits::ShowProvides, ZyppFlags::StoreTrue),
            // translators: --provides
            _("Show provides.")
      },
      { "requires", '\0', ZyppFlags::NoArgument, ZyppFlags::BitFieldType( that._options._flags, InfoBits::ShowRequires, ZyppFlags::StoreTrue),
            // translators: --requires
            _("Show requires and prerequires.")
      },
      { "conflicts", '\0', ZyppFlags::NoArgument, ZyppFlags::BitFieldType( that._options._flags, InfoBits::ShowConflicts, ZyppFlags::StoreTrue),
            // translators: --conflicts
            _("Show conflicts.")
      },
      { "obsoletes", '\0', ZyppFlags::NoArgument, ZyppFlags::BitFieldType( that._options._flags, InfoBits::ShowObsoletes, ZyppFlags::StoreTrue),
            // translators: --obsoletes
            _("Show obsoletes.")
      },
      { "recommends", '\0', ZyppFlags::NoArgument, ZyppFlags::BitFieldType( that._options._flags, InfoBits::ShowRecommends, ZyppFlags::StoreTrue),
            // translators: --recommends
            _("Show recommends.")
      },
      { "supplements", '\0', ZyppFlags::NoArgument, ZyppFlags::BitFieldType( that._options._flags, InfoBits::ShowSupplements, ZyppFlags::StoreTrue),
            // translators: --supplements
            _("Show supplements.")
      },
      { "suggests", '\0', ZyppFlags::NoArgument, ZyppFlags::BitFieldType( that._options._flags, InfoBits::ShowSuggests, ZyppFlags::StoreTrue),
            // translators: --suggests
            _("Show suggests.")
      },
  }};
}

void InfoCmd::doReset()
{
  _options = PrintInfoOptions();
}

int InfoCmd::execute( Zypper &zypper, const std::vector<std::string> &positionalArgs_r )
{
  if ( positionalArgs_r.size() < 1 )
  {
    zypper.out().error(_("Required argument missing.") );
    ERR << "Required argument missing." << endl;
    std::ostringstream s;
    s << _("Usage") << ':' << endl;
    s << help();
    zypper.out().info( s.str() );
    return ( ZYPPER_EXIT_ERR_INVALID_ARGS );
  }

  //for aliased modes we override the _kinds in the option object
  switch ( _cmdMode ) {
    case Mode::RugPatchInfo:
      _options._kinds = { ResKind::patch };
      break;
    case Mode::RugPatternInfo:
      _options._kinds = { ResKind::pattern };
      break;
    case Mode::RugProductInfo:
      _options._kinds = { ResKind::product };
      break;
    case Mode::Default:
      break;
  }
  bool all_caps_exist = false;
  printInfo( zypper, positionalArgs_r, _options, all_caps_exist );
  if (all_caps_exist)
    return ZYPPER_EXIT_OK;
  return ZYPPER_EXIT_INF_CAP_NOT_FOUND;
}

std::string InfoCmd::summary() const
{
  switch ( _cmdMode ) {
    case Mode::RugPatchInfo:
      // translators: command summary: patch-info
      return _("Show full information for specified patches.");
    case Mode::RugPatternInfo:
      // translators: command summary: pattern-info
      return _("Show full information for specified patterns.");
    case Mode::RugProductInfo:
      // translators: command summary: product-info
      return _("Show full information for specified products.");
    case Mode::Default:
      break;
  }

  return ZypperBaseCommand::summary();
}

std::vector<std::string> InfoCmd::synopsis() const
{
  switch ( _cmdMode ) {
    case Mode::RugPatchInfo:
      // translators: command synopsis; do not translate lowercase words
      return { _("patch-info <PATCHNAME> ...") };
    case Mode::RugPatternInfo:
      // translators: command synopsis; do not translate lowercase words
      return { _("pattern-info <PATTERN_NAME> ...") };
    case Mode::RugProductInfo:
      // translators: command synopsis; do not translate lowercase words
      return { _("product-info <PRODUCT_NAME> ...") };
    case Mode::Default:
      break;
  }
  return ZypperBaseCommand::synopsis();
}

std::string InfoCmd::description() const
{
  switch ( _cmdMode ) {
    case Mode::RugPatchInfo:
      // translators: command description
      return _("Show detailed information for patches.");
    case Mode::RugPatternInfo:
      // translators: command description
      return _("Show detailed information for patterns.");
    case Mode::RugProductInfo:
      // translators: command description
      return _("Show detailed information for products.");
    case Mode::Default:
      break;
  }
  return ZypperBaseCommand::description();
}
