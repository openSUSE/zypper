/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZYPPER_SOURCE_DOWNLOAD_H
#define ZYPPER_SOURCE_DOWNLOAD_H

#include "zypp/Pathname.h"

class Zypper;

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

/** source-download specific options */
struct SourceDownloadOptions : public Options
{
  static const Pathname _defaultDirectory;
  static const std::string _manifestName;

  SourceDownloadOptions()
    : _directory( _defaultDirectory )
//     , _manifest( true )
    , _delete( true )
    , _dryrun( false )
  {}

  Pathname _directory;	//< Download all source rpms to this directory.
//   int _manifest;	//< Whether to write a MANIFEST file.
  int _delete;		//< Whether to delete extranous source rpms.
  int _dryrun;		//< Dryrun mode.
};

/** Download source rpms for all installed packages to a local directory.
 * \returns zypper.exitCode
 */
int sourceDownload( Zypper & zypper_r );

#endif // ZYPPER_SOURCE_DOWNLOAD_H