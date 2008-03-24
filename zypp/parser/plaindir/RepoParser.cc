/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <iostream>
#include "zypp/base/Easy.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Iterator.h"
#include "zypp/base/String.h"
#include "zypp/base/Regex.h"

#include <zypp/target/rpm/RpmHeader.h>
#include <zypp/target/rpm/RpmDb.h>

#include "zypp/parser/plaindir/RepoParser.h"
#include "zypp/parser/ParseException.h"
#include "zypp/PathInfo.h"
#include "zypp/ZConfig.h"

using namespace std;
using namespace zypp::target::rpm;

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

/** RepoParser implementation.
 * \todo Clean data on exeption.
 */
class RepoParser::Impl
{
  public:
    Impl( const std::string & repositoryId_r,
          data::ResolvableDataConsumer & consumer_r,
          const ProgressData::ReceiverFnc & fnc_r )
    : _repositoryId( repositoryId_r )
    , _consumer( consumer_r )
    , _sysarch( ZConfig::instance().systemArchitecture() )
    {
      _ticks.sendTo( fnc_r );
    }
#if 0
    int extract_packages_from_directory( const Pathname & rootpath,
                                         const Pathname & subdir,
                                         bool recursive);
#endif
    /** Main entry to parser. */
    void parse( const Pathname & reporoot_r );
  public:

  private:
    std::string                 _repositoryId;
    data::ResolvableDataConsumer & _consumer;
    ProgressData                   _ticks;
    Arch			   _sysarch;

  private: // these (and _ticks) are actually scoped per parse() run.
};
//////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : RepoParser::Impl::parse
//	METHOD TYPE : void
//
void RepoParser::Impl::parse( const Pathname & reporoot_r )
{
  //extract_packages_from_directory( reporoot_r, Pathname(), true );
/*if ( ! _ticks.incr() )
      ZYPP_THROW( AbortRequestException() );*/
  // Done
  if ( ! _ticks.toMax() )
    ZYPP_THROW( AbortRequestException() );
}

#if 0
int RepoParser::Impl::extract_packages_from_directory( const Pathname & rootpath,
                                                       const Pathname & subdir, 
                                                       bool recursive)
{
  using target::rpm::RpmHeader;
  Pathname path = rootpath / subdir;
  Pathname filename;
  PathInfo magic;
  bool distro_magic, pkginfo_magic;

//   DBG << "extract_packages_from_directory(.., " << path << ", " << repo.info().alias() << ", " << recursive << ")" << endl;

    /*
      Check for magic files that indicate how to treat the
      directory.  The files aren't read -- it is sufficient that
      they exist.
    */

  magic = PathInfo( path + "/RC_SKIP" );
  if (magic.isExist()) {
    return 0;
  }

  magic = PathInfo( path + "/RC_RECURSIVE" );
  if (magic.isExist())
    recursive = true;

  magic = PathInfo( path + "/RC_BY_DISTRO" );
  distro_magic = magic.isExist();

  pkginfo_magic = true;
  magic = PathInfo( path + "/RC_IGNORE_PKGINFO" );
  if (magic.isExist())
    pkginfo_magic = false;


  std::list<std::string> dircontent;
  if (filesystem::readdir( dircontent, path, false) != 0) {           // dont look for dot files
    ERR << "readdir " << path << " failed" << endl;
    return -1;
  }

  for (std::list<std::string>::const_iterator it = dircontent.begin(); it != dircontent.end(); ++it) {
    Pathname file_path = path + *it;
    PathInfo file_info( file_path );
    if (recursive && file_info.isDir())
    {
      extract_packages_from_directory( rootpath, subdir / *it, recursive );
    }
    else if (file_info.isFile() && file_path.extension() == ".rpm" )
    {
      RpmHeader::constPtr header = RpmHeader::readPackage( file_path, RpmHeader::NOSIGNATURE );
#warning FIX creation of Package from src.rpm header
      // make up proper location relative to rootpath (bnc #368218)
      data::Package_Ptr package = makePackageDataFromHeader( header, NULL, subdir / *it, _repositoryId );
      if (package != NULL) {
	if (Arch(package->arch).compatibleWith(_sysarch))
	{
	  DBG << "Adding package " << *package << endl;
	  _consumer.consumePackage( package );
	}
	else
	{
	  DBG << "Ignoring package " << *package << endl;
	}
      }
    }
  }
  return 0;
}
#endif

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : RepoParser
//
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : RepoParser::RepoParser
//	METHOD TYPE : Ctor
//
RepoParser::RepoParser( const std::string & repositoryId_r,
                        data::ResolvableDataConsumer & consumer_r,
                        const ProgressData::ReceiverFnc & fnc_r )
: _pimpl( new Impl( repositoryId_r, consumer_r, fnc_r ) )
{}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : RepoParser::~RepoParser
//	METHOD TYPE : Dtor
//
RepoParser::~RepoParser()
{}

///////////////////////////////////////////////////////////////////
//
//	METHOD NAME : RepoParser::parse
//	METHOD TYPE : void
//
void RepoParser::parse( const Pathname & reporoot_r )
{
  _pimpl->parse( reporoot_r );
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
