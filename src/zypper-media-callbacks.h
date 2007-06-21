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

#include <boost/format.hpp>

#include <zypp/base/Logger.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/Pathname.h>
#include <zypp/KeyRing.h>
#include <zypp/Digest.h>
#include <zypp/Url.h>

#include "AliveCursor.h"

using zypp::media::MediaChangeReport;
using zypp::media::DownloadProgressReport;

///////////////////////////////////////////////////////////////////
namespace ZmartRecipients
{

  struct MediaChangeReportReceiver : public zypp::callback::ReceiveReport<MediaChangeReport>
  {/*
    virtual MediaChangeReport::Action requestMedia( zypp::Source_Ref source, unsigned mediumNr, MediaChangeReport::Error error, const std::string & description )
    { 
      cout << "Please insert media [" << description << "] # " << mediumNr << ". Retry [y/n]: " << endl;
      if (readBoolAnswer())
        return MediaChangeReport::RETRY; 
      else
        return MediaChangeReport::ABORT; 
    
    }
    */
  };

    // progress for downloading a file
  struct DownloadProgressReportReceiver : public zypp::callback::ReceiveReport<zypp::media::DownloadProgressReport>
  {
    virtual void start( const zypp::Url & file, zypp::Pathname localfile )
    {
      cout_v  << CLEARLN << _("Downloading: ") << file;
	    cout_vv << " to " << localfile;
      cout_v  << std::endl;
    }

    virtual bool progress(int value, const zypp::Url & /*file*/)
    {
      display_progress ("Downloading", value);
      return true;
    }

    virtual DownloadProgressReport::Action problem( const zypp::Url & /*file*/, DownloadProgressReport::Error error, const std::string & description )
    {
      display_done ();
      display_error (error, description);
      return DownloadProgressReport::ABORT;
    }

    virtual void finish( const zypp::Url & /*file*/, Error error, const std::string & konreason )
    {
      display_done ();
      display_error (error, konreason);
    }
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
// Local Variables:
// mode: c++
// c-basic-offset: 2
// End:
