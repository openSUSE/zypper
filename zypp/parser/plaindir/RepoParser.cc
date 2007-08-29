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
#include "zypp/base/Iterator.h"
#include "zypp/base/String.h"
#include "zypp/base/Regex.h"

#include <zypp/target/rpm/RpmHeader.h>
#include <zypp/target/rpm/RpmDb.h>

#include "zypp/parser/plaindir/RepoParser.h"
#include "zypp/parser/ParseException.h"
#include "zypp/capability/CapabilityImpl.h"
#include "zypp/PathInfo.h"
#include "zypp/ResStore.h"
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

data::Package_Ptr makePackageDataFromHeader( const RpmHeader::constPtr header,
                                             set<string> * filerequires,
                                             const Pathname & location, data::RecordId &repoid )
{
  if ( ! header )
    return 0;

  if ( header->isSrc() ) {
    WAR << "Can't make Package from SourcePackage header" << endl;
    return 0;
  }

  data::Package_Ptr pkg = new data::Package;

  pkg->name = header->tag_name();

  try  {
    pkg->edition = Edition( header->tag_version(),
                            header->tag_release(),
                            header->tag_epoch());
  }
  catch (Exception & excpt_r) {
    ZYPP_CAUGHT( excpt_r );
    WAR << "Package " << pkg->name << " has bad edition '"
    << (header->tag_epoch().empty()?"":(header->tag_epoch()+":"))
    << header->tag_version()
    << (header->tag_release().empty()?"":(string("-") + header->tag_release())) << "'";
    return 0;
  }

  Arch arch;
  try {
    pkg->arch = Arch( header->tag_arch() );
  }
  catch (Exception & excpt_r) {
    ZYPP_CAUGHT( excpt_r );
    WAR << "Package " << pkg->name << " has bad architecture '" << header->tag_arch() << "'";
    return 0;
  }

  pkg->deps[Dep::REQUIRES]    = header->tag_requires( filerequires );
  pkg->deps[Dep::PREREQUIRES] = header->tag_prerequires( filerequires );
  pkg->deps[Dep::CONFLICTS]   = header->tag_conflicts( filerequires );
  pkg->deps[Dep::OBSOLETES]   = header->tag_obsoletes( filerequires );
  pkg->deps[Dep::ENHANCES]    = header->tag_enhances( filerequires );
  pkg->deps[Dep::SUPPLEMENTS] = header->tag_supplements( filerequires );
  pkg->vendor                 = header->tag_vendor();
  pkg->installedSize          = header->tag_size();
  pkg->buildTime              = header->tag_buildtime();
  pkg->summary                = (TranslatedText)header->tag_summary();
  pkg->description            = (TranslatedText)header->tag_description();

  pkg->repositoryLocation     = location;

  header->tag_du( pkg->diskusage );

  list<string> filenames = header->tag_filenames();
  pkg->deps[Dep::PROVIDES] = header->tag_provides ( filerequires );

  for (list<string>::const_iterator filename = filenames.begin();
       filename != filenames.end();
       ++filename)
  {
    if ( capability::isInterestingFileSpec( *filename ) )
    {
      try {
        pkg->deps[Dep::PROVIDES].insert(capability::buildFile( ResTraits<Package>::kind, *filename ));
      }
      catch (Exception & excpt_r)
      {
        ZYPP_CAUGHT( excpt_r );
        WAR << "Ignoring invalid capability: " << *filename << endl;
      }
    }
  }

  return pkg;
}


/** RepoParser implementation.
 * \todo Clean data on exeption.
 */
class RepoParser::Impl
{
  public:
    Impl( const data::RecordId & repositoryId_r,
          data::ResolvableDataConsumer & consumer_r,
          const ProgressData::ReceiverFnc & fnc_r )
    : _repositoryId( repositoryId_r )
    , _consumer( consumer_r )
    {
      _ticks.sendTo( fnc_r );
    }
    int extract_packages_from_directory( const Pathname & path,
                                         bool recursive);
    /** Main entry to parser. */
    void parse( const Pathname & reporoot_r );
  public:

  private:
    data::RecordId                 _repositoryId;
    data::ResolvableDataConsumer & _consumer;
    ProgressData                   _ticks;

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
  extract_packages_from_directory( reporoot_r, true );
/*if ( ! _ticks.incr() )
      ZYPP_THROW( AbortRequestException() );*/
  // Done
  if ( ! _ticks.toMax() )
    ZYPP_THROW( AbortRequestException() );
}

int RepoParser::Impl::extract_packages_from_directory( const Pathname & path,
                                                       bool recursive)
{
  using target::rpm::RpmHeader;
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
    if (recursive && file_info.isDir()) {

      extract_packages_from_directory( file_path, recursive );

    } else if (file_info.isFile() && file_path.extension() == ".rpm" ) {
      RpmHeader::constPtr header = RpmHeader::readPackage( file_path, RpmHeader::NOSIGNATURE );
#warning FIX creation of Package from src.rpm header
      data::Package_Ptr package = makePackageDataFromHeader( header, NULL, *it, _repositoryId );
      if (package != NULL) {
        DBG << "Adding package " << *package << endl;
        _consumer.consumePackage( _repositoryId, package );
      }
    }
  }
  return 0;
}
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
RepoParser::RepoParser( const data::RecordId & repositoryId_r,
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
