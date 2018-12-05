/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "solveroptionset.h"
#include "utils/flags/flagtypes.h"


std::vector<zypp::ZyppFlags::CommandGroup> SolverRecommendsOptionSet::options()
{
  auto &set = SolverSettings::instanceNoConst();
  return {
    {
      _("Solver options"),
      {
        { "recommends", '\0', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( set._recommends, ZyppFlags::StoreTrue ), _("Install also recommended packages in addition to the required ones.") },
        { "no-recommends", '\0', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( set._recommends, ZyppFlags::StoreFalse ), _("Do not install recommended packages, only required ones.") }
      },
      {
        //conflicting flags
        { "recommends" , "no-recommends" }
      }
    }};
}

void SolverRecommendsOptionSet::reset()
{
  //reset all solver settings
  SolverSettings::reset();
}

std::vector<zypp::ZyppFlags::CommandGroup> SolverCommonOptionSet::options()
{
  auto &set = SolverSettings::instanceNoConst();
  return {
    {
      _("Solver options"),
      {
        { "debug-solver", '\0', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( set._debugSolver, ZyppFlags::StoreTrue ), _("Create a solver test case for debugging.") },
        { "force-resolution", '\0', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( set._forceResolution, ZyppFlags::StoreTrue ), _("Force the solver to find a solution (even an aggressive one) rather than asking.") },
        { "no-force-resolution", 'R', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( set._forceResolution, ZyppFlags::StoreFalse ), _("Do not force the solver to find solution, let it ask.") }
      },
      {
        //conflicting flags
        { "force-resolution" , "no-force-resolution" }
      }
    }};
}

void SolverCommonOptionSet::reset()
{
  //reset all solver settings
  SolverSettings::reset();
}

std::vector<zypp::ZyppFlags::CommandGroup> SolverInstallsOptionSet::options()
{
  auto &set = SolverSettings::instanceNoConst();
  return {
    {
      _("Expert options"),
      {
        { "allow-downgrade", '\0', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( set._allowDowngrade, ZyppFlags::StoreTrue ) },
        { "no-allow-downgrade", '\0', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( set._allowDowngrade, ZyppFlags::StoreFalse ), _("Whether to allow downgrading installed resolvables.") },
        { "allow-name-change", '\0', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( set._allowNameChange, ZyppFlags::StoreTrue ) },
        { "no-allow-name-change", '\0', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( set._allowNameChange, ZyppFlags::StoreFalse ), _("Whether to allow changing the names of installed resolvables.") },
        { "allow-arch-change", '\0', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( set._allowArchChange, ZyppFlags::StoreTrue ) },
        { "no-allow-arch-change", '\0', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( set._allowArchChange, ZyppFlags::StoreFalse ), _("Whether to allow changing the architecture of installed resolvables.") },
        { "allow-vendor-change", '\0', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( set._allowVendorChange, ZyppFlags::StoreTrue ) },
        { "no-allow-vendor-change", '\0', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( set._allowVendorChange, ZyppFlags::StoreFalse ), _("Whether to allow changing the vendor of installed resolvables.") }
      },
      {
        //conflicting flags
        { "allow-downgrade" , "no-allow-downgrade" },
        { "allow-name-change", "no-allow-name-change" },
        { "allow-arch-change", "no-allow-arch-change" },
        { "allow-vendor-change", "no-allow-vendor-change" }
      }
    }};
}

void SolverInstallsOptionSet::reset()
{
  //reset all solver settings
  SolverSettings::reset();
}

std::vector<ZyppFlags::CommandGroup> SolverCleanDepsOptionSet::options()
{
  auto &set = SolverSettings::instanceNoConst();
  return {{{
        { "clean-deps", 'u', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( set._cleanDeps, ZyppFlags::StoreTrue, set._cleanDeps ),
          // translators: -u, --clean-deps
          _("Automatically remove unneeded dependencies.")
        },
        { "no-clean-deps", 'U', ZyppFlags::NoArgument, ZyppFlags::TriBoolType( set._cleanDeps, ZyppFlags::StoreFalse, !set._cleanDeps ),
          // translators: -U, --no-clean-deps
          _("No automatic removal of unneeded dependencies.")
        }
  }}};
}

void SolverCleanDepsOptionSet::reset()
{
  //reset all solver settings
  SolverSettings::reset();
}
