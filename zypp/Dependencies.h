/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/Dependencies.h
 *
*/
#ifndef ZYPP_DEPENDENCIES_H
#define ZYPP_DEPENDENCIES_H

#include <iosfwd>

#include "zypp/CapSetFwd.h"
#include "zypp/CapSet.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Dependencies
  //
  /** Helper keeping CapSet for all kinds of dependencies.
  */
  struct Dependencies
  {
    /**  */
    CapSet provides;
    /**  */
    CapSet prerequires;
    /**  */
    CapSet requires;
    /**  */
    CapSet conflicts;
    /**  */
    CapSet obsoletes;
    /**  */
    CapSet recommends;
    /**  */
    CapSet suggests;
    /**  */
    CapSet freshens;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Dependencies Stream output */
  inline std::ostream & operator<<( std::ostream & str, const Dependencies & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DEPENDENCIES_H
