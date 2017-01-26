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
{
  ///////////////////////////////////////////////////////////////////
  namespace repo
  {

    SrcPackageProvider::SrcPackageProvider( repo::RepoMediaAccess & access_r )
      : _access( access_r )
    {}

    SrcPackageProvider::~SrcPackageProvider()
    {}

    ManagedFile SrcPackageProvider::provideSrcPackage( const SrcPackage_constPtr & srcPackage_r ) const
    { return _access.provideFile( srcPackage_r->repoInfo(), srcPackage_r->location() ); }

  } // namespace repo
  ///////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
