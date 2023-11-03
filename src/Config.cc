/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

extern "C"
{
  #include <libintl.h>
}
#include <iostream>

#include <zypp/base/Logger.h>
#include <zypp/base/Measure.h>
#include <zypp/base/String.h>
#include <zypp/base/Exception.h>
#include <zypp/ZConfig.h>

#include "utils/messages.h"
#include "utils/Augeas.h"
#include "utils/flags/flagtypes.h"
#include "output/OutNormal.h"
#include "output/OutXML.h"
#include "Config.h"
#include "global-settings.h"
#include "Zypper.h"

// redefine _ gettext macro defined by ZYpp
#ifdef _
#undef _
#endif
#define _(MSG) ::gettext(MSG)

using namespace zypp;
using std::endl;

namespace zypp
{
  namespace ZyppFlags
  {
    template<>
    TableLineStyle argValueConvert ( const CommandOption &opt, const boost::optional<std::string> &in )
    {
      if ( !in || in->empty() ) ZYPP_THROW(MissingArgumentException(opt.name)); //value required

      unsigned s;
      str::strtonum( *in, s );
      if ( s < TableLineStyle::TLS_End )
        return (TableLineStyle)s;
      else
        throw InvalidValueException ( opt.name, *in,
                                      ( str::Format(_("Invalid table style %d.")) % s ).asString() +
                                      ( str::Format(_(" Use an integer number from %d to %d")) % 0 % ( (int)TableLineStyle::TLS_End - 1 ) ).asString()) ;
    }
  }
}

//////////////////////////////////////////////////////////////////
namespace
{
  void setColorForOut ( bool yesno_r )
  { Zypper::instance().out().setUseColors( yesno_r ); }

  void setOutVerbosity ( Out::Verbosity verb_r )
  { Zypper::instance().out().setVerbosity( verb_r ); }
} // namespace
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
namespace
{
  enum class ConfigOption {
    MAIN_SHOW_ALIAS,
    MAIN_REPO_LIST_COLUMNS,

    SOLVER_INSTALL_RECOMMENDS,
    SOLVER_FORCE_RESOLUTION_COMMANDS,

    COMMIT_AUTO_AGREE_WITH_LICENSES,
    COMMIT_PS_CHECK_ACCESS_DELETED,

    COLOR_USE_COLORS,
    COLOR_RESULT,
    COLOR_MSG_STATUS,
    COLOR_MSG_ERROR,
    COLOR_MSG_WARNING,
    COLOR_PROMPT,
    COLOR_PROMPT_OPTION,
    COLOR_POSITIVE,
    COLOR_CHANGE,
    COLOR_NEGATIVE,
    COLOR_HIGHLIGHT,
    COLOR_LOWLIGHT,
    COLOR_OSDEBUG,

    COLOR_PKGLISTHIGHLIGHT,
    COLOR_PKGLISTHIGHLIGHT_ATTRIBUTE,

    SEARCH_RUNSEARCHPACKAGES,

    OBS_BASE_URL,
    OBS_PLATFORM,

    SUBCOMMAND_SEACHSUBCOMMANDINPATH,
  };

