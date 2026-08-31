/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include "configcheck.h"
#include "commands/conditions.h"
#include "commands/commonflags.h"
#include "utils/flags/flagtypes.h"
#include "Zypper.h"

#include <zypp-core/fs/PathInfo.h>
#include <zypp-core/ExternalProgram.h>
#include <zypp-core/base/String.h>
#include <zypp-core/base/Logger.h>
#include <cerrno>

using namespace zypp;

std::vector<std::string> ConfigCheck::getSystemConfigFiles() const
{
  std::vector<std::string> paths;
  const char *argv[] = { "rpm", "-qa", "--configfiles", nullptr };
  
  zypp::ExternalProgram prog(argv, zypp::ExternalProgram::Discard_Stderr);
  for (std::string line = prog.receiveLine(); !line.empty(); line = prog.receiveLine()) {
    line = zypp::str::trim(line);
    if (!line.empty() && line.front() == '/') {
      paths.push_back(line);
    }
  }
  return paths;
}

bool ConfigCheck::run(Zypper &zypper, const std::vector<std::string> &configFiles)
{
  std::vector<std::string> foundFiles;
  bool hadError = false;

  for (const auto& path : configFiles) {
    for (const char* ext : { ".rpmnew", ".rpmsave" }) {
      std::string testPath = path + ext;
      zypp::filesystem::PathInfo info(testPath);
      if (info.isExist()) {
        foundFiles.push_back(testPath);
      } else if (info.error() == EACCES) {
        zypper.out().warning(str::form(_("Missing permissions to read file/directory: %s"), testPath.c_str()));
        hadError = true;
      }
    }
  }

  if (!foundFiles.empty() > 0) {
    zypper.out().info("Orphaned configuration files:");
  }

  for (const auto& f : foundFiles) {
    zypper.out().info(f);
  }

  return !hadError;
}

ConfigCheckCmd::ConfigCheckCmd(std::vector<std::string> &&commandAliases_r) :
  ZypperBaseCommand(
    std::move( commandAliases_r ),
    // translators: command synopsis; do not translate lowercase words
    _("configcheck"),
    // translators: command summary
    _("Check for .rpmnew and .rpmsave configuration files."),
    // translators: command description
    _("Queries the RPM database for all known configuration files and checks the filesystem for the existence of .rpmnew and .rpmsave files."),
    ResetRepoManager | InitTarget
  )
{ }

zypp::ZyppFlags::CommandGroup ConfigCheckCmd::cmdOptions() const
{
  return {};
}

void ConfigCheckCmd::doReset()
{
}

int ConfigCheckCmd::execute( Zypper &zypper , const std::vector<std::string> & )
{
  ConfigCheck checker;
  try {
    std::vector<std::string> files = checker.getSystemConfigFiles();
    if (!checker.run(zypper, files)) {
      return ZYPPER_EXIT_ERR_PRIVILEGES;
    }
  } catch (const Exception &e) {
    zypper.out().error(e, _("Failed to initialize target or query RPM database."));
    return ZYPPER_EXIT_ERR_ZYPP;
  }
  
  return ZYPPER_EXIT_OK;
}

std::vector<BaseCommandConditionPtr> ConfigCheckCmd::conditions() const
{
  return {};
}
