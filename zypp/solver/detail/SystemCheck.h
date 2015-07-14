/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/SystemCheck.h
 *
*/
#ifndef ZYPP_TARGET_SYSTEMCHECK_H
#define ZYPP_TARGET_SYSTEMCHECK_H
#ifndef ZYPP_USE_RESOLVER_INTERNALS
#error Do not directly include this file!
#else

#include <iosfwd>

#include "zypp/base/NonCopyable.h"
#include "zypp/Capability.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SystemCheck
    //
    /** Save and restore locale set from file.
     */
    class SystemCheck : private base::NonCopyable
    {
      friend std::ostream & operator<<( std::ostream & str, const SystemCheck & obj );

      public:

	/** Singleton */
	static const SystemCheck & instance();

        /** Return the file path. */
        const Pathname & file();

        /** Return the directory path. */
        const Pathname & dir();

        /** Set configuration file of system requirements
	 *  Should be used for testcase only   
	 */
        bool setFile(const Pathname & file) const;

        /** Set configuration directory for files of system
	 *  requirements.
         *  Should be used for testcase only
	 */
        bool setDir(const Pathname & dir) const;

        /** Returns a list of required system capabilities.
        */
        const CapabilitySet & requiredSystemCap() const;

        /** Returns a list of conflicting system capabilities.
        */
        const CapabilitySet & conflictSystemCap() const;

      private:
        /** Ctor taking the file to read. */
        SystemCheck();
        bool loadFile(Pathname &file, bool reset_caps = true) const;
	bool loadFiles() const;
    };
    ///////////////////////////////////////////////////////////////////

    /** \relates SystemCheck Stream output */
    std::ostream & operator<<( std::ostream & str, const SystemCheck & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_USE_RESOLVER_INTERNALS
#endif // ZYPP_TARGET_SYSTEMCHECK_H
