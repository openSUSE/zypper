/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repo/SrcPackageProvider.h
 *
*/
#ifndef ZYPP_REPO_SRCPACKAGEPROVIDER_H
#define ZYPP_REPO_SRCPACKAGEPROVIDER_H

#include <iosfwd>

#include "zypp/base/NonCopyable.h"
#include "zypp/base/PtrTypes.h"

#include "zypp/repo/RepoProvideFile.h"
#include "zypp/ManagedFile.h"
#include "zypp/ResTraits.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////

    class RepoMediaAccess;

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : SrcPackageProvider
    //
    /** */
    class SrcPackageProvider : private base::NonCopyable
    {
    public:
      /** Ctor */
      SrcPackageProvider( repo::RepoMediaAccess & access_r );
      /** Dtor */
      ~SrcPackageProvider();

    public:
      /** Provide SrcPackage in a local file. */
      ManagedFile provideSrcPackage( const SrcPackage_constPtr & srcPackage_r ) const;

    private:
      RepoMediaAccess & _access;
    };
    ///////////////////////////////////////////////////////////////////

    /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_REPO_SRCPACKAGEPROVIDER_H
