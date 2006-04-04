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

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"
#include <string>
#include <iostream>

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
        DBG << "Checksum size is " << checksum.size() << ", checksum type set to " << _type << std::endl;
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
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_CHECKSUM_H
