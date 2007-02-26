/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_PROVIDEFILEPOLICY_H
#define ZYPP_PROVIDEFILEPOLICY_H

#include <iosfwd>

#include "zypp/base/Function.h"
#include "zypp/base/Functional.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{ /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
// CLASS NAME : ProvideFilePolicy
  
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

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PROVIDEFILEPOLICY_H
