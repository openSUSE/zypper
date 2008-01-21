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
#include <map>

#include "zypp/Dep.h"
#include "zypp/Capability.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  //
  //	CLASS NAME : Dependencies
  //
  /** Helper keeping CapabilitySet for all kinds of dependencies.
  */
  struct ZYPP_DEPRECATED Dependencies
  {
    CapabilitySet & operator[]( Dep idx_r )
    { return _capsets[idx_r]; }

    const CapabilitySet & operator[]( Dep idx_r ) const
    { return const_cast<std::map<Dep,CapabilitySet>&>(_capsets)[idx_r]; }

  private:
    std::map<Dep,CapabilitySet> _capsets;
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates Dependencies Stream output */
  std::ostream & operator<<( std::ostream & str, const Dependencies & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_DEPENDENCIES_H
