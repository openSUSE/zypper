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
#include "zypp/source/ManagedFile.h"
#include "zypp/source/OnMediaLocation.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  namespace source
  { /////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : ProvideFilePolicy
    //
    /** Policy for \ref provideFile.
     * Provides callback hooks for e.g progress reporting or
     * behaviour on checksum failure. Provides default
     * implementations if no callback is set.
    */
    class ProvideFilePolicy
    {
    public:
      /** Progress callback signature. */
      typedef function<bool ( int )> ProgressCB;

      /** Set callback. */
      ProvideFilePolicy & progressCB( ProgressCB progressCB_r )
      { _progressCB = progressCB_r; return *this; }

      /** Evaluate callback. */
      bool progress( int value ) const;

    public:
      /** FailOnChecksumError callback signature. */
      typedef function<bool ()> FailOnChecksumErrorCB;

      /** Set callback. */
      ProvideFilePolicy & failOnChecksumErrorCB( FailOnChecksumErrorCB failOnChecksumErrorCB_r )
      { _failOnChecksumErrorCB = failOnChecksumErrorCB_r; return *this; }

      /** Set callback convenience.
       * Let callback return \c yesno_r.
      */
      ProvideFilePolicy & failOnChecksumErrorCB( bool yesno_r );

      /** Evaluate callback. */
      bool failOnChecksumError() const;

    private:
      FailOnChecksumErrorCB _failOnChecksumErrorCB;
      ProgressCB            _progressCB;
    };

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
