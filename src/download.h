/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_DOWNLOAD_H
#define ZYPPER_DOWNLOAD_H

#include "zypp/Pathname.h"

class Zypper;

/*
      "download [options] <packages>...\n"
      "\n"
      "Download rpms specified on the commandline to a local directory.\n"
      "Per default packages are downloaded to the libzypp package cache\n"
      "(/var/cache/zypp/packages), but this can be changed by using the\n"
      "global --pkg-cache-dir option.\n"
      "In XML output a <download-result> node is written for each\n"
      "package zypper tried to downlad. Upon success the local path is\n"
      "is found in 'download-result/localpath@path'.\n"
      "\n"
      "  Command options:\n"
      "--all-matches        Download all versions matching the commandline\n"
      "                     arguments. Otherwise only the best version of\n"
      "                     each matching package is downloaded.\n"
      "--dry-run            Don't download any package, just report waht\n"
      "                     would be done.\n"
*/

/** download specific options */
struct DownloadOptions : public Options
{
  static const Pathname _defaultDirectory;

  DownloadOptions()
    : _dryrun( false )
    , _allmatches( false )
  {}

  int _dryrun;		//< Dryrun mode.
  int _allmatches;	//< Download all matching packages, not just the best one
};

/** Download rpms specified on the commandline to a local directory.
 * \returns zypper.exitCode
 */
int download( Zypper & zypper_r );

#endif // ZYPPER_DOWNLOAD_H
