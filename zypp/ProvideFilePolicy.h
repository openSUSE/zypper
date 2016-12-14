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
    * Provides callback hook for progress reporting.
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
    typedef function<bool ()> FailOnChecksumErrorCB;	///< Legacy to remain bincompat
  private:
    FailOnChecksumErrorCB _failOnChecksumErrorCB;	///< Legacy to remain bincompat
    ProgressCB            _progressCB;
  };

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PROVIDEFILEPOLICY_H
