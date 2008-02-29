/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repo/SrcPackageProvider.cc
 *
*/
#include <iostream>
#include <fstream>

#include "zypp/repo/SrcPackageProvider.h"
#include "zypp/PathInfo.h"
#include "zypp/TmpPath.h"
#include "zypp/SrcPackage.h"

using std::endl;

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    namespace
    { /////////////////////////////////////////////////////////////////

      typedef std::string     (SrcPackage::*inlined)() const;
      typedef OnMediaLocation (SrcPackage::*location)() const;

      /** Provide a SrcPackage in a local file. */
      ManagedFile doProvideSrcPackage( repo::RepoMediaAccess & access_r,
                                   const SrcPackage & script_r,
                                   inlined inlined_r, location location_r )
      {
        ManagedFile ret;

        // 1st try inlined
        std::string inlined( (script_r.*inlined_r)() );
        if ( ! inlined.empty() )
        {
          // Take care the TmpFile goes out of scope BEFORE the
          // ofstream opens the file again.
          ret = ManagedFile( filesystem::TmpFile( filesystem::TmpPath::defaultLocation(),
                                                  "zypp-script-"+script_r.name() ),
                             filesystem::unlink );
          std::ofstream str( ret.value().c_str() );
          str << inlined << endl;
        }
        else
        {
          // otherwise try download
          OnMediaLocation location( (script_r.*location_r)() );
          if ( ! location.filename().empty() )
          {
            ret = access_r.provideFile( script_r.repoInfo(), location );
          }
          else
          {
            // no script
            return ManagedFile();
          }
        }

        // HERE: got the script
        filesystem::chmod( ret, 0700 );
        return ret;
      }

      /////////////////////////////////////////////////////////////////
    } // namespace
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : SrcPackageProvider::SrcPackageProvider
    //	METHOD TYPE : Ctor
    //
    SrcPackageProvider::SrcPackageProvider( repo::RepoMediaAccess & access_r )
      : _access( access_r )
    {}

    ///////////////////////////////////////////////////////////////////
    //
    //	METHOD NAME : SrcPackageProvider::~SrcPackageProvider
    //	METHOD TYPE : Dtor
    //
    SrcPackageProvider::~SrcPackageProvider()
    {}

    ManagedFile SrcPackageProvider::provideSrcPackage( const SrcPackage_constPtr & srcPackage_r ) const
    {
      return _access.provideFile( srcPackage_r->repoInfo(), srcPackage_r->location() );
    }

    /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
