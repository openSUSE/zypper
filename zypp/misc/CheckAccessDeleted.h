/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/misc/CheckAccessDeleted.h
 *
*/
#ifndef ZYPP_MISC_CHECKACCESSDELETED_H
#define ZYPP_MISC_CHECKACCESSDELETED_H

#include <iosfwd>
#include <vector>
#include <string>
#include <zypp/Pathname.h>
#include <zypp/base/PtrTypes.h>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  /**
   * Check for running processes which access deleted executables or libraries.
   *
   * Executed after commit, this gives a hint which processes/services
   * need to be restarted.
   *
   * Per default upon construction or explicit call to \ref check,
   * information about running processes which access deleted files
   * or libraries is collected and provided as a \ref ProcInfo
   * container.
   *
   * Provides support for reproducing check results from a foreign system by
   * creating a debug output file containing all required information,
   * enabled by \ref setDebugOutputFile.\n
   * This data file can be used as datasource when passed to \ref check(const Pathname &, bool).
   */
  class CheckAccessDeleted
  {

    public:
      class Impl;
      /**
       * Data about one running process accessing deleted files.
       */
      struct ProcInfo
      {
        std::string pid;		//!< process ID
        std::string ppid;		//!< parent process ID
        std::string puid;		//!< process user ID
        std::string login;		//!< process login name
        std::string command;		//!< process command name
        std::vector<std::string> files;	//!< list of deleted executables or libraries accessed

        /** Guess if command was started by a systemd service script.
         * The service name  might be used to restart the service.
         * \warning This is just a guess.
        */
        std::string service() const;
      };

      typedef size_t					size_type;
      typedef ProcInfo					value_type;
      typedef std::vector<ProcInfo>::const_iterator	const_iterator;

    public:
      /** Default ctor performs check immediately.
       * Pass \c false and the initial check is omitted.
       * \throws Exception if \ref check throws.
       * \see \ref check.
       */
      CheckAccessDeleted( bool doCheck_r = true );

    public:
      /** Check for running processes which access deleted executables or libraries.
       *
       * Per default \ref check will try guess and collect executables and
       * libraries only by looking at the files path and name. (e.g named
       * \c lib* or located in \c *bin/).
       *
       * A verbose check will omit this test and collect all processes using
       * any deleted file.
       *
       * \return the number of processes found.
       * \throws Exception On error collecting the data (e.g. no lsof installed)
       */
      size_type check( bool verbose_r = false );

      /**
       * \overload
       * Performs the same checks but instead of investigating the current system it
       * uses information from \a lsofOutput_r to support debugging.
       *
       * \sa setDebugOutputFile
       */
      size_type check( const Pathname &lsofOutput_r, bool verbose_r = false );

      bool empty() const;
      size_type size() const;
      const_iterator begin() const;
      const_iterator end() const;

      /**
       * Writes all filtered process entries that make it into the final set into
       * a file specified by \a filename_r.
       */
      void setDebugOutputFile (const Pathname &filename_r);

    public:
      /** Guess if pid was started by a systemd service script.
       * The service name  might be used to restart the service.
       * \warning This is just a guess.
       */
      static std::string findService( pid_t pid_r );
  private:
      RWCOW_pointer<Impl> _pimpl;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates CheckAccessDeleted Stream output */
  std::ostream & operator<<( std::ostream & str, const CheckAccessDeleted & obj );

  /** \relates CheckAccessDeleted::ProcInfo Stream output */
  std::ostream & operator<<( std::ostream & str, const CheckAccessDeleted::ProcInfo & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_MISC_CHECKACCESSDELETED_H
