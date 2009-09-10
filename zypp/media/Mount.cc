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

#include "zypp/base/ExternalDataSource.h"
#include "zypp/base/Logger.h"
#include "zypp/media/Mount.h"
#include "zypp/media/MediaException.h"

#ifndef N_
#define N_(STR) STR
#endif

using namespace std;

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
{
    process = 0;
    exit_code = -1;
}

Mount::~Mount()
{
   MIL <<  "~Mount()" << endl;

   if ( process )
      delete process;

   process = NULL;

   MIL << "~Mount() end" << endl;
}

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

    std::string err;

    this->run(argv, environment, ExternalProgram::Stderr_To_Stdout);

    if ( process == NULL )
    {
      ZYPP_THROW(MediaMountException("Mounting media failed", source, target));
    }

    string value;
    string output = process->receiveLine();

    // parse error messages
    while ( output.length() > 0)
    {
	string::size_type 	ret;

	// extract \n
	ret = output.find_first_of ( "\n" );
	if ( ret != string::npos )
	{
	    value.assign ( output, 0, ret );
	}
	else
	{
	    value = output;
	}

	DBG << "stdout: " << value << endl;

	if  ( value.find ( "is already mounted on" ) != string::npos )
	{
	    err = "Media already mounted";
	}
	else if  ( value.find ( "ermission denied" ) != string::npos )
	{
	    err = "Permission denied";
	}
	else if  ( value.find ( "wrong fs type" ) != string::npos )
	{
	    err = "Invalid filesystem on media";
	}
	else if  ( value.find ( "No medium found" ) != string::npos )
	{
	    err = "No medium found";
	}
	else if  ( value.find ( "Not a directory" ) != string::npos )
	{
	    if( filesystem == "nfs" || filesystem == "nfs4" )
	    {
		err = "Nfs path is not a directory";
	    }
	    else
	    {
	       err = "Unable to find directory on the media";
	    }
	}

	output = process->receiveLine();
    }

    int status = Status();

    if ( status == 0 )
    {
	// return codes overwites parsed error message
	err = "";
    }
    else if ( status != 0 && err == "" )
    {
        err = "Mounting media failed";
    }

    if ( err != "" ) {
      WAR << "mount " << source << " " << target << ": " << err << endl;
      ZYPP_THROW(MediaMountException(err, source, target, value));
    } else {
      MIL << "mounted " << source << " " << target << endl;
    }
}

void Mount::umount( const std::string & path )
{
    const char *const argv[] = {
	"/bin/umount",
	path.c_str(),
	NULL
     };

    std::string err;

    this->run(argv, ExternalProgram::Stderr_To_Stdout);

    if ( process == NULL )
    {
        ZYPP_THROW(MediaUnmountException("E_mount_failed", path));
    }

    string value;
    string output = process->receiveLine();

    // parse error messages
    while ( output.length() > 0)
    {
	string::size_type 	ret;

	// extract \n
	ret = output.find_first_of ( "\n" );
	if ( ret != string::npos )
	{
	    value.assign ( output, 0, ret );
	}
	else
	{
	    value = output;
	}

	DBG << "stdout: " << value << endl;

	// if  ( value.find ( "not mounted" ) != string::npos )
	// {
	//    err = Error::E_already_mounted;
	// }

	if  ( value.find ( "device is busy" ) != string::npos )
	{
	    err = "Device is busy";
	}

	output = process->receiveLine();
    }

    int status = Status();

    if ( status == 0 )
    {
	// return codes overwites parsed error message
	err = "";
    }
    else if ( status != 0 && err == "" )
    {
	err = "Unmounting media failed";
    }

    if ( err != "") {
      WAR << "umount " << path << ": " << err << endl;
      ZYPP_THROW(MediaUnmountException(err, path));
    } else {
      MIL << "unmounted " << path << endl;
    }
}

void Mount::run( const char *const *argv, const Environment& environment,
		 ExternalProgram::Stderr_Disposition disp )
{
  exit_code = -1;

  if ( process != NULL )
  {
     delete process;
     process = NULL;
  }
  // Launch the program

  process = new ExternalProgram(argv, environment, disp, false, -1, true);
}

/*--------------------------------------------------------------*/
/* Return the exit status of the Mount process, closing the	*/
/* connection if not already done				*/
/*--------------------------------------------------------------*/
int Mount::Status()
{
   if ( process == NULL )
      return -1;

   exit_code = process->close();
   process->kill();
   delete process;
   process = 0;

   DBG << "exit code: " << exit_code << endl;

   return exit_code;
}

/* Forcably kill the process */
void Mount::Kill()
{
  if (process) process->kill();
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
    mtabs.push_back("/etc/mtab");
    mtabs.push_back("/proc/mounts");
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
    FILE *fp = setmntent(t->c_str(), "r");
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
