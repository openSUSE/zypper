/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/repo/RepoProvideFile.h
 *
*/
#ifndef ZYPP_REPO_REPOPROVIDEFILE_H
#define ZYPP_REPO_REPOPROVIDEFILE_H

#include <iosfwd>

#include "zypp/base/PtrTypes.h"
#include "zypp/base/Function.h"
#include "zypp/base/Functional.h"
#include "zypp/RepoInfo.h"
#include "zypp/ManagedFile.h"
#include "zypp/OnMediaLocation.h"
#include "zypp/ProvideFilePolicy.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace repo
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	provideFile
    //
    ///////////////////////////////////////////////////////////////////

    /** Provide a file from a Repository.
     * Let \a source_r provide the file described by \a loc_r. In case
     * \a loc_r contains a checksum, the file is verified. \a policy_r
     * provides callback hooks for download progress reporting and behaviour
     * on failed checksum verification.
     *
     * \throws Exception
    */
    ManagedFile provideFile( RepoInfo repo_r,
                             const OnMediaLocation & loc_r,
                             const ProvideFilePolicy & policy_r = ProvideFilePolicy() );

    /**
     * \short Provides files from different repos
     *
     * Class that allows to get files from repositories
     * It handles automatically setting media verifiers if the
     * repo is cached, and reuses media set access opened for
     * repositories during its scope, so you can provide
     * files from different repositories in different order
     * without opening and closing medias all the time
     */
    class RepoMediaAccess
    {
    public:
      /** Ctor taking the default \ref ProvideFilePolicy. */
      RepoMediaAccess( const ProvideFilePolicy & defaultPolicy_r = ProvideFilePolicy() );
      ~RepoMediaAccess();

      /** Provide a file from a Repository.
      * Let \a source_r provide the file described by \a loc_r. In case
      * \a loc_r contains a checksum, the file is verified. \a policy_r
      * provides callback hooks for download progress reporting and behaviour
      * on failed checksum verification.
      *
      * \throws Exception
      * \todo Investigate why this needs a non-const Repository as arg.
      */
      ManagedFile provideFile( RepoInfo repo_r,
                               const OnMediaLocation & loc_r,
                               const ProvideFilePolicy & policy_r );

      /** \overload Using the current default \ref ProvideFilePolicy. */
      ManagedFile provideFile( RepoInfo repo_r, const OnMediaLocation & loc_r )
      { return provideFile( repo_r, loc_r, defaultPolicy() ); }

    public:
      /** Set a new default \ref ProvideFilePolicy. */
      void setDefaultPolicy( const ProvideFilePolicy & policy_r );

      /** Get the current default \ref ProvideFilePolicy. */
      const ProvideFilePolicy & defaultPolicy() const;

   private:
      class Impl;
       RW_pointer<Impl> _impl;
    };

    /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_REPO_REPOPROVIDEFILE_H
