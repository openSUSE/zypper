/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaCallbacks.h
 *
*/

#ifndef ZYPP_MEDIA_MEDIACALLBACKS_H
#define ZYPP_MEDIA_MEDIACALLBACKS_H

#include <iosfwd>

#include "zypp/Url.h"
#include "zypp/Callback.h"
#include "zypp/base/Exception.h"

namespace zypp {
  namespace media {

    ///////////////////////////////////////////////////////////////////
    // Reporting progress of download
    ///////////////////////////////////////////////////////////////////
    class DownloadProgressReport : public HACK::Callback {
      virtual ~DownloadProgressReport()
      {}
      virtual void start( const Url & url_r, const Pathname & localpath_r )
      { }
      virtual bool progress( /*const ProgressData & prg*/ )
      { return false; }
      virtual void stop( Exception & excpt_r )
      { }
    };
  
    extern DownloadProgressReport downloadProgressReport;

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_MEDIACALLBACKS_H
