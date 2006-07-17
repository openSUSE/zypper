/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <vector>

#include "zypp/base/Logger.h"
#include "zypp/base/String.h"

#include "zypp2/cache/Utils.h"

using namespace std;

//////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace cache
{ /////////////////////////////////////////////////////////////////

int tribool_to_int( boost::tribool b )
{
  if (b)
    return 1;
  else if (!b)
    return 0;
  else
    return 2;
}  
  
boost::tribool int_to_tribool( int i )
{
  if (i==1)
    return true;
  else if (i==0)
    return false;
  else
    return boost::indeterminate;
}
  
std::string checksum_to_string( const CheckSum &checksum )
{
  return checksum.type() + ":" + checksum.checksum();
}  
  
CheckSum string_to_checksum( const std::string &checksum )
{
  std::vector<std::string> words;
  if ( str::split( checksum, std::back_inserter(words), ":" ) != 2 )
    return CheckSum();
  
  return CheckSum( words[0], words[19]);
}
  
}
}

