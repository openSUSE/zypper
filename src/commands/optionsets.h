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
#include "utils/misc.h"

#include <zypp/DownloadMode.h>
#include <zypp/base/Flags.h>

enum class CompatModeBits {
  EnableRugOpt = 1 << 0,
  EnableNewOpt = 1 << 1
};
ZYPP_DECLARE_FLAGS( CompatModeFlags, CompatModeBits );
ZYPP_DECLARE_OPERATORS_FOR_FLAGS( CompatModeFlags )

struct RugCompatModeMixin
{
  void setCompatibilityMode ( CompatModeFlags flags_r );

  protected:
    CompatModeFlags _compatMode = CompatModeBits::EnableNewOpt;
};

class DryRunOptionSet : public BaseCommandOptionSet, public RugCompatModeMixin
{
public:
 using BaseCommandOptionSet::BaseCommandOptionSet;

  // BaseCommandOptionSet interface
public:
  std::vector<zypp::ZyppFlags::CommandGroup> options() override;
  void reset() override;
};

class InitReposOptionSet : public BaseCommandOptionSet, public RugCompatModeMixin
{
public:
  using BaseCommandOptionSet::BaseCommandOptionSet;

  // BaseCommandOptionSet interface
public:
  std::vector<zypp::ZyppFlags::CommandGroup> options() override;
  void reset() override;
};

class DownloadOptionSet : public BaseCommandOptionSet
{
public:
  enum Mode {
    Default,
    SourceInstall
  };

  DownloadOptionSet( ZypperBaseCommand &parent, DownloadOptionSet::Mode cmdMode = DownloadOptionSet::Default );

  zypp::DownloadMode mode() const;
  void setMode( const zypp::DownloadMode &mode );
  bool wasSetBefore () const;

private:
  zypp::DownloadMode _mode;
  bool _wasSetBefore = false;
  Mode _cmdMode = Default;

  // BaseCommandOptionSet interface
public:
  std::vector<zypp::ZyppFlags::CommandGroup> options() override;
  void reset() override;
};

struct NotInstalledOnlyOptionSet : public BaseCommandOptionSet
{
public:
  using BaseCommandOptionSet::BaseCommandOptionSet;

  SolvableFilterMode _mode = SolvableFilterMode::ShowAll;

  // BaseCommandOptionSet interface
public:
  std::vector<zypp::ZyppFlags::CommandGroup> options() override;
  void reset() override;
};

class LicensePolicyOptionSet : public BaseCommandOptionSet, public RugCompatModeMixin
{
public:
 using BaseCommandOptionSet::BaseCommandOptionSet;

  // BaseCommandOptionSet interface
public:
  std::vector<zypp::ZyppFlags::CommandGroup> options() override;
  void reset() override;
};

class FileConflictPolicyOptionSet : public BaseCommandOptionSet
{
public:
 using BaseCommandOptionSet::BaseCommandOptionSet;

  // BaseCommandOptionSet interface
public:
  std::vector<ZyppFlags::CommandGroup> options() override;
  void reset() override;
};

/**
 * Provides pkg/apt/yum user convenience,
 * the no-confirm option is mapped to the global --non-interactive
 */
class NoConfirmRugOption : public BaseCommandOptionSet
{
public:
  using BaseCommandOptionSet::BaseCommandOptionSet;

  // BaseCommandOptionSet interface
public:
  std::vector<ZyppFlags::CommandGroup> options() override;
  void reset() override;
};

class OptionalPatchesOptionSet : public BaseCommandOptionSet
{
public:
  using BaseCommandOptionSet::BaseCommandOptionSet;

  // BaseCommandOptionSet interface
public:
  std::vector<ZyppFlags::CommandGroup> options() override;
  void reset() override;
};

class InteractiveUpdatesOptionSet : public BaseCommandOptionSet
{
public:
  using BaseCommandOptionSet::BaseCommandOptionSet;

  bool skipInteractive () const;

private:
  TriBool _withInteractive;

  // BaseCommandOptionSet interface
public:
  std::vector<ZyppFlags::CommandGroup> options() override;
  void reset() override;
};

class SortResultOptionSet : public BaseCommandOptionSet, public RugCompatModeMixin
{
public:
  using BaseCommandOptionSet::BaseCommandOptionSet;

  enum SortMode {
    Default, //<! Option was never set
    ByName,  //<! Sort result by name
    ByRepo   //<! Sort result by repo
  };
  SortMode _mode = Default;

  // BaseCommandOptionSet interface
public:
  std::vector<ZyppFlags::CommandGroup> options() override;
  void reset() override;
};


#endif
