/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/SourceInfo.h
 *
*/
#ifndef ZYPP_SourceInfo_H
#define ZYPP_SourceInfo_H

#include <list>

#include "zypp/Pathname.h"
#include "zypp/Url.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace source
{
  
  struct SourceInfo
  {
    enum EnabledState {
      Enabled,
      Disabled,
      NotSet
    };
    
    SourceInfo() :
        enabled (NotSet),
        autorefresh(NotSet)
    {
      
    };
    
    EnabledState enabled;
    EnabledState autorefresh;
    Pathname product_dir;
    std::string type;
    Url url;
    Pathname cache_dir;
    std::string alias;
  };  
  
  std::ostream & operator<<( std::ostream & str, const SourceInfo::EnabledState & obj );
  typedef std::list<SourceInfo> SourceInfoList;
  
}
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SourceInfo_H