  const std::vector<std::pair<std::string,ConfigOption>> & optionPairs()
  {
    static const std::vector<std::pair<std::string,ConfigOption>> _data = {
      { "main/showAlias",			ConfigOption::MAIN_SHOW_ALIAS			},
      { "main/repoListColumns",			ConfigOption::MAIN_REPO_LIST_COLUMNS		},
      { "solver/installRecommends",		ConfigOption::SOLVER_INSTALL_RECOMMENDS		},
      { "solver/forceResolutionCommands",	ConfigOption::SOLVER_FORCE_RESOLUTION_COMMANDS	},

      { "commit/autoAgreeWithLicenses",		ConfigOption::COMMIT_AUTO_AGREE_WITH_LICENSES	},
      { "commit/psCheckAccessDeleted",		ConfigOption::COMMIT_PS_CHECK_ACCESS_DELETED	},

      { "color/useColors",			ConfigOption::COLOR_USE_COLORS			},
      //"color/background"			LEGACY
      { "color/result",				ConfigOption::COLOR_RESULT			},
      { "color/msgStatus",			ConfigOption::COLOR_MSG_STATUS			},
      { "color/msgError",			ConfigOption::COLOR_MSG_ERROR			},
      { "color/msgWarning",			ConfigOption::COLOR_MSG_WARNING			},
      { "color/prompt",				ConfigOption::COLOR_PROMPT			},
      { "color/promptOption",			ConfigOption::COLOR_PROMPT_OPTION		},
      { "color/positive",			ConfigOption::COLOR_POSITIVE			},
      { "color/change",				ConfigOption::COLOR_CHANGE			},
      { "color/negative",			ConfigOption::COLOR_NEGATIVE			},
      { "color/highlight",			ConfigOption::COLOR_HIGHLIGHT			},
      { "color/lowlight",			ConfigOption::COLOR_LOWLIGHT			},
      { "color/osdebug",			ConfigOption::COLOR_OSDEBUG			},

      { "color/pkglistHighlight",		ConfigOption::COLOR_PKGLISTHIGHLIGHT		},
      { "color/pkglistHighlightAttribute",	ConfigOption::COLOR_PKGLISTHIGHLIGHT_ATTRIBUTE	},

      { "search/runSearchPackages",		ConfigOption::SEARCH_RUNSEARCHPACKAGES		},

      { "obs/baseUrl",				ConfigOption::OBS_BASE_URL			},
      { "obs/platform",				ConfigOption::OBS_PLATFORM			},

      { "subcommand/seachSubcommandInPath",	ConfigOption::SUBCOMMAND_SEACHSUBCOMMANDINPATH	},
    };
    return _data;
  }

  std::string asString( ConfigOption value_r )
  {
    for ( const auto & p : optionPairs() )
    {
      if ( p.second == value_r )
        return p.first;
    }
    return std::string();
  }

  ZyppFlags::SetterFun handleRootOptionCB ( Config &conf )
  {
    return [ &conf ] ( const ZyppFlags::CommandOption &opt, const boost::optional<std::string> &val ) {
      Zypper &zypper = Zypper::instance();
      conf.root_dir = *val;
      conf.changedRoot = true;
      conf.is_install_root = ( opt.name == "installroot");

      //make sure ZConfig knows the RepoManager root is not inside the target rootfs
      if ( conf.is_install_root )
        ZConfig::instance().setRepoManagerRoot("/");

      Pathname tmp( conf.root_dir );
      if ( !tmp.absolute() )
      {
        std::string reason = _("The path specified in the --root option must be absolute.");
        zypper.out().error( reason );
        zypper.setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
        ZYPP_THROW( ZyppFlags::InvalidValueException( opt.name, *val, reason ) );
      }

      DBG << "root dir = " << conf.root_dir << " is_install_root = " << conf.is_install_root << endl;
      if (conf.is_install_root)
        conf.rm_options = RepoManagerOptions("/");
      else
        conf.rm_options = RepoManagerOptions(conf.root_dir);
    };
  }

} // namespace
//////////////////////////////////////////////////////////////////

Config::Config()
  : repo_list_columns("anr")
  , solver_installRecommends(!ZConfig::instance().solver_onlyRequires())
  , psCheckAccessDeleted(true)
  , color_useColors	("autodetect")
  , color_pkglistHighlight(true)
  , color_pkglistHighlightAttribute(ansi::Color::nocolor())
  , search_runSearchPackages(indeterminate)		// ask
  , obs_baseUrl("https://download.opensuse.org/repositories/")
  , obs_platform("")	// guess
  , verbosity( Out::NORMAL )
  , disable_system_sources( false )
  , disable_system_resolvables( false )
  , non_interactive( false )
  , non_interactive_skip_manual_patches( false )
  , reboot_req_non_interactive( false )
  , no_gpg_checks( false )
  , gpg_auto_import_keys( false )
  , machine_readable( false )
  , no_refresh( false )
  , no_cd( false )
  , no_remote( false )
  , root_dir( "/" )
  , is_install_root( false )
  , no_abbrev( false )
  , terse( false )
  , changedRoot( false )
  , ignore_unknown( false )
  , exclude_optional_patches_default( true )
  , exclude_optional_patches( exclude_optional_patches_default )
  , wantHelp ( false )
{}

