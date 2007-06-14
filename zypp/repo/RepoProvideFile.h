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

#include "zypp/base/Function.h"
#include "zypp/base/Functional.h"
#include "zypp/Repository.h"
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
    ManagedFile provideFile( Repository repo_r,
                             const OnMediaLocation & loc_r,
                             const ProvideFilePolicy & policy_r = ProvideFilePolicy() );

    /////////////////////////////////////////////////////////////////
  } // namespace repo
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_REPO_REPOPROVIDEFILE_H
