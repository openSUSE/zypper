/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_REPO_SERVICE_OPTIONSET_H_INCLUDED
#define ZYPPER_COMMANDS_REPO_SERVICE_OPTIONSET_H_INCLUDED

#include "basecommand.h"

#include <zypp/TriBool.h>
#include <zypp/RepoInfo.h>

enum class OptCommandCtx
{
  ServiceContext,
  RepoContext
};

// Common Repo/Service properties (argdef only)
// LEGACY: --refresh short option was -f in ADD_REPO, -r in all other Repo/Service commands.
//         Unfortunately -r is already --repo in ADD_REPO, so switching all Repo/Service commands
//         to prefer -f/F.
class RepoServiceCommonOptions : public BaseCommandOptionSet
{
public:
  RepoServiceCommonOptions( OptCommandCtx ctx );
  RepoServiceCommonOptions( OptCommandCtx ctx, ZypperBaseCommand &parent );

  // BaseCommandOptionSet interface
  std::vector<zypp::ZyppFlags::CommandGroup> options() override;
  void reset() override;

  //will be removed
  void fillFromCopts (Zypper &zypper );

  std::string _name;
  zypp::TriBool _enable = zypp::indeterminate;
  zypp::TriBool _enableAutoRefresh = zypp::indeterminate;

private:
  OptCommandCtx _cmdContext = OptCommandCtx::RepoContext;
};

// Common modify Repo/Service aggregate options (argdef only)
class RepoServiceCommonSelectOptions : public BaseCommandOptionSet
{
public:
  RepoServiceCommonSelectOptions( OptCommandCtx ctx );
  RepoServiceCommonSelectOptions( OptCommandCtx ctx, ZypperBaseCommand &parent );

  // BaseCommandOptionSet interface
  std::vector<zypp::ZyppFlags::CommandGroup> options() override;
  void reset() override;

  //will be removed
  void fillFromCopts (Zypper &zypper );

  bool _all    = false;
  bool _local  = false;
  bool _remote = false;
  std::vector<std::string> _mediumTypes;

private:
  OptCommandCtx _cmdContext = OptCommandCtx::RepoContext;
};

class RepoProperties : public BaseCommandOptionSet
{
public:
  using BaseCommandOptionSet::BaseCommandOptionSet;

  // BaseCommandOptionSet interface
  std::vector<zypp::ZyppFlags::CommandGroup> options() override;
  void reset() override;

  //will be removed
  void fillFromCopts (Zypper &zypper );

  unsigned _priority = 0U;
  zypp::TriBool _keepPackages = zypp::indeterminate;
  zypp::RepoInfo::GpgCheck _gpgCheck = zypp::RepoInfo::GpgCheck::indeterminate;
};


#endif
