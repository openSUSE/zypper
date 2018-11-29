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
#include <zypp/base/Flags.h>

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

  unsigned _priority = 0U;
  zypp::TriBool _keepPackages = zypp::indeterminate;
  zypp::RepoInfo::GpgCheck _gpgCheck = zypp::RepoInfo::GpgCheck::indeterminate;
};

class RSCommonListOptions : public BaseCommandOptionSet
{
public:

  RSCommonListOptions( OptCommandCtx ctx );
  RSCommonListOptions( OptCommandCtx ctx, ZypperBaseCommand &parent );


  enum RSCommonListFlagsBits
  {
    ShowAlias          = 1,       //< only supported in list repos
    ShowName           = 1 << 1,  //< only supported in list repos
    ShowRefresh        = 1 << 2,  //< only supported in list repos
    ShowURI            = 1 << 3,  //< supported in service and repos
    ShowPriority       = 1 << 4,  //< supported in service and repos
    ShowWithRepos      = 1 << 5,  //< only supported in list services
    ShowWithService    = 1 << 6,  //< only supported in list repos
    ListServiceShowAll = ShowURI | ShowPriority | ShowWithRepos,
    ListRepoShowAll    = ShowAlias | ShowName | ShowRefresh | ShowURI | ShowPriority | ShowWithService,
    ShowEnabledOnly    = 1 << 7,  //< supported in service and repos
    SortByURI          = 1 << 8,  //< supported in service and repos
    SortByAlias        = 1 << 9,  //< only supported in list repos
    SortByName         = 1 << 10, //< supported in service and repos
    SortByPrio         = 1 << 11, //< supported in service and repos
    ServiceRepo        = 1 << 15  //< special value used in list services
  };
  ZYPP_DECLARE_FLAGS( RSCommonListFlags, RSCommonListFlagsBits );

  // BaseCommandOptionSet interface
  std::vector<zypp::ZyppFlags::CommandGroup> options() override;
  void reset() override;

  RSCommonListFlags _flags;

private:
  OptCommandCtx _cmdContext = OptCommandCtx::RepoContext;
};

ZYPP_DECLARE_OPERATORS_FOR_FLAGS( RSCommonListOptions::RSCommonListFlags )

#endif
