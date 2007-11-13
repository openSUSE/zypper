/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/capability/VersionedCap.cc
 *
*/
#include <iostream>
#include "zypp/base/Hash.h"
#include "zypp/capability/VersionedCap.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

    IMPL_PTR_TYPE(VersionedCap)

    std::string VersionedCap::encode() const
    {
      std::string ret( name() );
      if ( _range.op != Rel::ANY )
        {
          ret += " ";
          ret += _range.op.asString();
          ret += " ";
          ret += _range.value.asString();
        }
      return ret;
    }

    size_t VersionedCap::hash() const
    {
      size_t ret = __gnu_cxx::hash<const char*>()( name().c_str() );
      ret ^= __gnu_cxx::hash<const char*>()( _range.value.version().c_str() );
      return ret;
    }

    std::string VersionedCap::index() const
    { return name(); }

    Rel VersionedCap::op() const
    { return _range.op; }

    Edition VersionedCap::edition () const
    { return _range.value; }

    const Edition::MatchRange & VersionedCap::range() const
    { return _range; }

    /////////////////////////////////////////////////////////////////
  } // namespace capability
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
