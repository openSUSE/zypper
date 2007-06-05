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
#include "zypp/Locale.h"
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

      /** The prefered locale for translated labels, descriptions,
       *  descriptions, etc. passed to the UI.
       */
      Locale defaultTextLocale() const;
      
      /**
       * Path where the repo metadata is downloaded and kept.
       */
      Pathname defaultRepoRawCachePath() const;
      
      /**
       * Path where the processed cache is kept
       * (this is where zypp.db is located.
       */
      Pathname defaultRepoCachePath() const;
      
      /**
       * Path where the known repositories
       * .repo files are kept
       */
      Pathname defaultKnownReposPath() const;
      
      /**
       * Separator string for storing/reading sets of strings to/from
       * metadata cache DB.
       */
      const std::string & cacheDBSplitJoinSeparator() const;
  };
  ///////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_ZCONFIG_H
