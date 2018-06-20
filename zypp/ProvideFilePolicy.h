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
#include "zypp/FileChecker.h"

///////////////////////////////////////////////////////////////////
namespace zypp
{
  ///////////////////////////////////////////////////////////////////
  /// \class ProvideFilePolicy
  /// \brief Policy for \ref provideFile and \ref RepoMediaAccess.
  ///
  /// Provides callback hook for progress reporting and an optional
  /// \ref FileCecker passed down to the \ref Fetcher.
  ///////////////////////////////////////////////////////////////////
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
    /** Add a \ref FileCecker passed down to the \ref Fetcher */
    ProvideFilePolicy & fileChecker( FileChecker fileChecker_r )
    { _fileChecker = std::move(fileChecker_r); return *this; }

    /** The \ref FileCecker. */
    const FileChecker & fileChecker() const
    { return _fileChecker; }

  private:
    FileChecker           _fileChecker;
    ProgressCB            _progressCB;
  };

} // namespace zypp
///////////////////////////////////////////////////////////////////
#endif // ZYPP_PROVIDEFILEPOLICY_H
