/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "packages.h"
#include "Zypper.h"
#include "utils/flags/flagtypes.h"
#include "commands/commonflags.h"
#include "global-settings.h"

using namespace zypp;

PackagesCmdBase::PackagesCmdBase(std::vector<std::string> &&commandAliases_r) :
  ZypperBaseCommand (
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate lowercase words
    _("packages (pa) [OPTIONS] [REPOSITORY] ..."),
    // translators: command summary: packages, pa
    _("List all available packages."),
    // translators: command description
    _("List all packages available in specified repositories."),
    DisableAll
  )
{
  _initRepoFlags.setCompatibilityMode( CompatModeBits::EnableRugOpt | CompatModeBits::EnableNewOpt );
}

zypp::ZyppFlags::CommandGroup PackagesCmdBase::cmdOptions() const
{
  auto that = const_cast<PackagesCmdBase *>(this);
  return {{
      {"autoinstalled", '\0', ZyppFlags::NoArgument, ZyppFlags::BitFieldType( that->_flags, ListPackagesBits::ShowByAuto ),
            // translators: --autoinstalled
            _("Show installed packages which were automatically selected by the resolver.")
      },
      {"userinstalled", '\0', ZyppFlags::NoArgument, ZyppFlags::BitFieldType( that->_flags, ListPackagesBits::ShowByUser ),
            // translators: --userinstalled
            _("Show installed packages which were explicitly selected by the user.")
      },
      {"system", '\0', ZyppFlags::NoArgument, ZyppFlags::BitFieldType( that->_flags, ListPackagesBits::ShowSystem ),
            // translators: --system
            _("Show installed packages which are not provided by any repository.")
      },
      {"orphaned", '\0', ZyppFlags::NoArgument, ZyppFlags::BitFieldType( that->_flags, ListPackagesBits::ShowOrphaned ),
            // translators: --orphaned
            _("Show system packages which are orphaned (without repository and without update candidate).")
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
      CommonFlags::idsOnlyFlag( that->_idsOnly, ResKind::package ),
      {"sort-by-catalog", '\0', ZyppFlags::NoArgument | ZyppFlags::Hidden, ZyppFlags::BitFieldType( that->_flags, ListPackagesBits::SortByRepo ), ""}
  }};
}

void PackagesCmdBase::doReset()
{
  _flags = ListPackagesBits::Default;
  _idsOnly = false;
}

int PackagesCmdBase::execute( Zypper &zypper, const std::vector<std::string> & )
{
  ListPackagesFlags  flags = _flags;
  flags.setFlag( ListPackagesBits::HideInstalled, _notInstalledOnly._mode == SolvableFilterMode::ShowOnlyNotInstalled );
  flags.setFlag( ListPackagesBits::HideNotInstalled, _notInstalledOnly._mode == SolvableFilterMode::ShowOnlyInstalled );
  flags.setFlag( ListPackagesBits::IdsOnly, _idsOnly );

  list_packages( zypper, flags );

  return ZYPPER_EXIT_OK;
}

