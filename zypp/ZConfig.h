/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/ZConfig.h
 *
*/
#ifndef ZYPP_ZCONFIG_H
#define ZYPP_ZCONFIG_H

#include <iosfwd>

#include "zypp/Arch.h"
#include "zypp/Pathname.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : ZConfig
  //
  /** Interim helper class to collect global options and settings.
   * Use it to avoid hardcoded values and calls to getZypp() just
   * to retrieve some value like architecture, languages or tmppath.
  */
  class ZConfig
  {
    public:
      /** The system architecture. */
      Arch systemArchitecture() const;
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_ZCONFIG_H
