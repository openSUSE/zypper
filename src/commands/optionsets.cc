/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "optionsets.h"
#include "utils/flags/flagtypes.h"
#include "global-settings.h"

std::vector<ZyppFlags::CommandGroup> DryRunOptionSet::options()
{
  return {{{
    { "dry-run", 'D', ZyppFlags::NoArgument, ZyppFlags::BoolType( &DryRunSettings::instanceNoConst()._enabled, ZyppFlags::StoreTrue, DryRunSettings::instance()._enabled ),
         _("Don't change anything, just report what would be done.")}
  }}};
}

void DryRunOptionSet::reset()
{
  DryRunSettings::reset();
}

void InitReposOptionSet::setEnableRugCompatibility(bool set)
{
  _isRugCmd = set;
}

std::vector<ZyppFlags::CommandGroup> InitReposOptionSet::options()
{
  if ( _isRugCmd ) {
    return {{{
          { "catalog", 'c',
                ZyppFlags::RequiredArgument | ZyppFlags::Repeatable,
                ZyppFlags::StringVectorType( &InitRepoSettings::instanceNoConst()._repoFilter, "ALIAS|#|URI" ),
                ""
          }
    }}};
  }
  return {{{
        { "repo", 'r',
              ZyppFlags::RequiredArgument | ZyppFlags::Repeatable,
              ZyppFlags::StringVectorType( &InitRepoSettings::instanceNoConst()._repoFilter, "ALIAS|#|URI" ),
              // translators: -r, --repo <ALIAS|#|URI>
              _("Work only with the specified repository.")
        }
  }}};
}

void InitReposOptionSet::reset()
{
  InitRepoSettings::reset();
}
