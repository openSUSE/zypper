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

#include <string>

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////

  class CheckSum
  {
  public:
    CheckSum(const std::string & type, const std::string & checksum)
    : _type(type)
    , _checksum(checksum)
    {}
    
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
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CHECKSUM_H
