/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_REPOS_REFRESH_H_INCLUDED
#define ZYPPER_COMMANDS_REPOS_REFRESH_H_INCLUDED

#include "commands/basecommand.h"

#include <zypp/base/Flags.h>

class RefreshRepoCmd : public ZypperBaseCommand
{
public:

  enum RefreshFlagsBits {
    Default       = 0,
    Force         = 1,
    ForceBuild    = 1 << 1,
    ForceDownload = 1 << 2,
    BuildOnly     = 1 << 3,
    DownloadOnly  = 1 << 4
  };
  ZYPP_DECLARE_FLAGS(RefreshFlags,RefreshFlagsBits);

  RefreshRepoCmd( const std::vector<std::string> &commandAliases_r );

  static int refreshRepositories ( Zypper &zypper, RefreshFlags flags_r = Default, const std::vector<std::string> repos_r = std::vector<std::string>() );

  /** \return false on success, true on error */
  static bool refreshRepository  ( Zypper & zypper, const zypp::RepoInfo & repo, RefreshFlags flags_r = Default );

  // ZypperBaseCommand interface
protected:
  std::vector<BaseCommandConditionPtr> conditions() const override;
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypper, const std::vector<std::string> &positionalArgs) override;


private:
  RefreshFlags _flags;
  std::vector<std::string> _repos;
  bool _services = false;
};
ZYPP_DECLARE_OPERATORS_FOR_FLAGS(RefreshRepoCmd::RefreshFlags);

#endif
