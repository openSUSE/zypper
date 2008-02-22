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

#include "zypp/base/Logger.h"
#include "zypp/ZYppCallbacks.h"
#include "zypp/Pathname.h"
#include "zypp/KeyRing.h"
#include "zypp/Repository.h"
#include "zypp/Digest.h"
#include "zypp/Url.h"
#include "zypp/media/MediaUserAuth.h"

#include "zypper.h"
#include "zypper-callbacks.h"
#include "zypper-utils.h"
#include "output/prompt.h"

using zypp::media::MediaChangeReport;
using zypp::media::DownloadProgressReport;
using zypp::Repository;

///////////////////////////////////////////////////////////////////
namespace ZmartRecipients
{

  struct MediaChangeReportReceiver : public zypp::callback::ReceiveReport<MediaChangeReport>
  {
    virtual MediaChangeReport::Action requestMedia( zypp::Url & url,
                                                    unsigned mediumNr,
                                                    MediaChangeReport::Error error,
                                                    const std::string & description )
    {
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
  struct DownloadProgressReportReceiver : public zypp::callback::ReceiveReport<zypp::media::DownloadProgressReport>
  {
    DownloadProgressReportReceiver() : _gopts(Zypper::instance()->globalOpts())
    {}

    virtual void start( const zypp::Url & uri, zypp::Pathname localfile )
    {
      Out & out = Zypper::instance()->out();
      // don't show download info unless show_media_progress_hack is used 
      if (!gData.show_media_progress_hack && out.verbosity() < Out::HIGH)
        return;
      out.dwnldProgressStart(uri);
    }

    //! \todo return false on SIGINT
    virtual bool progress(int value, const zypp::Url & uri)
    {
      Out & out = Zypper::instance()->out();
      // don't show download info unless show_media_progress_hack is used 
      if (!gData.show_media_progress_hack && out.verbosity() < Out::HIGH)
        return true;
      out.dwnldProgress(uri, value); //! \todo add rate
      return true;
    }

    // not used anywhere in libzypp 3.20.0
    virtual DownloadProgressReport::Action
    problem( const zypp::Url & uri, DownloadProgressReport::Error error, const std::string & description )
    {
      Out & out = Zypper::instance()->out();
      if (gData.show_media_progress_hack || out.verbosity() >= Out::HIGH)
        out.dwnldProgressEnd(uri, true);
      out.error(zcb_error2str(error, description));

      return (Action) read_action_ari(PROMPT_ARI_MEDIA_PROBLEM, DownloadProgressReport::ABORT);
    }

    // used only to finish, errors will be reported in media change callback (libzypp 3.20.0)
    virtual void finish( const zypp::Url & uri, Error error, const std::string & konreason )
    {
      Out & out = Zypper::instance()->out();
      // don't show download info unless show_media_progress_hack is used 
      if (!gData.show_media_progress_hack && out.verbosity() < Out::HIGH)
        return;
      out.dwnldProgressEnd(uri);
    }
    
  private:
    const GlobalOptions & _gopts;
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
            PROMPT_AUTH_USERNAME, description, _("User Name"));
        string username;
        std::cin >> username;
        auth_data_ptr->setUserName(username);

        Zypper::instance()->out().prompt(
            PROMPT_AUTH_PASSWORD, description, _("Password"));
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
