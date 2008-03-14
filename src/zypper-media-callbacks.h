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
#include <ctime>

#include <boost/format.hpp>

#include "zypp/ZYppCallbacks.h"
#include "zypp/Pathname.h"
#include "zypp/Url.h"
#include "zypp/media/MediaUserAuth.h"

#include "zypper.h"
#include "zypper-prompt.h"
#include "output/prompt.h"

using zypp::media::MediaChangeReport;
using zypp::media::DownloadProgressReport;

///////////////////////////////////////////////////////////////////
namespace ZmartRecipients
{

  struct MediaChangeReportReceiver : public zypp::callback::ReceiveReport<MediaChangeReport>
  {
    virtual MediaChangeReport::Action
    requestMedia(zypp::Url & url,
                 unsigned                         mediumNr,
                 const std::string &              label,
                 MediaChangeReport::Error         error,
                 const std::string &              description,
                 const std::vector<std::string> & devices,
                 unsigned int &                   index)
    {
      /*std::cout << "detected devices: "; 
      for (std::vector<std::string>::const_iterator it = devices.begin();
           it != devices.end(); ++it)
        std::cout << *it << " ";
      cout << std::endl;*/
      if (is_changeable_media(url))
      {
        //cerr << endl; // may be in the middle of RepoReport or ProgressReport \todo check this
        Zypper::instance()->out().error(description);
  
        std::string request = boost::str(boost::format(
            // TranslatorExplanation translate letters 'y' and 'n' to whathever is appropriate for your language.
            // Try to check what answers does zypper accept (it always accepts y/n at least)
            // You can also have a look at the regular expressions used to check the answer here:
            // /usr/lib/locale/<your_locale>/LC_MESSAGES/SYS_LC_MESSAGES
            _("Please insert medium [%s] #%d and type 'y' to continue or 'n' to cancel the operation."))
            % gData.current_repo.name() % mediumNr);
        if (read_bool_answer(PROMPT_YN_MEDIA_CHANGE, request, false))
          return MediaChangeReport::RETRY; 
        else
          return MediaChangeReport::ABORT;
      }

      // not displaying the error for non-changeable media, it will be displayed
      // where it is caught
      return MediaChangeReport::ABORT;
    }
  };

  // progress for downloading a file
  struct DownloadProgressReportReceiver
    : public zypp::callback::ReceiveReport<zypp::media::DownloadProgressReport>
  {
    DownloadProgressReportReceiver()
      : _gopts(Zypper::instance()->globalOpts()), _be_quiet(false)
    {}

    virtual void start( const zypp::Url & uri, zypp::Pathname localfile )
    {
      _last_reported = time(NULL);
      _last_drate_avg = -1;

      Out & out = Zypper::instance()->out();

      if (out.verbosity() < Out::HIGH &&
           (
             // don't show download info unless show_media_progress_hack is used 
             !gData.show_media_progress_hack ||
             // don't report download of the media file (bnc #330614)
             zypp::Pathname(uri.getPathName()).basename() == "media"
           )
         )
      {
        _be_quiet = true;
        return;
      }
      else
        _be_quiet = false;

      out.dwnldProgressStart(uri);
    }

    //! \todo return false on SIGINT
    virtual bool progress(int value, const zypp::Url & uri, double drate_avg, double drate_now)
    {
      if (_be_quiet)
        return true;

//      std::cout << "avg: " << drate_avg << " current: " << drate_now << std::endl;

      // don't report more often than 1 second
      time_t now = time(NULL);
      if (now > _last_reported)
        _last_reported = now;
      else
        return true;

      Zypper::instance()->out().dwnldProgress(uri, value, (long) drate_now);
      _last_drate_avg = drate_avg;
      return true;
    }

    // not used anywhere in libzypp 3.20.0
    virtual DownloadProgressReport::Action
    problem( const zypp::Url & uri, DownloadProgressReport::Error error, const std::string & description )
    {
      if (_be_quiet)
        Zypper::instance()->out().dwnldProgressEnd(uri, _last_drate_avg, true);
      Zypper::instance()->out().error(zcb_error2str(error, description));

      return (Action) read_action_ari(PROMPT_ARI_MEDIA_PROBLEM, DownloadProgressReport::ABORT);
    }

    // used only to finish, errors will be reported in media change callback (libzypp 3.20.0)
    virtual void finish( const zypp::Url & uri, Error error, const std::string & konreason )
    {
      if (_be_quiet)
        return;

      Zypper::instance()->out().dwnldProgressEnd(uri, _last_drate_avg);
    }

  private:
    const GlobalOptions & _gopts;
    bool _be_quiet;
    time_t _last_reported;
    double _last_drate_avg;
  };


  struct AuthenticationReportReceiver : public zypp::callback::ReceiveReport<zypp::media::AuthenticationReport>
  {
    virtual bool prompt(const zypp::Url & url,
                        const std::string & description,
                        zypp::media::AuthData & auth_data)
    {
      if (Zypper::instance()->globalOpts().non_interactive)
      {
        MIL << "Non-interactive mode: aborting" << std::endl;
        return false;
      }

      // curl authentication
      zypp::media::CurlAuthData * auth_data_ptr =
        dynamic_cast<zypp::media::CurlAuthData*> (&auth_data);
      if (auth_data_ptr)
      {
//! \todo move this to prompt help once it's done
//        cout_vv << "available auth types: "
//          << auth_data_ptr->authTypeAsString() << std::endl;

        Zypper::instance()->out().prompt(
            PROMPT_AUTH_USERNAME, description, PromptOptions(_("User Name"), 0));
        string username;
        std::cin >> username;
        auth_data_ptr->setUserName(username);

        Zypper::instance()->out().prompt(
            PROMPT_AUTH_PASSWORD, description, PromptOptions(_("Password"), 0));
        string password;
        std::cin >> password;
        if (password.empty()) return false;
        auth_data_ptr->setPassword(password);

        auth_data_ptr->setAuthType("basic,digest");

        return true;
      }

      return false;
    }
  };

    ///////////////////////////////////////////////////////////////////
}; // namespace ZmartRecipients
///////////////////////////////////////////////////////////////////

class MediaCallbacks {

  private:
    ZmartRecipients::MediaChangeReportReceiver _mediaChangeReport;
    ZmartRecipients::DownloadProgressReportReceiver _mediaDownloadReport;
    ZmartRecipients::AuthenticationReportReceiver _mediaAuthenticationReport;
  public:
    MediaCallbacks()
    {
      MIL << "Set media callbacks.." << endl;
      _mediaChangeReport.connect();
      _mediaDownloadReport.connect();
      _mediaAuthenticationReport.connect();
    }

    ~MediaCallbacks()
    {
      _mediaChangeReport.disconnect();
      _mediaDownloadReport.disconnect();
      _mediaAuthenticationReport.disconnect();
    }
};

#endif 
// Local Variables:
// mode: c++
// c-basic-offset: 2
// End:
