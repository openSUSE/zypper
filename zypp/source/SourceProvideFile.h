/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/source/SourceProvideFile.h
 *
*/
#ifndef ZYPP_SOURCE_SOURCEPROVIDEFILE_H
#define ZYPP_SOURCE_SOURCEPROVIDEFILE_H

#include <iosfwd>

#include "zypp/base/Function.h"
#include "zypp/base/Functional.h"
#include "zypp/Source.h"
#include "zypp/ManagedFile.h"
#include "zypp/source/OnMediaLocation.h"
#include "zypp/ProvideFilePolicy.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	provideFile
    //
    ///////////////////////////////////////////////////////////////////

    /** Provide a file from a Source.
     * Let \a source_r provide the file described by \a loc_r. In case
     * \a loc_r contains a checksum, the file is verified. \a policy_r
     * provides callback hooks for download progress reporting and behaviour
     * on failed checksum verification.
     *
     * \throws Exception
    */
    ManagedFile provideFile( Source_Ref source_r,
                             const source::OnMediaLocation & loc_r,
                             const ProvideFilePolicy & policy_r = ProvideFilePolicy() );

    /////////////////////////////////////////////////////////////////
  } // namespace source
  ///////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////
} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_SOURCE_SOURCEPROVIDEFILE_H
