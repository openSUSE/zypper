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
    /**
     * Creates a checksum for algorithm \param type
     * \throws if the checksum is invalid and can't be constructed
     */
    CheckSum(const std::string & type, const std::string & checksum);
    CheckSum();

    std::string type() const;
    std::string checksum() const;

    bool empty() const;
  private:
    std::string _type;
    std::string _checksum;
  };

  /** \relates CheckSum Stream output. */
  std::ostream & operator<<( std::ostream & str, const CheckSum & obj );

  /** \relates CheckSum */
  bool operator==( const CheckSum & lhs, const CheckSum & rhs );

  /** \relates CheckSum */
  bool operator!=( const CheckSum & lhs, const CheckSum & rhs );

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CHECKSUM_H
