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

#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
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

void Mount::mount ( const string& source,
		       const string& target,
		       const string& filesystem,
		       const string& options,
		       const Environment& environment )
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
      ZYPP_THROW(MediaMountException(source, target, "Error::E_mount_failed"));
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
	    err = "Error::E_already_mounted";
	}
	else if  ( value.find ( "ermission denied" ) != string::npos )
	{
	    err = "Error::E_no_permission";
	}
	else if  ( value.find ( "wrong fs type" ) != string::npos )
	{
	    err = "Error::E_invalid_filesystem";
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
        err = "Error::E_mount_failed";
    }

    if ( err != "" ) {
      WAR << "mount " << source << " " << target << ": " << err << endl;
      ZYPP_THROW(MediaMountException(source, target, err));
    } else {
      MIL << "mounted " << source << " " << target << endl;
    }
}

void Mount::umount (const string& path)
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
	    err = "Error::E_busy";
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
	err = "Error::E_umount_failed";
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

  } // namespace media
} // namespace zypp