std::vector<ZyppFlags::CommandGroup> Config::cliOptions()
{
  //defines the order in which the CLI values are parsed.
  enum Priority : int {
    HIGHEST = 4000,
    CONFIG  = 3000,
    OUTPUT  = 2000,
    ROOT    = 1000
  };
  return {
    {
      "", //unnamed section
      {
        { "help", 'h', ZyppFlags::NoArgument, ZyppFlags::BoolType( &wantHelp, ZyppFlags::StoreTrue ),
          // translators: --help, -h
          _("Help.")
        },
        std::move( ZyppFlags::CommandOption(
          "version", 'V', ZyppFlags::NoArgument,
              ZyppFlags::CallbackVal ( []( const ZyppFlags::CommandOption &, const boost::optional<std::string> & ) {
                Zypper::instance().out().info( PACKAGE " " VERSION, Out::QUIET );
                ZYPP_THROW( ExitRequestException("version shown") );
              } ),
              // translators: --version, -V
              _("Output the version number.")
          ).setPriority( Priority::HIGHEST )
        ),
        std::move( ZyppFlags::CommandOption(
          "promptids", 0, ZyppFlags::NoArgument,
              ZyppFlags::CallbackVal ( []( const ZyppFlags::CommandOption &, const boost::optional<std::string> & ) {
                #define PR_ENUML(nam, val) Zypper::instance().out().info(#nam "=" #val, Out::QUIET);
                #define PR_ENUM(nam, val) PR_ENUML(nam, val)
                #include "output/prompt.h"
                ZYPP_THROW( ExitRequestException("promptids shown") );
              } ),
              // translators: --promptids
              _("Output a list of zypper's user prompts.")
          ).setPriority( Priority::HIGHEST )
        ),
        std::move( ZyppFlags::CommandOption(
          "config", 'c', ZyppFlags::RequiredArgument, std::move(
              ZyppFlags::CallbackVal ( [ this ]( const ZyppFlags::CommandOption &, const boost::optional<std::string> & val ){
                this->read ( *val );
              }, ARG_FILE ).notSeen( [ this ]( ) {
                //config option was never used. Load the default
                this->read();
              })
            ),
            // translators: --config, -c <FILE>
            _("Use specified config file instead of the default.")
          ).setPriority( Priority::CONFIG )
        ),
        { "userdata", 0, ZyppFlags::RequiredArgument,
              ZyppFlags::CallbackVal( []( const ZyppFlags::CommandOption &, const boost::optional<std::string> &val ) {
                if ( ! ZConfig::instance().setUserData( *val ) ) {
                  auto &zypp = Zypper::instance();
                  zypp.out().error(_("User data string must not contain nonprintable or newline characters!"));
                  zypp.setExitCode( ZYPPER_EXIT_ERR_INVALID_ARGS );
                  ZYPP_THROW( ExitRequestException("userdata") );
                }
              }, ARG_STRING ),
              // translators: --userdata <STRING>
              _("User defined transaction id used in history and plugins.")
        },
        std::move( ZyppFlags::CommandOption(
            "quiet", 'q', ZyppFlags::NoArgument,
            std::move( ZyppFlags::WriteFixedValueType( verbosity, Out::QUIET ).after( [this](){
              setOutVerbosity( verbosity );
            })),
            // translators: --quiet, -q
            _("Suppress normal output, print only error messages.")
        ).setPriority( Priority::OUTPUT + 1 )),
        std::move( ZyppFlags::CommandOption(
           "verbose", 'v', ZyppFlags::NoArgument | ZyppFlags::Repeatable,
            std::move( ZyppFlags::CounterType( reinterpret_cast<int *>( &verbosity ), boost::optional<int>(), static_cast<int>( Out::DEBUG ), false ).after( [this](){
              setOutVerbosity( verbosity );
            })),
            // translators: --verbose, -v
            _("Increase verbosity.")
        ).setPriority( Priority::OUTPUT + 1 )),
        // rug compatibility alias for -vv
        std::move( ZyppFlags::CommandOption(
            "debug", 0, ZyppFlags::NoArgument | ZyppFlags::Hidden,
            std::move( ZyppFlags::WriteFixedValueType( verbosity, Out::DEBUG ).after( [this](){
              setOutVerbosity( verbosity );
            })),
            ""
        ).setPriority( Priority::OUTPUT + 1 )),
        std::move( ZyppFlags::CommandOption(
            "color", 0, ZyppFlags::NoArgument, std::move( ZyppFlags::BoolType( &do_colors, ZyppFlags::StoreTrue ).after([ this ](){
              setColorForOut( do_colors );
            })),
            ""
          ).setDependencies( { "terse" } ) //do not parse before terse, so color can be enabled again
        ),
        { "no-color", 0, ZyppFlags::NoArgument, std::move( ZyppFlags::BoolType( &do_colors, ZyppFlags::StoreFalse ).after([ this ](){
                setColorForOut( do_colors );
              })),
              // translators: --color / --no-color
              _("Whether to use colors in output if tty supports it.")
        },
        std::move( ZyppFlags::CommandOption(
          "no-abbrev", 'A', ZyppFlags::NoArgument, ZyppFlags::BoolType( &no_abbrev, ZyppFlags::StoreTrue, no_abbrev ),
          // translators: --no-abbrev, -A
          _("Do not abbreviate text in tables.")
          ).setDependencies( { "terse" } )
        ),
        { "table-style", 's', ZyppFlags::RequiredArgument, ZyppFlags::GenericValueType( Table::defaultStyle, ARG_INTEGER ),
              // translators: --table-style, -s, %1% denotes the supported range of the integer argument (e.g. "1-11")
              str::Format(_("Table style (%1%).") ) % ("0-"+str::numstring((int)TableLineStyle::TLS_End-1))
        },
        { "non-interactive", 'n', ZyppFlags::NoArgument, std::move( ZyppFlags::BoolType( &non_interactive, ZyppFlags::StoreTrue, non_interactive )
              .after( []( ){
                  Zypper::instance().out().info(_("Entering non-interactive mode."), Out::HIGH );
                  MIL << "Entering non-interactive mode" << endl;
              })),
              // translators: --non-interactive, -n
              _("Do not ask anything, use default answers automatically.")
        },
        { "non-interactive-include-reboot-patches", '\0', ZyppFlags::NoArgument,
              std::move( ZyppFlags::BoolType( &reboot_req_non_interactive, ZyppFlags::StoreTrue, reboot_req_non_interactive )
              .after( []( ){
                  Zypper::instance().out().info(_("Patches having the flag rebootSuggested set will not be treated as interactive."), Out::HIGH );
                  MIL << "Patches having the flag rebootSuggested set will not be treated as interactive" << endl;
              })),
              // translators: --non-interactive-include-reboot-patches
              _("Do not treat patches as interactive, which have the rebootSuggested-flag set.")
        },
        std::move( ZyppFlags::CommandOption(
          "xmlout", 'x', ZyppFlags::NoArgument, ZyppFlags::CallbackVal( [ this ]( const ZyppFlags::CommandOption &, const boost::optional<std::string> & ) {
                do_colors = false;	// no color in xml mode!
                Zypper::instance().setOutputWriter( new OutXML( verbosity ) );
                machine_readable = true;
                no_abbrev = true;
              }),
              // translators: --xmlout, -x
              _("Switch to XML output.")
          ).setPriority( Priority::OUTPUT )
        ),
        { "ignore-unknown", 'i', ZyppFlags::NoArgument, ZyppFlags::BoolType( &ignore_unknown, ZyppFlags::StoreTrue, ignore_unknown ),
              // translators: --ignore-unknown, -i
              _("Ignore unknown packages.")
        },
        { "terse", 't', ZyppFlags::NoArgument,
            ZyppFlags::CallbackVal( [ this ]( const ZyppFlags::CommandOption &, const boost::optional<std::string> & ) {
              machine_readable = true;
              no_abbrev = true;
              terse = true;
              do_colors = false;
              setColorForOut( do_colors );
            }),
            // translators: --terse, -t
            _("Terse output for machine consumption. Implies --no-abbrev and --no-color.")
        },
        // -------------------- deprecated and hidden switches------------------------------------------

        // rug compatibility alias for the default output level => ignored
        { "normal-output", 0, ZyppFlags::NoArgument | ZyppFlags::Hidden, ZyppFlags::NoValue(), "" },
        { "rug-compatible", 'r', ZyppFlags::NoArgument | ZyppFlags::Hidden,
            ZyppFlags::CallbackVal ( []( const ZyppFlags::CommandOption &, const boost::optional<std::string> & ) {
              /* DEPRECATED and UNSUPPORTED SINCE SLE12 */
              exit_rug_compat();
            } )
        }
      }, {
        //conflicting flags
        { "quiet", "verbose", "debug" },
        { "color", "no-color" },
        { "color", "xmlout" } //color will always be disabled for XML
      }
    } , {
      //start a new section of commands
      "", //unnamed section
      {
        { "reposd-dir", 'D', ZyppFlags::RequiredArgument, ZyppFlags::PathNameType( rm_options.knownReposPath, boost::optional<std::string>(), ARG_DIR ),
              // translators: --reposd-dir, -D <DIR>
              _("Use alternative repository definition file directory.")
        },
        { "cache-dir", 'C', ZyppFlags::RequiredArgument,
              ZyppFlags::CallbackVal( [ this ]( const ZyppFlags::CommandOption &, const boost::optional<std::string> &val ) {
                ZConfig &zconfig = ZConfig::instance();
                if ( val ) {
                  zconfig.setRepoCachePath( *val );
                  rm_options.repoCachePath          = zconfig.repoCachePath();
                  rm_options.repoRawCachePath       = zconfig.repoMetadataPath();
                  rm_options.repoSolvCachePath      = zconfig.repoSolvfilesPath();
                  rm_options.repoPackagesCachePath  = zconfig.repoPackagesPath();
                }
              }, ARG_DIR ),

              // translators: --cache-dir, -C <DIR>
              _("Use alternative directory for all caches.")
        },
        std::move( ZyppFlags::CommandOption(
            "raw-cache-dir",  0, ZyppFlags::RequiredArgument,
            ZyppFlags::CallbackVal( [ this ]( const ZyppFlags::CommandOption &, const boost::optional<std::string> &val ) {
              ZConfig &zconfig = ZConfig::instance();
              if ( val ) {
                ZConfig::instance().setRepoMetadataPath( *val );
                rm_options.repoRawCachePath = zconfig.repoMetadataPath();
              }
            }, ARG_DIR ),

            // translators: --raw-cache-dir <DIR>
            _("Use alternative raw meta-data cache directory.")
          ).setDependencies( { "cache-dir" } )
        ),
        std::move( ZyppFlags::CommandOption(
            "solv-cache-dir", 0, ZyppFlags::RequiredArgument,
            ZyppFlags::CallbackVal( [ this ]( const ZyppFlags::CommandOption &, const boost::optional<std::string> &val ) {
              ZConfig &zconfig = ZConfig::instance();
              if ( val ) {
                ZConfig::instance().setRepoSolvfilesPath( *val );
                rm_options.repoSolvCachePath = zconfig.repoSolvfilesPath();
              }
            }, ARG_DIR ),

            // translators: --solv-cache-dir <DIR>
            _("Use alternative solv file cache directory.")
          ).setDependencies( { "cache-dir" } )
        ),
        std::move( ZyppFlags::CommandOption(
            "pkg-cache-dir", 0, ZyppFlags::RequiredArgument,
            ZyppFlags::CallbackVal( [ this ]( const ZyppFlags::CommandOption &, const boost::optional<std::string> &val ) {
              ZConfig &zconfig = ZConfig::instance();
              if ( val ) {
                ZConfig::instance().setRepoPackagesPath( *val );
                rm_options.repoPackagesCachePath = zconfig.repoPackagesPath();
              }
            }, ARG_DIR ),

            // translators: --pkg-cache-dir <DIR>
            _("Use alternative package cache directory.")
          ).setDependencies( { "cache-dir" } )
        )
      }
    } , {
      _("Repository Options") ,
      {
        { "no-gpg-checks", 0, ZyppFlags::NoArgument, std::move( ZyppFlags::BoolType( &no_gpg_checks, ZyppFlags::StoreTrue, no_gpg_checks )
              .after( [](){
                Zypper::instance().out().info(_("Entering 'no-gpg-checks' mode."), Out::HIGH );
                MIL << "Entering no-gpg-checks mode" << endl;
              } )),
              // translators: --no-gpg-checks
              _("Ignore GPG check failures and continue.")
        },
        { "gpg-auto-import-keys", 0, ZyppFlags::NoArgument, std::move( ZyppFlags::BoolType( &gpg_auto_import_keys, ZyppFlags::StoreTrue ).
              after( []() {
                std::string warn = str::form(
                  _("Turning on '%s'. New repository signing keys will be automatically imported!"),
                  "--gpg-auto-import-keys");
                Zypper::instance().out().warning( warn, Out::HIGH );
                MIL << "gpg-auto-import-keys is on" << endl;
              })),
              // translators: --gpg-auto-import-keys
              _("Automatically trust and import new repository signing keys.")
        },
        { "plus-repo", 'p', ZyppFlags::Repeatable | ZyppFlags::RequiredArgument, ZyppFlags::GenericContainerType( plusRepoFromCLI, ARG_URI ),
              // translators: --plus-repo, -p <URI>
              _("Use an additional repository.")
        },
        { "plus-content", 0, ZyppFlags::Repeatable | ZyppFlags::RequiredArgument, ZyppFlags::GenericContainerType( plusContentFromCLI, ARG_TAG ),
              // translators: --plus-content <TAG>
              _("Additionally use disabled repositories providing a specific keyword. Try '--plus-content debug' to enable repos indicating to provide debug packages.")
        },
        { "disable-repositories", 0, ZyppFlags::NoArgument, std::move(ZyppFlags::BoolType( &disable_system_sources, ZyppFlags::StoreTrue )
              .after([](){
                MIL << "Repositories disabled, using target only." << endl;
                Zypper::instance().out().info(
                  _("Repositories disabled, using the database of installed packages only."),
                  Out::HIGH);
              })),
              // translators: --disable-repositories
              _("Do not read meta-data from repositories.")
        },
        { "no-refresh", 0, ZyppFlags::NoArgument, std::move(ZyppFlags::BoolType( &no_refresh, ZyppFlags::StoreTrue )
              .after( []() {
                Zypper::instance().out().info(_("Autorefresh disabled."), Out::HIGH );
                MIL << "Autorefresh disabled." << endl;
              })),
              // translators: --no-refresh
              _("Do not refresh the repositories.")
        },
        { "no-cd", 0, ZyppFlags::NoArgument, std::move(ZyppFlags::BoolType( &no_cd, ZyppFlags::StoreTrue )
              .after( [](){
                Zypper::instance().out().info(_("CD/DVD repositories disabled."), Out::HIGH );
                MIL << "No CD/DVD repos." << endl;
              })),
              // translators: --no-cd
              _("Ignore CD/DVD repositories.")
        },
        { "no-remote", 0, ZyppFlags::NoArgument, std::move( ZyppFlags::BoolType( &no_remote, ZyppFlags::StoreTrue )
              .after( [](){
                Zypper::instance().out().info(_("Remote repositories disabled."), Out::HIGH );
                MIL << "No remote repos." << endl;
              })),
              // translators: --no-remote
              _("Ignore remote repositories.")
        },
        { "releasever", 0, ZyppFlags::RequiredArgument,  ZyppFlags::CallbackVal( []( const ZyppFlags::CommandOption &, const boost::optional<std::string> &val ){
                if ( val )
                  ::setenv( "ZYPP_REPO_RELEASEVER", (*val).c_str(), 1 );
              }),
              // translators: --releasever
              _("Set the value of $releasever in all .repo files (default: distribution version)")
        }
      }
    } , {
      _("Target Options"),
      {
        std::move( ZyppFlags::CommandOption(
          "root", 'R', ZyppFlags::RequiredArgument, ZyppFlags::CallbackVal( handleRootOptionCB( *this ), ARG_DIR ),
                    // translators: --root, -R <DIR>
                    _("Operate on a different root directory.")
          ).setPriority( Priority::ROOT )
        ),
        std::move( ZyppFlags::CommandOption(
          "installroot", '\0', ZyppFlags::RequiredArgument, ZyppFlags::CallbackVal( handleRootOptionCB( *this ), ARG_DIR ),
          // translators: --installroot <DIR>
          _("Operate on a different root directory, but share repositories with the host.")
          ).setPriority( Priority::ROOT )
        ),
        { "disable-system-resolvables", 0, ZyppFlags::NoArgument,
            std::move( ZyppFlags::BoolType( &disable_system_resolvables, ZyppFlags::StoreTrue ).after( []() {
                MIL << "System resolvables disabled" << endl;
                Zypper::instance().out().info(_("Ignoring installed resolvables."), Out::HIGH );
            })),
            // translators: --disable-system-resolvables
            _("Do not read installed packages.")
        },
      },
      //conflicting flags
      {
        { "root", "installroot" }
      }
    }
  };
}

