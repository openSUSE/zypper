/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZMART_MEDIA_CALLBACKS_H
#define ZMART_MEDIA_CALLBACKS_H

#include <stdlib.h>
#include <ctime>

#include <zypp/ZYppCallbacks.h>
#include <zypp/base/Logger.h>
#include <zypp/Pathname.h>
#include <zypp/Url.h>

#include "Zypper.h"
#include "utils/prompt.h"

// auto-repeat counter limit
#define REPEAT_LIMIT 3

using media::MediaChangeReport;
using media::DownloadProgressReport;

///////////////////////////////////////////////////////////////////
namespace ZmartRecipients
{
  class repeat_counter_ {
    private:
      Url url;
      unsigned counter;
    public:
      repeat_counter_():counter(0){}
      bool counter_overrun(const Url & u){
        if (u==url)
        {
          if (++counter>=REPEAT_LIMIT)
          {
            counter = 0;	// reset!: next request might use the same URL again.
            return true;
          }
        }
        else
        {
          url = u;
          counter = 0;
        }
        return false;
      }
  };

  ///////////////////////////////////////////////////////////////////
  /// \brief Legacy or feature - Base class protecting a callback receiver from exiting on the 1st CTRL-C
  ///
  /// Media callbacks pass down the exit request to abort the download and
  /// handle the user response. This base class handles the Zypper::SigExitGuard
  /// to accompoish this.
  ///
  /// \note Derived classes overriding reportbegin/reportend must explicitly call
  ///       the method in this instance.
  ///
  template <typename ReportT>
  struct ExitGuardedReceiveReport : public callback::ReceiveReport<ReportT>
  {
    void reportbegin() override
    { _g = Zypper::sigExitGuard(); }

    void reportend() override
    { _g.reset(); }

   private:
     Zypper::SigExitGuard _g;
  };


  struct MediaChangeReportReceiver : public ExitGuardedReceiveReport<MediaChangeReport>
  {
    virtual MediaChangeReport::Action
    requestMedia(Url & url,
                 unsigned                         mediumNr,
                 const std::string &              label,
                 MediaChangeReport::Error         error,
                 const std::string &              description,
                 const std::vector<std::string> & devices,
                 unsigned &                   index);
    private:
      repeat_counter_ repeat_counter;
  };

  // progress for downloading a file
  struct DownloadProgressReportReceiver : public ExitGuardedReceiveReport<media::DownloadProgressReport>
  {
    DownloadProgressReportReceiver()
      : _be_quiet(false)
    {}

    virtual void start( const Url & uri, Pathname localfile )
    {
      _last_drate_avg = -1;

      Out & out = Zypper::instance().out();

      if (out.verbosity() < Out::HIGH &&
           (
             // don't show download info unless scopedVerboseDownloadProgress is demanded
             not Zypper::instance().runtimeData().scopedVerboseDownloadProgress.isDemanded() ||
             // don't report download of the media file (bnc #330614)
             Pathname(uri.getPathName()).basename() == "media"
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

    virtual bool progress(int value, const Url & uri, double drate_avg, double drate_now)
    {
      Zypper & zypper( Zypper::instance() );

      if (zypper.exitRequested())
      {
        DBG << "received exit request" << std::endl;
        return false;
      }

      if (!zypper.runtimeData().raw_refresh_progress_label.empty())
        zypper.out().progress(
          "raw-refresh", zypper.runtimeData().raw_refresh_progress_label);

      if (_be_quiet)
        return true;

      zypper.out().dwnldProgress(uri, value, (long) drate_now);
      _last_drate_avg = drate_avg;
      return true;
    }

    virtual DownloadProgressReport::Action
    problem( const Url & uri, DownloadProgressReport::Error error, const std::string & description )
    {
      if ( error == DownloadProgressReport::NO_ERROR ) {
        // NO_ERROR: just a report but let the caller proceed as appropriate...
        Zypper::instance().out().info() << "- " << ( ColorContext::LOWLIGHT << description );
        return DownloadProgressReport::IGNORE;
      }

      if ( not _be_quiet )
        Zypper::instance().out().dwnldProgressEnd(uri, _last_drate_avg, true);
      Zypper::instance().out().error(zcb_error2str(error, description));

      Action action = (Action) read_action_ari(
          PROMPT_ARI_MEDIA_PROBLEM, DownloadProgressReport::ABORT);
      if (action == DownloadProgressReport::RETRY)
        Zypper::instance().requestExit(false);
      return action;
    }

    // used only to finish, errors will be reported in media change callback (libzypp 3.20.0)
    virtual void finish( const Url & uri, Error error, const std::string & konreason )
    {
      if (_be_quiet)
        return;

      Zypper::instance().out().dwnldProgressEnd(
          uri, _last_drate_avg, ( error == NOT_FOUND ? indeterminate : TriBool(error != NO_ERROR) ) );
    }

  private:
    bool _be_quiet;
    double _last_drate_avg;
  };


  struct AuthenticationReportReceiver : public ExitGuardedReceiveReport<media::AuthenticationReport>
  {
    virtual bool prompt(const Url & url,
                        const std::string & description,
                        media::AuthData & auth_data);
  };

} // namespace ZmartRecipients
///////////////////////////////////////////////////////////////////

class MediaCallbacks {

  private:
    ZmartRecipients::MediaChangeReportReceiver _mediaChangeReport;
    ZmartRecipients::DownloadProgressReportReceiver _mediaDownloadReport;
    ZmartRecipients::AuthenticationReportReceiver _mediaAuthenticationReport;
  public:
    MediaCallbacks()
    {
      MIL << "Set media callbacks.." << std::endl;
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
