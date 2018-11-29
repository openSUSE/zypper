/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "optionsets.h"
#include "utils/flags/flagtypes.h"
#include "global-settings.h"
#include "Zypper.h"

#include <sstream>

void RugCompatModeMixin::setCompatibilityMode(CompatModeFlags flags_r)
{
  _compatMode = flags_r;
}

std::vector<ZyppFlags::CommandGroup> DryRunOptionSet::options()
{
  std::vector<ZyppFlags::CommandGroup> myOpts;
  if ( _compatMode.testFlag( CompatModeBits::EnableRugOpt ) ) {
    // rug uses -N shorthand
    myOpts.push_back( {{
       { "rug-dry-run", 'N', ZyppFlags::NoArgument | ZyppFlags::Hidden | ZyppFlags::Deprecated, ZyppFlags::BoolType( &DryRunSettings::instanceNoConst()._enabled, ZyppFlags::StoreTrue, DryRunSettings::instance()._enabled ),
            "" }
    }});
  }

  if ( _compatMode.testFlag( CompatModeBits::EnableNewOpt ) ) {
    myOpts.push_back( {{
       { "dry-run", 'D', ZyppFlags::NoArgument, ZyppFlags::BoolType( &DryRunSettings::instanceNoConst()._enabled, ZyppFlags::StoreTrue, DryRunSettings::instance()._enabled ),
            _("Don't change anything, just report what would be done.")}
    }});
  }

  return myOpts;
}

void DryRunOptionSet::reset()
{
  DryRunSettings::reset();
}

std::vector<ZyppFlags::CommandGroup> InitReposOptionSet::options()
{
  std::vector<ZyppFlags::CommandGroup> myOpts;
  if ( _compatMode.testFlag( CompatModeBits::EnableRugOpt ) ) {
    myOpts.push_back( {{
            { "catalog", 'c',
                  ZyppFlags::RequiredArgument | ZyppFlags::Repeatable | ZyppFlags::Deprecated | ZyppFlags::Hidden,
                  ZyppFlags::StringVectorType( &InitRepoSettings::instanceNoConst()._repoFilter, ARG_REPOSITORY ),
                  ""
            }
        }}
    );
  }

  if ( _compatMode.testFlag( CompatModeBits::EnableNewOpt ) ) {
    myOpts.push_back( {{
           { "repo", 'r',
                 ZyppFlags::RequiredArgument | ZyppFlags::Repeatable,
                 ZyppFlags::StringVectorType( &InitRepoSettings::instanceNoConst()._repoFilter, ARG_REPOSITORY ),
                 // translators: -r, --repo <ALIAS|#|URI>
                 _("Work only with the specified repository.")
           }
        }}
    );
  }

  return myOpts;
}

void InitReposOptionSet::reset()
{
  InitRepoSettings::reset();
}


namespace
{

  inline std::string overrideWarning ()
  {
    return _("Option '--%s' overrides a previously set download mode.");
  }

  //A flag type that takes a download mode string and maps it to the correct Download  mode
  ZyppFlags::Value DownloadModeArgType( DownloadOptionSet &target, DownloadMode defValue ) {
    return ZyppFlags::Value (
      [defValue]() ->  boost::optional<std::string>{
        std::stringstream str;
        str << defValue;
        return str.str();
      },
      [ &target ]( const ZyppFlags::CommandOption &opt, const boost::optional<std::string> &in ){
        if (!in)
          ZYPP_THROW(ZyppFlags::MissingArgumentException(opt.name));

        if ( target.wasSetBefore() ) {
          Zypper::instance().out().warning(
            str::form( overrideWarning().c_str(), ( opt.name + " " + *in ).c_str() ) );
        }

        if (*in == "only")
          target.setMode( DownloadOnly );
        else if (*in == "in-advance")
          target.setMode( DownloadInAdvance );
        else if (*in == "in-heaps")
          target.setMode( DownloadInHeaps );
        else if (*in == "as-needed")
          target.setMode( DownloadAsNeeded );
        else {
          ZYPP_THROW( ZyppFlags::InvalidValueException( opt.name, *in, str::form(_("Available download modes: %s"), "only, in-advance, in-heaps, as-needed") ) );
        }
        return;
      }
    );
  }

  //A flag type that directly writes the DownloadMode variable
  ZyppFlags::Value DownloadModeNoArgType( DownloadOptionSet &target, DownloadMode setFlag ) {
    return ZyppFlags::Value (
      ZyppFlags::noDefaultValue,
      [ &target, setFlag ]( const ZyppFlags::CommandOption &opt, const boost::optional<std::string> & ){

        if ( target.wasSetBefore() ) {
          Zypper::instance().out().warning(
            str::form( overrideWarning().c_str(), opt.name.c_str() ) );
        }

        target.setMode( setFlag );
        return;
      }
    );
  }
}

