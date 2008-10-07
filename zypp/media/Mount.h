/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/Mount.h
 *
*/

// -*- C++ -*-

#ifndef ZYPP_MEDIA_MOUNT_H
#define ZYPP_MEDIA_MOUNT_H

#include <set>
#include <map>
#include <string>
#include <iosfwd>

#include "zypp/ExternalProgram.h"
#include "zypp/KVMap.h"

namespace zypp {
  namespace media {


    /**
     * A "struct mntent" like mount entry structure,
     * but using std::strings.
     */
    struct MountEntry
    {
        MountEntry(const std::string &source,
                   const std::string &target,
                   const std::string &fstype,
                   const std::string &options,
                   const int         dumpfreq = 0,
                   const int         passnum  = 0)
            : src(source)
            , dir(target)
            , type(fstype)
            , opts(options)
            , freq(dumpfreq)
            , pass(passnum)
        {}

        std::string src;  //!< name of mounted file system
        std::string dir;  //!< file system path prefix
        std::string type; //!< filesystem / mount type
        std::string opts; //!< mount options
        int         freq; //!< dump frequency in days
        int         pass; //!< pass number on parallel fsck
    };

    /** \relates MountEntry
     * A vector of mount entries.
     */
    typedef std::vector<MountEntry> MountEntries;

    /** \relates MountEntry Stream output */
    std::ostream & operator<<( std::ostream & str, const MountEntry & obj );

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
	typedef KVMap<kvmap::KVMapBase::CharSep<'=',','> > Options;

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

    public:

	/**
	* Return mount entries from /etc/mtab or /etc/fstab file.
	*
	* @param mtab The name of the (mounted) file system description
	*             file to read from. This file should be one /etc/mtab,
	*             /etc/fstab or /proc/mounts. Default is to try the
	*             /etc/mtab and fail back to /proc/mounts.
	* @returns A vector with mount entries or empty vector if reading
	*          or parsing of the mtab file(s) failed.
	*/
	static MountEntries
	getEntries(const std::string &mtab = "");

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
