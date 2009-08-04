/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/misc/DefaultLoadSystem.h
 *
*/
#ifndef ZYPP_MISC_DEFAULTLOADSYSTEM_H
#define ZYPP_MISC_DEFAULTLOADSYSTEM_H

#include <iosfwd>

#include "zypp/Pathname.h"
#include "zypp/base/Flags.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace misc
  { /////////////////////////////////////////////////////////////////

    /**
     * Bits for tuning \ref defaultLoadSystem.
     *
     * Use \ref LoadSystemFlags as a type-safe way of
     * storing OR-combinations.
     */
    enum LoadSystemFlag
    {
      LS_READONLY	= (1 << 0),	//!< // Create readonly ZYpp instance.
      LS_NOREFRESH	= (1 << 1)	//!< // Don't refresh existing repos.
    };

    /** \relates LoadSystemFlag Type-safe way of storing OR-combinations. */
    ZYPP_DECLARE_FLAGS_AND_OPERATORS( LoadSystemFlags, LoadSystemFlag );

    /**
     * Create the ZYpp instance and load target and enabled repositories.
     *
     * \see LoadSystemFlag for options.
     *
     * \throws Exception on error
     *
     * \todo properly handle service refreshs
     */
    void defaultLoadSystem( const Pathname & sysRoot_r = "/", LoadSystemFlags flags_r = LoadSystemFlags() );

    /** \overload */
    inline void defaultLoadSystem( LoadSystemFlags flags_r )
    { defaultLoadSystem( "/", flags_r ); }

    /////////////////////////////////////////////////////////////////
  } // namespace misc
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_MISC_DEFAULTLOADSYSTEM_H
