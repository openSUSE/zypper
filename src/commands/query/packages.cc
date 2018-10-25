/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "packages.h"
#include "Zypper.h"
#include "utils/flags/flagtypes.h"
#include "global-settings.h"

using namespace zypp;

PackagesCmdBase::PackagesCmdBase(const std::vector<std::string> &commandAliases_r) :
  ZypperBaseCommand (
    commandAliases_r,
    // translators: command synopsis; do not translate lowercase words
    _("packages (pa) [OPTIONS] [REPOSITORY] ..."),
    // translators: command summary: packages, pa
    _("List all available packages."),
    // translators: command description
    _("List all packages available in specified repositories."),
    DisableAll
  )
{
  _initRepoFlags.setCompatibilityMode( InitReposOptionSet::CompatModeBits::EnableRugOpt | InitReposOptionSet::CompatModeBits::EnableNewOpt );
}


zypp::ZyppFlags::CommandGroup PackagesCmdBase::cmdOptions() const
{
  auto that = const_cast<PackagesCmdBase *>(this);
  return {{
      {"orphaned", '\0', ZyppFlags::NoArgument, ZyppFlags::BitFieldType( that->_flags, ListPackagesBits::ShowOrphaned ),
            // translators: --orphaned
            _("Show packages which are orphaned (without repository).")
      },
      {"suggested", '\0', ZyppFlags::NoArgument, ZyppFlags::BitFieldType( that->_flags, ListPackagesBits::ShowSuggested ),
            // translators: --suggested
            _("Show packages which are suggested.")
      },
      {"recommended", '\0', ZyppFlags::NoArgument, ZyppFlags::BitFieldType( that->_flags, ListPackagesBits::ShowRecommended ),
            // translators: --recommended
            _("Show packages which are recommended.")
      },
      {"unneeded", '\0', ZyppFlags::NoArgument, ZyppFlags::BitFieldType( that->_flags, ListPackagesBits::ShowUnneeded ),
            // translators: --unneeded
            _("Show packages which are unneeded.")
      },
      {"sort-by-name", 'N', ZyppFlags::NoArgument, ZyppFlags::BitFieldType( that->_flags, ListPackagesBits::SortByRepo, ZyppFlags::StoreFalse ),
            // translators: -N, --sort-by-name
            _("Sort the list by package name.")
      },
      {"sort-by-repo", 'R', ZyppFlags::NoArgument, ZyppFlags::BitFieldType( that->_flags, ListPackagesBits::SortByRepo ),
            // translators: -R, --sort-by-repo
            _("Sort the list by repository.")
      },
      {"sort-by-catalog", '\0', ZyppFlags::NoArgument | ZyppFlags::Hidden, ZyppFlags::BitFieldType( that->_flags, ListPackagesBits::SortByRepo ), ""}
  }};
}

void PackagesCmdBase::doReset()
{
  _flags = ListPackagesBits::Default;
}

int PackagesCmdBase::execute( Zypper &zypp_r, const std::vector<std::string> & )
{
  ListPackagesFlags  flags = _flags;
  flags.setFlag( ListPackagesBits::HideInstalled, _notInstalledOnly._mode == SolvableFilterMode::ShowOnlyNotInstalled );
  flags.setFlag( ListPackagesBits::HideNotInstalled, _notInstalledOnly._mode == SolvableFilterMode::ShowOnlyInstalled );

  list_packages( zypp_r, flags );

  return ZYPPER_EXIT_OK;
}
