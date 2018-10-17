/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_COMMANDS_UTILS_DOWNLOAD_INCLUDED
#define ZYPPER_COMMANDS_UTILS_DOWNLOAD_INCLUDED

/*
      "download [options] <packages>...\n"
      "\n"
      "Download rpms specified on the commandline to a local directory.\n"
      "Per default packages are downloaded to the libzypp package cache\n"
      "(/var/cache/zypp/packages), but this can be changed by using the\n"
      "global --pkg-cache-dir option.\n"
      "In XML output a <download-result> node is written for each\n"
      "package zypper tried to download. Upon success the local path is\n"
      "is found in 'download-result/localpath@path'.\n"
      "\n"
      "  Command options:\n"
      "--all-matches        Download all versions matching the commandline\n"
      "                     arguments. Otherwise only the best version of\n"
      "                     each matching package is downloaded.\n"
      "--dry-run            Don't download any package, just report what\n"
      "                     would be done.\n"
*/

#include "commands/basecommand.h"
#include "commands/optionsets.h"
#include "utils/flags/zyppflags.h"

class Zypper;

/**
 * Download rpms specified on the commandline to a local directory.
 */
class DownloadCmd : public ZypperBaseCommand
{
public:
  DownloadCmd ( const std::vector<std::string> &commandAliases_r );

private:
  DryRunOptionSet _dryRun { *this };
  bool _allMatches = false;


  // ZypperBaseCommand interface
protected:
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;
  int execute(Zypper &zypp_r, const std::vector<std::string> &positionalArgs_r) override;
  std::vector<BaseCommandConditionPtr> conditions() const override;
};

#endif // ZYPPER_DOWNLOAD_H
