/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/
#ifndef ZYPPER_COMMANDS_OPTIONSET_H_INCLUDED
#define ZYPPER_COMMANDS_OPTIONSET_H_INCLUDED

/**
 * \file contains all optionsets that do not otherwise belong to a group
 */

#include "basecommand.h"

#include <zypp/DownloadMode.h>
#include <zypp/base/Flags.h>

class DryRunOptionSet : public BaseCommandOptionSet
{
public:
 using BaseCommandOptionSet::BaseCommandOptionSet;

  // BaseCommandOptionSet interface
public:
  std::vector<zypp::ZyppFlags::CommandGroup> options() override;
  void reset() override;
};

class InitReposOptionSet : public BaseCommandOptionSet
{
public:

  enum class CompatModeBits {
    EnableRugOpt = 1 << 0,
    EnableNewOpt = 1 << 1
  };
  ZYPP_DECLARE_FLAGS( CompatModeFlags, CompatModeBits );

  using BaseCommandOptionSet::BaseCommandOptionSet;
  void setCompatibilityMode ( CompatModeFlags flags_r );

private:
  CompatModeFlags _compatMode = CompatModeBits::EnableNewOpt;

  // BaseCommandOptionSet interface
public:
  std::vector<zypp::ZyppFlags::CommandGroup> options() override;
  void reset() override;
};
ZYPP_DECLARE_OPERATORS_FOR_FLAGS( InitReposOptionSet::CompatModeFlags )

class DownloadOptionSet : public BaseCommandOptionSet
{
public:
 using BaseCommandOptionSet::BaseCommandOptionSet;

  zypp::DownloadMode mode() const;
  void setMode( const zypp::DownloadMode &mode );
  bool wasSetBefore () const;

private:
  zypp::DownloadMode _mode;
  bool _wasSetBefore = false;

  // BaseCommandOptionSet interface
public:
  std::vector<zypp::ZyppFlags::CommandGroup> options() override;
  void reset() override;
};

struct NotInstalledOnlyOptionSet : public BaseCommandOptionSet
{
public:
  using BaseCommandOptionSet::BaseCommandOptionSet;

  enum class Mode {
    Default,
    ShowOnlyInstalled,
    ShowOnlyNotInstalled
  };

  Mode _mode = Mode::Default;

  // BaseCommandOptionSet interface
public:
  std::vector<zypp::ZyppFlags::CommandGroup> options() override;
  void reset() override;
};
#endif
