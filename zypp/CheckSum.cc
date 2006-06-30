/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <iosfwd>
#include <string>

#include "zypp/base/Exception.h"
#include "zypp/CheckSum.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  CheckSum::CheckSum(const std::string & type, const std::string & checksum)
  {
    _checksum = checksum;

    // correct ambiguous algorithm
    if (str::toLower(type) == "sha")
    {
      if (checksum.size() == 40)
        _type = "sha1";
      else if (checksum.size() == 64)
        _type = "sha256";
      else
        ZYPP_THROW(Exception("Bad " + type + " algorithm of size " + str::numstring(checksum.size()) + " specified. Can't determine if it is sha1 or sha256 from size."));
    }
    else
    {
      _type = type;
    }

    if (
          ( (str::toLower(type) == "md2"     ) && (checksum.size() != 32) ) ||
          ( (str::toLower(type) == "md4"     ) && (checksum.size() != 32) ) ||
          ( (str::toLower(type) == "md5"     ) && (checksum.size() != 32) ) ||
          ( (str::toLower(type) == "sha1"    ) && (checksum.size() != 40) ) ||
          ( (str::toLower(type) == "rmd160"  ) && (checksum.size() != 40) ) ||
          ( (str::toLower(type) == "sha256"  ) && (checksum.size() != 64) ) )
    {
      ZYPP_THROW(Exception("Bad checksum, " + type + " algorithm of size " + str::numstring(checksum.size())));
    }

  }

  CheckSum::CheckSum()
  {}

  std::string CheckSum::type() const
  { return _type; }
  
  std::string CheckSum::checksum() const
  { return _checksum; }

  bool CheckSum::empty() const
  { return (checksum().empty() || type().empty()); }
  
  /** \relates CheckSum Stream output. */
  std::ostream & operator<<( std::ostream & str, const CheckSum & obj )
  { return str << (obj.empty() ? std::string("NoCheckSum")
                               : obj.type()+"-"+obj.checksum()  ); }

  /** \relates CheckSum */
  bool operator==( const CheckSum & lhs, const CheckSum & rhs )
  { return lhs.checksum() == rhs.checksum() && lhs.type() == rhs.type(); }

  /** \relates CheckSum */
  bool operator!=( const CheckSum & lhs, const CheckSum & rhs )
  { return ! ( lhs == rhs ); }

} // namespace zypp
///////////////////////////////////////////////////////////////////
