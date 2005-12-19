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
    public:
      virtual ~DownloadProgressReport()
      {}
      /** Start the operation */
      virtual void start( const Url & url_r, const Pathname & localpath_r ) 
      { }
      /**
       * Inform about progress
       * Return true on abort
       */
      virtual bool progress( unsigned percent )
      { return false; }
      /** Finish operation in case of success */
      virtual void end()
      { }
      /** Finish operatino in case of fail, report fail exception */
      virtual void end( Exception & excpt_r )
      { }
    };
  
    extern DownloadProgressReport downloadProgressReport;

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_MEDIACALLBACKS_H
