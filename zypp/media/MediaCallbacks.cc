/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaCallbacks.cc
 *
*/

#include <iostream>

#include <y2pm/MediaCallbacks.h>

using namespace std;

///////////////////////////////////////////////////////////////////
namespace MediaCallbacks {
///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  // Reporting progress of download
  ///////////////////////////////////////////////////////////////////
  DownloadProgressReport downloadProgressReport;

  void DownloadProgressCallback::start( const Url & url_r, const Pathname & localpath_r ) {
  }
  bool DownloadProgressCallback::progress( const ProgressData & prg ) {
    return true; // continue download
  }
  void DownloadProgressCallback::stop( PMError error ) {
  }

#if 0
  ///////////////////////////////////////////////////////////////////
  // Reporting @
  ///////////////////////////////////////////////////////////////////
  @Report @Report;

  void @Callback::start() {
  }
  void @Callback::progress( const ProgressData & prg ) {
  }
  void @Callback::stop( PMError error ) {
  }
#endif

///////////////////////////////////////////////////////////////////
} // namespace MediaCallbacks
///////////////////////////////////////////////////////////////////
