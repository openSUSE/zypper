/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/BaseVersion.h
 *
*/
#ifndef ZYPP_BASEVERSION_H
#define ZYPP_BASEVERSION_H

#include "zypp/Edition.h"
#include "zypp/CheckSum.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class BaseVersion
  {
  public:
    BaseVersion(const Edition & edition,
                const CheckSum & checksum,
    	    const Date & buildtime)
    : _edition(edition)
    , _checksum(checksum)
    , _buildtime(buildtime)
    {}
    Edition edition() const { return _edition; }
    CheckSum checksum() const { return _checksum; }
    Date buildtime() const { return _buildtime; }
  private:
    Edition _edition;
    CheckSum _checksum;
    Date _buildtime;
  };

  inline bool operator==(const BaseVersion & bv1, const BaseVersion & bv2)
  { return bv1.edition() == bv2.edition(); }

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_BASEVERSION_H
