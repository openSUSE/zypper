/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_SOURCE_DOWNLOAD_H
#define ZYPPER_SOURCE_DOWNLOAD_H

#include "zypp/Pathname.h"
#include "commands/basecommand.h"
//#include "utils/flags/zyppflags.h"

/*
      "source-download\n"
      "\n"
      "Download source rpms for all installed packages to a local directory.\n"
      "\n"
      "  Command options:\n"
      "-d, --directory <dir>\n"
      "                     Download all source rpms to this directory.\n"
      "                     Default: /var/cache/zypper/source-download\n"
      "--manifest           Write MANIFEST of packages and coresponding source rpms.\n"
      "--no-manifest        Do not write MANIFEST.\n"
      "--delete             Delete extraneous source rpms in the local directory.\n"
      "--no-delete          Do not delete extraneous source rpms.\n"
      "--dry-run            Don't download any source rpms nor write a MANIFEST,\n"
      "                     but show which source rpms are missing or extraneous.\n"

      TBD: maybe write manifest file to download directory.
*/

namespace Pimpl {
class SourceDownloadImpl;
}


/**
 * Download source rpms for all installed packages to a local directory.
 */
class SourceDownloadCmd : public ZypperBaseCommand
{
public:
  SourceDownloadCmd ( const std::vector<std::string> &commandAliases_r );

  struct Options {
    static const zypp::filesystem::Pathname _defaultDirectory;
    static const std::string _manifestName;

    zypp::filesystem::Pathname _directory;  //< Download all source rpms to this directory.
  //   bool _manifest;                      //< Whether to write a MANIFEST file.
    bool _delete = true;                    //< Whether to delete extranous source rpms.
    bool _dryrun = false;                   //< Dryrun mode.
  };

  friend class Pimpl::SourceDownloadImpl;

private:
  Options _opt;

  // ZypperBaseCommand interface
protected:
  zypp::ZyppFlags::CommandGroup cmdOptions() const override;
  void doReset() override;

  int execute(Zypper &zypp_r, const std::vector<std::string> &positionalArgs_r) override;
};

#endif // ZYPPER_SOURCE_DOWNLOAD_H
