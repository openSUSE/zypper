/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/SourceInfo.cc
 *
*/

#include <string>
#include "zypp/source/SourceInfo.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace source
{
  
  std::ostream & operator<<( std::ostream & str, const SourceInfo::EnabledState & obj )
  {
    if ( obj == SourceInfo::Enabled )
      return str << std::string("true");
    if ( obj == SourceInfo::Disabled )
      return str << std::string("false");
    else
      return str << std::string("not-set");
  }
  
}
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
