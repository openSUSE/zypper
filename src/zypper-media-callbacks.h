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
#include <zypp/media/MediaUserAuth.h>

#include "zypper.h"
#include "zypper-callbacks.h"
#include "AliveCursor.h"
#include "zypper-utils.h"

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
        cerr << endl; // may be in the middle of RepoReport or ProgressReport
        cerr << description << endl;
  
        // TranslatorExplanation translate letters 'y' and 'n' to whathever is appropriate for your language.
        // Try to check what answers does zypper accept (it always accepts y/n at least)
        // You can also have a look at the regular expressions used to check the answer here:
        // /usr/lib/locale/<your_locale>/LC_MESSAGES/SYS_LC_MESSAGES
        std::string request = boost::str(boost::format(
            _("Please insert media [%s] # %d and type 'y' to continue or 'n' to cancel the operation."))
            % gData.current_repo.name() % mediumNr);
        if (read_bool_answer(request, false))
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
    virtual void start( const zypp::Url & file, zypp::Pathname localfile )
    {
      if (gSettings.verbosity == VERBOSITY_MEDIUM || gData.show_media_progress_hack)
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
      if (gData.show_media_progress_hack)
        display_progress ("download", cout, "Downloading", value);
      else
        display_progress ("download", cout_v, "Downloading", value);
      return true;
    }

    // not used anywhere in libzypp 3.20.0
    virtual DownloadProgressReport::Action problem( const zypp::Url & /*file*/, DownloadProgressReport::Error error, const std::string & description )
    {
      if (gData.show_media_progress_hack)
        display_done ("download", cout);
      else
        display_done ("download", cout_v);
      display_error (error, description);
      return DownloadProgressReport::ABORT;
    }

    // used only to finish, errors will be reported in media change callback (libzypp 3.20.0)
    virtual void finish( const zypp::Url & /*file*/, Error error, const std::string & konreason )
    {
      if (gData.show_media_progress_hack)
        display_done ("download", cout);
      else
        display_done ("download", cout_v);
      // don't display errors here, they will be reported in media change callback
      // display_error (error, konreason);
    }
  };

  struct AuthenticationReportReceiver : public zypp::callback::ReceiveReport<zypp::media::AuthenticationReport>
  {
    virtual bool prompt(const zypp::Url & url,
                        const std::string & description,
                        zypp::media::AuthData & auth_data)
    {
      if (gSettings.non_interactive)
      {
        MIL << "Non-interactive mode: aborting" << std::endl;
        cout_vv << description << std::endl;
        cout_vv << "Non-interactive mode: aborting" << std::endl;
        return false;
      }

      cout << description << std::endl;

      // curl authentication
      zypp::media::CurlAuthData * auth_data_ptr =
        dynamic_cast<zypp::media::CurlAuthData*> (&auth_data);
      if (auth_data_ptr)
      {
        cout_vv << "available auth types: "
          << auth_data_ptr->authTypeAsString() << std::endl;

        cout << "User Name: ";
        string username;
        std::cin >> username;
        auth_data_ptr->setUserName(username);

        cout << "Password: ";
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
