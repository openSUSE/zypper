/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "zypp/base/Logger.h"
#include "zypp/cache/SourceCacher.h"
#include "zypp/source/yum/YUMSourceCacher.h"

using namespace std;

//////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
namespace source
{ /////////////////////////////////////////////////////////////////
namespace yum
{ /////////////////////////////////////////////////////////////////


YUMSourceCacher::YUMSourceCacher( const Pathname &root_r ) : cache::SourceCacher(root_r)
{
  
}

YUMSourceCacher::~YUMSourceCacher()
{
}

std::ostream & YUMSourceCacher::dumpOn( std::ostream & str ) const
{
  return str;
}

}
}
}
