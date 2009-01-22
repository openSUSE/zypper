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

static void recursive_timestamp( const Pathname &dir, time_t & max )
{
  std::list<std::string> dircontent;
  if ( filesystem::readdir( dircontent, dir, false/*no dots*/ ) != 0 )
    return; // readdir logged the error

  for_( it, dircontent.begin(), dircontent.end() )
  {
    PathInfo pi( dir + *it, PathInfo::LSTAT );
    if ( pi.isDir() )
    {
      recursive_timestamp( pi.path(), max );
      if ( pi.mtime() > max )
        max = pi.mtime();
    }
  }
}

RepoStatus dirStatus( const Pathname &dir )
{
  time_t t = 0;
  PathInfo pi(dir);
  if ( pi.isDir() )
  {
    t = pi.mtime();
    recursive_timestamp(dir,t);
  }
  RepoStatus status;
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
