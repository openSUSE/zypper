/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/CheckSum.h
 *
*/
#ifndef ZYPP_CHECKSUM_H
#define ZYPP_CHECKSUM_H

#include <iosfwd>
#include <string>

#include "zypp/base/String.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class CheckSum
  {
  public:
    CheckSum(const std::string & type, const std::string & checksum)
    {
      _checksum = checksum;
      if (str::toLower(type) == "sha")
      {
        if (checksum.size() == 40)
          _type = "sha1";
        else if (checksum.size() == 64)
          _type = "sha256";
      }
      else
      {
        _type = type;
      }
    }

    CheckSum()
    {}

    std::string type() const
    { return _type; }
    std::string checksum() const
    { return _checksum; }

    bool empty() const
    { return (checksum().empty() || type().empty()); }
  private:
    std::string _type;
    std::string _checksum;
  };

  /** \relates CheckSum Stream output. */
  inline std::ostream & operator<<( std::ostream & str, const CheckSum & obj )
  { return str << (obj.empty() ? obj.type()+"-"+obj.checksum() : std::string("NoCheckSum") ); }

  /** \relates CheckSum */
  inline bool operator==( const CheckSum & lhs, const CheckSum & rhs )
  { return lhs.checksum() == rhs.checksum() && lhs.type() == rhs.type(); }

  /** \relates CheckSum */
  inline bool operator!=( const CheckSum & lhs, const CheckSum & rhs )
  { return ! ( lhs == rhs ); }

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CHECKSUM_H
