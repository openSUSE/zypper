/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                     (C) 2002 SuSE AG |
\----------------------------------------------------------------------/

   File:       Wget.h
   Purpose:    Declare interface to mount program
   Author:     Ludwig Nussel <lnussel@suse.de>
   Maintainer: Ludwig Nussel <lnussel@suse.de>

/-*/


// -*- C++ -*-

#ifndef ZYPP_MEDIA_MOUNT_H
#define ZYPP_MEDIA_MOUNT_H
#include <set>
#include <map>
#include <string>

#include "zypp/ExternalProgram.h"

namespace zypp {
  namespace media {

    /**
     * @short Interface to the mount program
     */
    class Mount
    {
    public:

	/**
	 * For passing additional environment variables
	 * to mount
	 **/
	typedef ExternalProgram::Environment Environment;

	/**
	 * Mount options. 'key' or 'key=value' pairs, separated by ','
	 **/
#warning Uncomment Options type if it is needed
#if 0
	typedef KVMap<_KVMap::CharSep<'=',','> > Options;
#endif

    public:

	/**
	* Create an new instance.
	*/
	Mount();

	/**
	* Clean up.
	*/
	~Mount();

	/**
	* mount device
	*
	* @param source what to mount (e.g. /dev/hda3)
	* @param target where to mount (e.g. /mnt)
	* @param filesystem which filesystem to use (e.g. reiserfs) (-t parameter)
	* @param options mount options (e.g. ro) (-o parameter)
	* @param environment optinal environment to pass (e.g. PASSWD="sennah")
        *
        * \throws MediaException
        *
	*/

	void mount ( const std::string& source,
			const std::string& target,
			const std::string& filesystem,
			const std::string& options,
			const Environment& environment = Environment() );

	/** umount device
	 *
	 * @param path device or mountpoint to umount
        *
        * \throws MediaException
        *
	 * */
	void umount (const std::string& path);

    private:

	/** The connection to the mount process.
	 * */
	ExternalProgram *process;

	/**
	 * Run mount with the specified arguments and handle stderr.
	 * @param argv Mount arguments
	 * @param environment Addittional environment to set
	 * @param stderr_disp How to handle stderr, merged with stdout by default
	 * */
	void run( const char *const *argv, const Environment& environment,
		  ExternalProgram::Stderr_Disposition stderr_disp =
		  ExternalProgram::Stderr_To_Stdout);

	void run( const char *const *argv,
		  ExternalProgram::Stderr_Disposition stderr_disp =
		  ExternalProgram::Stderr_To_Stdout) {
	  Environment notused;
	  run( argv, notused, stderr_disp );
	}

	/** Return the exit status of the process, closing the connection if
	 * not already done.
	 * */
	int Status();

	/** Forcably kill the process
	 * */
	void Kill();


	/** The exit code of the process, or -1 if not yet known.
	 * */
	int exit_code;
    };

  } // namespace media
} // namespace zypp

#endif