zypp::DownloadMode DownloadOptionSet::mode() const
{
  MIL << "Download mode: ";
  if      (_mode == DownloadInAdvance) MIL << "in-advance";
  else if (_mode == DownloadInHeaps)   MIL << "in-heaps";
  else if (_mode == DownloadOnly)      MIL << "only";
  else if (_mode == DownloadAsNeeded)  MIL << "as-needed";
  else                                 MIL << "UNKNOWN";
  MIL << (_mode == ZConfig::instance().commit_downloadMode() ? " (zconfig value)" : "") << endl;

  return _mode;
}

void DownloadOptionSet::setMode(const zypp::DownloadMode &mode)
{
  _mode = mode;
  _wasSetBefore = true;
}

bool DownloadOptionSet::wasSetBefore() const
{
  return _wasSetBefore;
}

std::vector<ZyppFlags::CommandGroup> DownloadOptionSet::options()
{
  // All the flags are defined as Repeatable, even though they do not fill a list, we want it to be possible to override
  // the download mode. This is more in sync with the previous behaviour
  return {{{
        { "download", '\0', ZyppFlags::RequiredArgument | ZyppFlags::Repeatable, DownloadModeArgType( *this, _mode ),
              // translators: --download
              str::Format(_("Set the download-install mode. Available modes: %s") ) % "only, in-advance, in-heaps, as-needed"
        },
        { "download-only", 'd', ZyppFlags::NoArgument | ZyppFlags::Repeatable, DownloadModeNoArgType( *this, DownloadMode::DownloadOnly ),
              // translators: -d, --download-only
              _("Only download the packages, do not install.")
        },
        { "download-in-advance", '\0', ZyppFlags::NoArgument | ZyppFlags::Repeatable | ZyppFlags::Hidden, DownloadModeNoArgType( *this, DownloadMode::DownloadInAdvance ), "" },
        { "download-in-heaps",   '\0', ZyppFlags::NoArgument | ZyppFlags::Repeatable | ZyppFlags::Hidden, DownloadModeNoArgType( *this, DownloadMode::DownloadInHeaps ), "" },
        { "download-as-needed",  '\0', ZyppFlags::NoArgument | ZyppFlags::Repeatable | ZyppFlags::Hidden, DownloadModeNoArgType( *this, DownloadMode::DownloadAsNeeded ), "" }
  }}};
}

void DownloadOptionSet::reset()
{
  _mode = ZConfig::instance().commit_downloadMode();
  _wasSetBefore = false;
}


std::vector<ZyppFlags::CommandGroup> NotInstalledOnlyOptionSet::options()
{
  return {{{
        { "installed-only",      'i',  ZyppFlags::NoArgument, ZyppFlags::WriteFixedValueType( _mode, SolvableFilterMode::ShowOnlyInstalled ),
              // translators: -i, --installed-only
              _("Show only installed packages.")
        },
        { "not-installed-only",  'u',  ZyppFlags::NoArgument, ZyppFlags::WriteFixedValueType( _mode, SolvableFilterMode::ShowOnlyNotInstalled ),
              // translators: -u, --not-installed-only
              _("Show only packages which are not installed.")
        },
        // bsc#972997: Prefer --not-installed-only over misleading --uninstalled-only
        { "uninstalled-only",    '\0', ZyppFlags::NoArgument | ZyppFlags::Hidden | ZyppFlags::Deprecated, ZyppFlags::WriteFixedValueType( _mode, SolvableFilterMode::ShowOnlyNotInstalled ), "" }
  }}};
}

void NotInstalledOnlyOptionSet::reset()
{
  _mode = SolvableFilterMode::ShowAll;
}


std::vector<ZyppFlags::CommandGroup> LicensePolicyOptionSet::options()
{
  auto &set = LicenseAgreementPolicy::instanceNoConst();
  std::vector<ZyppFlags::CommandGroup> myOpts;
  if ( _compatMode.testFlag( CompatModeBits::EnableRugOpt ) ) {
    myOpts.push_back( {{
            { "agree-to-third-party-licenses", '\0',
                  ZyppFlags::NoArgument | ZyppFlags::Deprecated | ZyppFlags::Hidden,
                  ZyppFlags::BoolType( &set._autoAgreeWithLicenses, ZyppFlags::StoreTrue ),
                  ""
            }
        }}
    );
  }

  if ( _compatMode.testFlag( CompatModeBits::EnableNewOpt ) ) {
    myOpts.push_back( {{
           { "auto-agree-with-licenses", 'l',
                 ZyppFlags::NoArgument,
                 ZyppFlags::BoolType( &set._autoAgreeWithLicenses, ZyppFlags::StoreTrue, set._autoAgreeWithLicenses ),
                 _("Automatically say 'yes' to third party license confirmation prompt. See 'man zypper' for more details.")
           },
           { "auto-agree-with-product-licenses", '\0',
                 ZyppFlags::NoArgument | ZyppFlags::Hidden,
                 ZyppFlags::BoolType( &set._autoAgreeWithProductLicenses, ZyppFlags::StoreTrue, set._autoAgreeWithProductLicenses ),
                 _("Automatically accept product licenses only. See 'man zypper' for more details.")
           }
        }}
    );
  }

  return myOpts;
}

