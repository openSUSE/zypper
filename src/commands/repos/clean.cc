/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#include "clean.h"

#include "commands/conditions.h"
#include "utils/flags/flagtypes.h"
#include "Zypper.h"

CleanRepoCmd::CleanRepoCmd( const std::vector<std::string> &commandAliases_r ):
  ZypperBaseCommand(
    commandAliases_r,
    _("clean (cc) [ALIAS|#|URI] ..."),
    _("Clean local caches."),
    _("Clean local caches."),
    ResetRepoManager )
{ }

std::vector<BaseCommandConditionPtr> CleanRepoCmd::conditions() const
{
  return {
    std::make_shared<NeedsRootCondition>()
  };
}

zypp::ZyppFlags::CommandGroup CleanRepoCmd::cmdOptions() const
{
  auto that = const_cast<CleanRepoCmd *>(this);
  return {{
      {
        "repo", 'r', ZyppFlags::RequiredArgument | ZyppFlags::Repeatable,
            ZyppFlags::StringVectorType( &that->_repos, ARG_REPOSITORY),
            // translators: -r, --repo <ALIAS|#|URI>
            _("Clean only specified repositories.")
      },{
        "metadata", 'm', ZyppFlags::NoArgument,
            ZyppFlags::BitFieldType ( that->_flags, CleanRepoBits::CleanMetaData),
            // translators: -m, --metadata
            _("Clean metadata cache.")

      },{
        "raw-metadata", 'M', ZyppFlags::NoArgument,
            ZyppFlags::BitFieldType ( that->_flags, CleanRepoBits::CleanRawMetaData),
            // translators: -M, --raw-metadata
            _("Clean raw metadata cache.")
      },{
        "all", 'a', ZyppFlags::NoArgument,
            ZyppFlags::BitFieldType ( that->_flags, CleanRepoBits::CleanAll),
            // translators: -a, --all
            _("Clean both metadata and package caches.")
      }
  }};
}

void CleanRepoCmd::doReset()
{
  _repos.clear();
  _flags = CleanRepoBits::Default;
}

int CleanRepoCmd::execute( Zypper &zypp_r, const std::vector<std::string> &positionalArgs_r )
{
  // get the list of repos specified on the command line ...
  std::vector<std::string> specifiedRepos = _repos;
  for ( const std::string &repoFromCLI : positionalArgs_r )
    specifiedRepos.push_back(repoFromCLI);

  clean_repos( zypp_r,  specifiedRepos, _flags );

  return zypp_r.exitCode();
}
