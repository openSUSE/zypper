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
#include <zypp/Repository.h>
#include <zypp/Digest.h>
#include <zypp/Url.h>

#include "zypper-callbacks.h"
#include "AliveCursor.h"

using zypp::media::MediaChangeReport;
using zypp::media::DownloadProgressReport;
using zypp::Repository;

///////////////////////////////////////////////////////////////////
namespace ZmartRecipients
{

  struct MediaChangeReportReceiver : public zypp::callback::ReceiveReport<MediaChangeReport>
  {
    virtual MediaChangeReport::Action requestMedia( Repository repo,
                                                    unsigned mediumNr,
                                                    MediaChangeReport::Error error,
                                                    const std::string & description )
    {
      cerr << description << endl; 
      // TranslatorExplanation translate letters 'y' and 'n' the same as you translated the [y/n] prompt. 
      std::string request = boost::str(boost::format(
          _("Please insert media [%s] # %d and type 'y' to continue or 'n' to cancel the operation."))
          % repo.info().name() % mediumNr);
      if (read_bool_answer(request, false))
        return MediaChangeReport::RETRY; 
      else
        return MediaChangeReport::ABORT; 
    }
  };

    // progress for downloading a file
  struct DownloadProgressReportReceiver : public zypp::callback::ReceiveReport<zypp::media::DownloadProgressReport>
  {
    virtual void start( const zypp::Url & file, zypp::Pathname localfile )
    {
      if (gSettings.verbosity == VERBOSITY_MEDIUM)
      {
        cout << CLEARLN << _("Downloading: ")
          << zypp::Pathname(file.getPathName()).basename()
          << std::endl;
      }
      else if (gSettings.verbosity >= VERBOSITY_HIGH)
      {
        cout  << CLEARLN << _("Downloading: ") << file << std::endl;
      }
    }

    virtual bool progress(int value, const zypp::Url & /*file*/)
    {
      display_progress ("download", cout_v, "Downloading", value);
      return true;
    }

    virtual DownloadProgressReport::Action problem( const zypp::Url & /*file*/, DownloadProgressReport::Error error, const std::string & description )
    {
      display_done ("download", cout_v);
      display_error (error, description);
      return DownloadProgressReport::ABORT;
    }

    virtual void finish( const zypp::Url & /*file*/, Error error, const std::string & konreason )
    {
      display_done ("download", cout_v);
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
      MIL << "Set media callbacks.." << endl;
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