void LicensePolicyOptionSet::reset()
{
  LicenseAgreementPolicy::reset();
}

std::vector<ZyppFlags::CommandGroup> FileConflictPolicyOptionSet::options()
{
  FileConflictPolicyData &set = FileConflictPolicy::instanceNoConst();
  return {{{
        { "replacefiles", '\0', ZyppFlags::NoArgument, ZyppFlags::BoolType( &set._replaceFiles, ZyppFlags::StoreTrue, set._replaceFiles ),
            // translators: --replacefiles
            _("Install the packages even if they replace files from other, already installed, packages. Default is to treat file conflicts as an error. --download-as-needed disables the fileconflict check.")
        }
  }}};
}

void FileConflictPolicyOptionSet::reset()
{
  FileConflictPolicy::reset();
}

std::vector<ZyppFlags::CommandGroup> NoConfirmRugOption::options()
{
  return {{{
      { "no-confirm", 'y', ZyppFlags::NoArgument, ZyppFlags::Value(
                [] () -> boost::optional<std::string> {
                  return std::string( ( Zypper::instance().globalOpts().non_interactive ) ? "true" : "false" );
                },
                []( const ZyppFlags::CommandOption &, const boost::optional<std::string> & ) {
                  bool &set = Zypper::instance().globalOptsNoConst().non_interactive;
                  if ( ! set ) {
                    Zypper::instance().out().info(_("Entering non-interactive mode."), Out::HIGH );
                    MIL << "Entering non-interactive mode" << endl;
                    set = true;
                  }
                }
            ),
            // translators: -y, --no-confirm
            _("Don't require user interaction. Alias for the --non-interactive global option.")
      }
  }}};
}

void NoConfirmRugOption::reset()
{
  //@bug might be handled wrong in zypper shell
}


std::vector<ZyppFlags::CommandGroup> OptionalPatchesOptionSet::options()
{
  auto &gOpts = Zypper::instance().globalOptsNoConst();
  return {{
      {
        {"with-optional", '\0', ZyppFlags::NoArgument, ZyppFlags::BoolCompatibleType( gOpts.exclude_optional_patches, ZyppFlags::StoreFalse ), "" },
        {"without-optional", '\0', ZyppFlags::NoArgument, ZyppFlags::BoolCompatibleType( gOpts.exclude_optional_patches, ZyppFlags::StoreTrue ),
              _("Whether applicable optional patches should be treated as needed or be excluded.")
                  + std::string(" ")
                  + ( gOpts.exclude_optional_patches_default
                  ? _("The default is to exclude optional patches.")
                  : _("The default is to include optional patches.") )
        }
      },{
          //conflicting flags
          { "with-optional",  "without-optional" }
      }
  }
 };
}

void OptionalPatchesOptionSet::reset()
{
  auto &gOpts = Zypper::instance().globalOptsNoConst();
  gOpts.exclude_optional_patches = gOpts.exclude_optional_patches_default;
}


bool InteractiveUpdatesOptionSet::skipInteractive() const
{
  bool skip_interactive = false;
  if ( _withInteractive != zypp::indeterminate )
    skip_interactive = ! _withInteractive;
  // bnc #497711
  else if ( Zypper::instance().globalOpts().non_interactive )
    skip_interactive = true;

  return skip_interactive;
}

std::vector<ZyppFlags::CommandGroup> InteractiveUpdatesOptionSet::options()
{
  return {{
      {
        {"with-interactive", '\0', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( _withInteractive, ZyppFlags::StoreTrue ),
              // translators: --with-interactive
              _("Do not skip interactive updates.")
        },
        {"skip-interactive", '\0', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( _withInteractive, ZyppFlags::StoreFalse ),
              // translators: --skip-interactive
              _("Skip interactive updates.")
        }
      },{
        //conflicting flags
        { "with-interactive", "skip-interactive" }
      }
  }};
}

void InteractiveUpdatesOptionSet::reset()
{
  _withInteractive = zypp::indeterminate;
}
