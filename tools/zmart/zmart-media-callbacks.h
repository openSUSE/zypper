/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZMART_MEDIA_CALLBACKS_H
#define ZMART_MEDIA_CALLBACKS_H

#include <stdlib.h>
#include <iostream>

#include <zypp/base/Logger.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/Pathname.h>
#include <zypp/KeyRing.h>
#include <zypp/Digest.h>
#include <zypp/Url.h>
#include <zypp/Source.h>

#include "AliveCursor.h"

using zypp::media::MediaChangeReport;
using zypp::media::DownloadProgressReport;

///////////////////////////////////////////////////////////////////
namespace ZmartRecipients
{

  struct MediaChangeReportReceiver : public zypp::callback::ReceiveReport<MediaChangeReport>
  {
    virtual MediaChangeReport::Action requestMedia( const zypp::Source_Ref source, unsigned mediumNr, MediaChangeReport::Error error, std::string description )
    { return MediaChangeReport::ABORT; }
  };

    // progress for downloading a file
  struct DownloadProgressReportReceiver : public zypp::callback::ReceiveReport<zypp::media::DownloadProgressReport>
  {
    virtual void start( const zypp::Url &file, zypp::Pathname localfile )
    {}

    virtual bool progress(int value, const zypp::Url &file)
    { return true; }

    virtual DownloadProgressReport::Action problem( const zypp::Url &file, DownloadProgressReport::Error error, std::string description )
    { return DownloadProgressReport::ABORT; }

    virtual void finish( const zypp::Url &file, Error error, std::string reason )
    {}
  };
  
    ///////////////////////////////////////////////////////////////////
}; // namespace ZmartRecipients
///////////////////////////////////////////////////////////////////

class MediaCallbacks {

  private:
    ZmartRecipients::MediaChangeReportReceiver _mediaChangeReport;
    ZmartRecipients::DownloadProgressReportReceiver _mediaDownloadReport;
  public:
    MediaCallbacks()
    {
      _mediaChangeReport.connect();
      _mediaDownloadReport.connect();
    }

    ~MediaCallbacks()
    {
      _mediaChangeReport.disconnect();
      _mediaDownloadReport.disconnect();
    }
};

#endif 
