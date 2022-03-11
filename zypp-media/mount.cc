/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/Mount.cc
 *
*/

#include <mntent.h>

#include <cstdio>
#include <climits>
#include <cerrno>

#include <iostream>
#include <fstream>
#include <string>

#include <zypp-media/Mount>
#include <zypp/base/ExternalDataSource.h>
#include <zypp/base/Logger.h>
#include <zypp-media/MediaException>

#include <zypp/PathInfo.h>

using std::endl;

#ifndef N_
#define N_(STR) STR
#endif


namespace zypp {
  namespace media {

    std::ostream & operator<<( std::ostream & str, const MountEntry & obj )
    {
      str << obj.src << " on " << obj.dir << " type " << obj.type;
      if ( ! obj.opts.empty() )
        str << " (" << obj.opts << ")";
      return str;
    }


Mount::Mount()
{}

Mount::~Mount()
{}

void Mount::mount( const std::string & source,
                   const std::string & target,
                   const std::string & filesystem,
                   const std::string & options,
                   const Environment & environment )
{
  const char *const argv[] = {
    "/bin/mount",
    "-t", filesystem.c_str(),
    "-o", options.c_str(),
    source.c_str(),
    target.c_str(),
    NULL
  };

  std::string err;    // Error summary
  std::string value;  // legacy: Exception collects just the last output line
  ExternalProgram prog { argv, environment, ExternalProgram::Stderr_To_Stdout, false, -1, true };
  for ( std::string output = prog.receiveLine(); output.length(); output = prog.receiveLine() ) {
    output[output.size()-1] = '\0'; // clip tailing NL
    value = std::move( output );
    DBG << "stdout: " << value << endl;

    if ( value.find( "is already mounted on" ) != std::string::npos ) {
      err = "Media already mounted";
    }
    else if ( value.find( "ermission denied" ) != std::string::npos ) {
      err = "Permission denied";
    }
    else if ( value.find( "wrong fs type" ) != std::string::npos ) {
      err = "Invalid filesystem on media";
    }
    else if ( value.find( "No medium found" ) != std::string::npos ) {
      err = "No medium found";
    }
    else if ( value.find( "Not a directory" ) != std::string::npos ) {
      if ( filesystem == "nfs" || filesystem == "nfs4" ) {
        err = "NFS path is not a directory";
      }
      else {
        err = "Unable to find directory on the media";
      }
    }
  }
  int exitCode = prog.close();

  if ( exitCode != 0 ) {
    if ( err.empty() ) err = "Mounting media failed";
    WAR << "mount " << source << " " << target << ": " << exitCode << ": " << err << endl;
    ZYPP_THROW(MediaMountException(err, source, target, value));
  }
  else
    MIL << "mounted " << source << " " << target << endl;
}

void Mount::umount( const std::string & path )
{
  const char *const argv[] = {
    "/bin/umount",
    path.c_str(),
    NULL
  };

  std::string err;  // Error summary
  int exitCode = -1;

  bool doRetry = false;
  unsigned numRetry = 2;
  do {
    if ( doRetry ) {
      if ( --numRetry ) {
        WAR << "umount " << path << ": " << exitCode << ": " << err << " - retrying in 1 sec." << endl;
        sleep( 1 );
        err.clear();
        doRetry = false;
      }
      else {
        WAR << "umount " << path << ": " << exitCode << ": " << err << " - giving up" << endl;
        break;
      }
    }

    ExternalProgram prog { argv, ExternalProgram::Stderr_To_Stdout, false, -1, true };
    for ( std::string output = prog.receiveLine(); output.length(); output = prog.receiveLine() ) {
      output[output.size()-1] = '\0'; // clip tailing NL
      DBG << "stdout: " << output << endl;

      if  ( output.find ( "device is busy" ) != std::string::npos ) {
        err = "Device is busy";
        doRetry = true;
      }
    }
    exitCode = prog.close();

  } while( exitCode != 0 && doRetry );

  if ( exitCode != 0 ) {
    if ( err.empty() ) err = "Unmounting media failed";
    WAR << "umount " << path << ": " << exitCode << ": " << err << endl;
    ZYPP_THROW(MediaUnmountException(err, path));
  }
  else
    MIL << "unmounted " << path << endl;
}

// STATIC
MountEntries
Mount::getEntries(const std::string &mtab)
{
  MountEntries             entries;
  std::vector<std::string> mtabs;
  bool                     verbose = false;

  if( mtab.empty())
  {
    mtabs.push_back("/proc/mounts");
    // Also read /etc/mtab if it is a file (on newer sytems
    // mtab is a symlink to /proc/mounts).
    // Reason for this is the different representation of
    // mounted loop devices:
    //   /etc/mtab:    /tmp/SLES-11-SP2-MINI-ISO-x86_64-Beta2-DVD.iso on /mnt type iso9660 (ro,loop=/dev/loop0)
    //   /proc/mounts: /dev/loop0 /mnt iso9660 ro,relatime 0 0
    if ( PathInfo( "/etc/mtab", PathInfo::LSTAT ).isFile() )
      mtabs.push_back("/etc/mtab");
  }
  else
  {
    mtabs.push_back(mtab);
  }

  std::vector<std::string>::const_iterator t;
  for( t=mtabs.begin(); t != mtabs.end(); ++t)
  {
    if( verbose)
    {
      DBG << "Reading mount table from '" << *t << "'" << std::endl;
    }
    FILE *fp = setmntent(t->c_str(), "re");
    if( fp)
    {
      char          buf[PATH_MAX * 4];
      struct mntent ent;

      memset(buf,  0, sizeof(buf));
      memset(&ent, 0, sizeof(ent));

      while( getmntent_r(fp, &ent, buf, sizeof(buf)) != NULL)
      {
        if( ent.mnt_fsname && *ent.mnt_fsname &&
            ent.mnt_dir    && *ent.mnt_dir    &&
            ent.mnt_type   && *ent.mnt_type   &&
            ent.mnt_opts   && *ent.mnt_opts)
        {
          MountEntry entry(
            ent.mnt_fsname, ent.mnt_dir,
            ent.mnt_type,   ent.mnt_opts,
            ent.mnt_freq,   ent.mnt_passno
          );

          // Attempt quick fix for bnc#710269:
          // MountEntry is "//dist/install/ on /var/adm/mount/AP_0x00000001 type cifs (ro,relatime,unc=\dist\install,username=,domain=suse.de"
          // but looking for "Looking for media(cifs<//dist/install>)attached(*/var/adm/mount/AP_0x00000001)"
          // Kick the trailing '/' in "//dist/install/"
          // TODO: Check and fix comparison in MediaHandler::checkAttached instead.
          if ( entry.src.size() > 1	// not for "/"
               && entry.src[entry.src.size()-1] == '/' )
          {
            entry.src.erase( entry.src.size()-1 );
          }
          entries.push_back(entry);

          memset(buf,  0, sizeof(buf));
          memset(&ent, 0, sizeof(ent));
        }
      }
      endmntent(fp);

      if( entries.empty())
      {
        WAR << "Unable to read any entry from the mount table '" << *t << "'"
            << std::endl;
      }
      else
      {
        // OK, have a non-empty mount table.
        t = mtabs.end();
        break;
      }
    }
    else
    {
      int err = errno;
      verbose = true;
      WAR << "Failed to read the mount table '" << *t << "': "
          << ::strerror(err)
          << std::endl;
      errno = err;
    }
  }
  return entries;
}

  } // namespace media
} // namespace zypp
