/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/CapSet.h
 *
*/
#ifndef ZYPP_CAPSET_H
#define ZYPP_CAPSET_H

#include <iosfwd>
#include <functional>
#include <set>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class Capability;

  struct CapOrder : public std::binary_function<Capability, Capability, bool>
  {
    bool operator()( const Capability & lhs, const Capability & rhs ) const;
  };

  typedef std::set<Capability,CapOrder> CapSet;

  extern std::ostream & operator<<( std::ostream & str, const CapSet & obj );

  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CAPSET_H
