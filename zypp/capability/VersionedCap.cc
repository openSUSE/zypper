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
#include "zypp/capability/VersionedCap.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace capability
  { /////////////////////////////////////////////////////////////////

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
