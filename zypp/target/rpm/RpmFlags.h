/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/target/rpm/RpmFlags.h
 *
*/
#ifndef ZYPP_TARGET_RPM_RPMFLAGS_H
#define ZYPP_TARGET_RPM_RPMFLAGS_H

#include <iosfwd>

#include "zypp/base/Flags.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace target
  { /////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////
    namespace rpm
    { /////////////////////////////////////////////////////////////////

      /**
       * Bits representing rpm installation options.
       *
       * Use \ref RpmInstFlags as a type-safe way of
       * storing OR-combinations.
       *
       * @see RpmDb::installPackage(), RpmDb::removePackage(),
       */
      enum RpmInstFlag
      {
        RPMINST_NONE       = 0x0000,
        RPMINST_EXCLUDEDOCS= 0x0001,
        RPMINST_NOSCRIPTS  = 0x0002,
        RPMINST_FORCE      = 0x0004,
        RPMINST_NODEPS     = 0x0008,
        RPMINST_IGNORESIZE = 0x0010,
        RPMINST_JUSTDB     = 0x0020,
        RPMINST_NODIGEST   = 0x0040,
        RPMINST_NOSIGNATURE= 0x0080,
        RPMINST_NOUPGRADE  = 0x0100,
        RPMINST_TEST	   = 0x0200,
	RPMINST_NOPOSTTRANS= 0x0400
      };

      /** \relates RpmInstFlag Type-safe way of storing OR-combinations. */
      ZYPP_DECLARE_FLAGS_AND_OPERATORS( RpmInstFlags, RpmInstFlag );

      /////////////////////////////////////////////////////////////////
    } // namespace rpm
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace target
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_TARGET_RPM_RPMFLAGS_H