void Config::read( const std::string & file )
{
  try
  {
    debug::Measure m("ReadConfig");
    std::string s;

    Augeas augeas( file );

    m.elapsed();

    // ---------------[ main ]--------------------------------------------------

    s = augeas.getOption(asString( ConfigOption::MAIN_SHOW_ALIAS ));
    if (!s.empty())
    {
      // using Repository::asUserString() will follow repoLabelIsAlias!
      ZConfig::instance().repoLabelIsAlias( str::strToBool(s, false) );
    }

    s = augeas.getOption(asString( ConfigOption::MAIN_REPO_LIST_COLUMNS ));
    if (!s.empty()) // TODO add some validation
      repo_list_columns = s;

    // ---------------[ solver ]------------------------------------------------

    s = augeas.getOption(asString( ConfigOption::SOLVER_INSTALL_RECOMMENDS ));
    if (s.empty())
      solver_installRecommends = !ZConfig::instance().solver_onlyRequires();
    else
      solver_installRecommends = str::strToBool(s, true);

    s = augeas.getOption(asString( ConfigOption::SOLVER_FORCE_RESOLUTION_COMMANDS ));
    if (s.empty())
      solver_forceResolutionCommands.insert(ZypperCommand::REMOVE);
    else
    {
      std::list<std::string> cmdstr;
      str::split(s, std::back_inserter(cmdstr), ",");
      for_(c, cmdstr.begin(), cmdstr.end())
        solver_forceResolutionCommands.insert(ZypperCommand(str::trim(*c)));
    }

    // ---------------[ commit ]------------------------------------------------

    s = augeas.getOption( asString(ConfigOption::COMMIT_AUTO_AGREE_WITH_LICENSES) );
    if ( ! s.empty() )
      LicenseAgreementPolicyData::_defaultAutoAgreeWithLicenses = str::strToBool( s, LicenseAgreementPolicyData::_defaultAutoAgreeWithLicenses );

    s = augeas.getOption(asString( ConfigOption::COMMIT_PS_CHECK_ACCESS_DELETED ));
    if ( ! s.empty() )
      psCheckAccessDeleted = str::strToBool( s, psCheckAccessDeleted );

    // ---------------[ colors ]------------------------------------------------

    s = augeas.getOption( asString( ConfigOption::COLOR_USE_COLORS ) );
    if (!s.empty())
      color_useColors = s;

    do_colors = ( color_useColors == "autodetect" && hasANSIColor() ) || color_useColors == "always";

    ansi::Color c;
    for ( const auto & el : std::initializer_list<std::pair<ansi::Color &, ConfigOption>> {
      { color_result,		ConfigOption::COLOR_RESULT		},
      { color_msgStatus,	ConfigOption::COLOR_MSG_STATUS		},
      { color_msgError,		ConfigOption::COLOR_MSG_ERROR		},
      { color_msgWarning,	ConfigOption::COLOR_MSG_WARNING		},
      { color_prompt,		ConfigOption::COLOR_PROMPT		},
      { color_promptOption,	ConfigOption::COLOR_PROMPT_OPTION	},
      { color_positive,		ConfigOption::COLOR_POSITIVE		},
      { color_change,		ConfigOption::COLOR_CHANGE		},
      { color_negative,		ConfigOption::COLOR_NEGATIVE		},
      { color_highlight,	ConfigOption::COLOR_HIGHLIGHT		},
      { color_lowlight,		ConfigOption::COLOR_LOWLIGHT		},
      { color_pkglistHighlightAttribute, ConfigOption::COLOR_PKGLISTHIGHLIGHT_ATTRIBUTE },
    } )
    {
      c = ansi::Color::fromString( augeas.getOption( asString( el.second ) ) );
      if ( c )
        el.first = c;
      // Fix color attributes: Default is mapped to Unchanged to allow
      // using the ColorStreams default rather than the terminal default.
      if ( el.second == ConfigOption::COLOR_PKGLISTHIGHLIGHT_ATTRIBUTE )	// currently the only one
      {
        ansi::Color & c( el.first );
        if ( c.fg() == ansi::Color::Fg::Default )
          c.fg( ansi::Color::Fg::Unchanged );
        if ( c.bg() == ansi::Color::Bg::Default )
          c.bg( ansi::Color::Bg::Unchanged );
      }
    }

    s = augeas.getOption( asString( ConfigOption::COLOR_PKGLISTHIGHLIGHT ) );
    if (!s.empty())
    {
      if ( s == "all" )
        color_pkglistHighlight = true;
      else if ( s == "first" )
        color_pkglistHighlight = indeterminate;
      else if ( s == "no" )
        color_pkglistHighlight = false;
      else
        WAR << "zypper.conf: color/pkglistHighlight: unknown value '" << s << "'" << endl;
    }

    s = augeas.getOption("color/background");	// legacy
    if ( !s.empty() )
      WAR << "zypper.conf: ignore legacy option 'color/background'" << endl;

    // ---------------[ search ]------------------------------------------------

    s = augeas.getOption( asString( ConfigOption::SEARCH_RUNSEARCHPACKAGES ) );
    if ( !s.empty() )
      search_runSearchPackages = str::strToTriBool( s );

    // ---------------[ obs ]---------------------------------------------------

    s = augeas.getOption(asString( ConfigOption::OBS_BASE_URL ));
    if (!s.empty())
    {
      try { obs_baseUrl = Url(s); }
      catch (Exception & e)
      {
        ERR << "Invalid OBS base URL (" << e.msg() << "), will use the default." << endl;
      }
    }

    s = augeas.getOption(asString( ConfigOption::OBS_PLATFORM ));
    if (!s.empty())
      obs_platform = s;

    s = augeas.getOption( asString( ConfigOption::SUBCOMMAND_SEACHSUBCOMMANDINPATH ) );
    if ( not s.empty() )
      seach_subcommand_in_path = str::strToBool( s, seach_subcommand_in_path );

    // finally remember the default config file for saving back values
    _cfgSaveFile = augeas.getSaveFile();
    m.stop();
  }
  catch (Exception & e)
  {
    std::cerr << e.asUserHistory() << endl;
    std::cerr << "*** Augeas exception: No config files read, sticking with defaults." << endl;
  }

  setColorForOut( do_colors );
}


void Config::saveback_search_runSearchPackages( const TriBool & value_r )
{
  auto & out { Zypper::instance().out() };

  if ( _cfgSaveFile.empty() )
  {
    out.gap();
    out.errorPar(_("No config file is in use. Can not save options.") );
  }
  else try
  {
    Augeas augeas( _cfgSaveFile, /*readmode=*/false );

    const std::string & value { asString( value_r, "ask", "always", "never" ) };
    augeas.setOption( asString( ConfigOption::SEARCH_RUNSEARCHPACKAGES ), value );
    augeas.save();

    out.gap();
    out.info( str::Format(_("Option '%1%' saved in '%2%'.") ) % ("[search] runSearchPackages = "+value) %_cfgSaveFile );

  }
  catch ( const Exception & excpt )
  {
    ZYPP_CAUGHT( excpt );
    out.gap();
    out.error( excpt, "", _("Failed to save option.") );
  }
}
