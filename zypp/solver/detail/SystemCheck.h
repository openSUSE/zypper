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

        /** Set configuration file of system requirements
	 *  Should be used for testcase only   
	 */
        bool setFile(const Pathname & file) const;

        /** Returns a list of required system capabilities.
        */
        const CapabilitySet & requiredSystemCap() const;

        /** Returns a list of conflicting system capabilities.
        */
        const CapabilitySet & conflictSystemCap() const;

      private:
        /** Ctor taking the file to read. */
        SystemCheck();
	bool loadFile() const;

    };
    ///////////////////////////////////////////////////////////////////

    /** \relates SystemCheck Stream output */
    std::ostream & operator<<( std::ostream & str, const SystemCheck & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TARGET_SYSTEMCHECK_H
