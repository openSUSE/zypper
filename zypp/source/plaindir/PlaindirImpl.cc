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

#include "zypp/source/plaindir/PlaindirImpl.h"

using std::endl;
using namespace std;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
namespace source
{ /////////////////////////////////////////////////////////////////
 ///////////////////////////////////////////////////////////////////
namespace plaindir
{ /////////////////////////////////////////////////////////////////

PlaindirImpl::PlaindirImpl()
{
      
}

PlaindirImpl::~PlaindirImpl()
{
      
}

      
void PlaindirImpl::factoryInit()
{
  if ( ! ( (url().getScheme() == "file") || (url().getScheme() == "dir") ) )
  {
    ZYPP_THROW( Exception( "Plaindir only supports local paths, scheme [" + url().getScheme() + "] is not local" ) );
  }
  
  Pathname thePath = Pathname(url().getPathName()) + path();
  MIL << "Going to read dir " << thePath << std::endl;
  
  extract_packages_from_directory( _store, thePath, selfSourceRef(), true );
  MIL << "Plaindir source initialized." << std::endl;
  MIL << "   Url      : " << url() << std::endl;
  MIL << "   Path     : " << path() << std::endl;
}

int PlaindirImpl::extract_packages_from_directory (ResStore & store, const Pathname & path, Source_Ref source, bool recursive)
{
  Pathname filename;
  PathInfo magic;
  bool distro_magic, pkginfo_magic;

  DBG << "extract_packages_from_directory(.., " << path << ", " << source.alias() << ", " << recursive << ")" << endl;
    
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

        extract_packages_from_directory( store, file_path, source, recursive );

      } else if (file_info.isFile()) {

        string::size_type dotpos = it->find_last_of(".");
        if (dotpos == string::npos)
          continue;
        if (string(*it, ++dotpos) != "rpm")
          continue;
        target::rpm::RpmHeader::constPtr header = target::rpm::RpmHeader::readPackage( file_path );
        Package::Ptr package = target::rpm::RpmDb::makePackageFromHeader( header, NULL, file_path, source );

        if (package != NULL) {
          DBG << "Adding package " << *package << endl;
          store.insert( package );
        }
      }
    }
    return 0;
}
      
      
      
      
      
      /////////////////////////////////////////////////////////////////
    } // namespace plaindir
    ///////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
