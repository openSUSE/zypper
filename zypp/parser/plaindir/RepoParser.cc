/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <iostream>
#include "zypp/base/Logger.h"
#include "zypp/base/String.h"

#include "zypp/PathInfo.h"

#include "zypp/parser/plaindir/RepoParser.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace parser
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace plaindir
{ /////////////////////////////////////////////////////////////////

static time_t recursive_timestamp( const Pathname &dir )
{
  time_t max = PathInfo(dir).mtime();
  std::list<std::string> dircontent;
  if (filesystem::readdir( dircontent, dir, false) != 0)
  {           // dont look for dot files
    ERR << "readdir " << dir << " failed" << endl;
    return 0;
  }

  for (std::list<std::string>::const_iterator it = dircontent.begin();
       it != dircontent.end();
       ++it)
  {
    Pathname dir_path = dir + *it;
    if ( PathInfo(dir_path).isDir())
    {
      time_t val = recursive_timestamp(dir_path);
      if ( val > max )
        max = val;
    }
  }
  return max;
}

RepoStatus dirStatus( const Pathname &dir )
{
  RepoStatus status;
  time_t t = recursive_timestamp(dir);
  status.setTimestamp(Date(t));
  status.setChecksum(str::numstring(t));
  return status;
}

/////////////////////////////////////////////////////////////////
} // namespace plaindir
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace parser
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
