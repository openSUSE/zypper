/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#ifndef ZMART_RPM_CALLBACKS_H
#define ZMART_RPM_CALLBACKS_H

#include <iostream>
#include <sstream>
#include <ctime>

#include <boost/format.hpp>

#include "zypp/base/Logger.h"
#include "zypp/ZYppCallbacks.h"
#include "zypp/Package.h"
#include "zypp/Patch.h"

#include "Zypper.h"
#include "output/prompt.h"


static bool report_again(timespec * last)
{
  // don't report more often than 5 times per sec
  timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  if (now.tv_sec > last->tv_sec ||
      (now.tv_sec == last->tv_sec && now.tv_nsec > last->tv_nsec + 200000000L))
  {
    *last = now;
    return true;
  }
  else
    return false;
}

///////////////////////////////////////////////////////////////////
namespace ZmartRecipients
{


// resolvable Message
struct PatchMessageReportReceiver : public zypp::callback::ReceiveReport<zypp::target::PatchMessageReport>
{

  /** Display \c patch->message().
   * Return \c true to continue, \c false to abort commit.
   */
  virtual bool show( zypp::Patch::constPtr & patch )
  {
    Out & out = Zypper::instance()->out();
    std::ostringstream s;
    s << patch; // [patch]important-patch-101 \todo make some meaningfull message out of this
    out.info(s.str(), Out::HIGH);
    out.info(patch->message());

    return read_bool_answer(PROMPT_PATCH_MESSAGE_CONTINUE, _("Continue?"), true);
  }
};


struct PatchScriptReportReceiver : public zypp::callback::ReceiveReport<zypp::target::PatchScriptReport>
{
  std::string _label;

  virtual void start( const zypp::Package::constPtr & package,
		      const zypp::Pathname & path_r ) // script path
  {
    _label = boost::str(
        // TranslatorExplanation speaking of a script - "Running: script file name (package name, script dir)"
        boost::format(_("Running: %s  (%s, %s)")) % path_r.basename() % package->name() % path_r.dirname());
    std::cout << _label << std::endl;
  }

  /**
   * Progress provides the script output. If the script is quiet,
   * from time to time still-alive pings are sent to the ui. (Notify=PING)
   * Returning \c FALSE aborts script execution.
   */
  virtual bool progress( Notify kind, const std::string &output )
  {
    Zypper & zypper = *Zypper::instance();
    static bool was_ping_before = false;
    if (kind == PING)
    {
      std::cout << "." << std::flush;
      was_ping_before = true;
    }
    else
    {
      if (was_ping_before)
       std::cout << std::endl;
      std::cout << output;
      was_ping_before = false;
    }

    return !zypper.exitRequested();
  }

  /** Report error. */
  virtual Action problem( const std::string & description )
  {
    Zypper & zypper = *Zypper::instance();

    zypper.out().error(description);

    Action action = (Action) read_action_ari (PROMPT_ARI_PATCH_SCRIPT_PROBLEM, ABORT);
    if (action == zypp::target::PatchScriptReport::ABORT)
      zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return action;
  }

  /** Report success. */
  virtual void finish()
  {
    Zypper::instance()->out().progressEnd("run-script", _label);
  }
};

///////////////////////////////////////////////////////////////////
 // progress for removing a resolvable
struct RemoveResolvableReportReceiver : public zypp::callback::ReceiveReport<zypp::target::rpm::RemoveResolvableReport>
{
  std::string _label;
  timespec _last_reported;

  virtual void start( zypp::Resolvable::constPtr resolvable )
  {
    ::clock_gettime(CLOCK_REALTIME, &_last_reported);
    // translators: This text is a progress display label e.g. "Removing packagename-x.x.x [42%]"
    _label = boost::str(boost::format(_("Removing %s-%s"))
        % resolvable->name() % resolvable->edition());
    Zypper::instance()->out().progressStart("remove-resolvable", _label);
  }

  virtual bool progress(int value, zypp::Resolvable::constPtr resolvable)
  {
    // don't report too often
    if (!report_again(&_last_reported))
      return true;

    Zypper::instance()->out().progress("remove-resolvable", _label, value);
    return true;
  }

  virtual Action problem( zypp::Resolvable::constPtr resolvable, Error error, const std::string & description )
  {
    Zypper::instance()->out().progressEnd("remove-resolvable", _label, true);
    std::ostringstream s;
    s << boost::format(_("Removal of %s failed:")) % resolvable << std::endl;
    s << zcb_error2str(error, description);
    Zypper::instance()->out().error(s.str());
    return (Action) read_action_ari (PROMPT_ARI_RPM_REMOVE_PROBLEM, ABORT);
  }

  virtual void finish( zypp::Resolvable::constPtr /*resolvable*/, Error error, const std::string & reason )
  {
    if (error != NO_ERROR)
      // set proper exit code, don't write to output, the error should have been reported in problem()
      Zypper::instance()->setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    else
    {
      Zypper::instance()->out().progressEnd("remove-resolvable", _label);

      // print additional rpm output
      // bnc #369450
      if (!reason.empty())
        Zypper::instance()->out().info(reason);
    }
  }
};

std::ostream & operator << (std::ostream & stm,
                            zypp::target::rpm::InstallResolvableReport::RpmLevel level)
{
  static const char * level_s[] = {
    // TranslatorExplanation --nodeps and --force are options of the rpm command, don't translate
    //! \todo use format
    "", _("(with --nodeps)"), _("(with --nodeps --force)")
  };
  return stm << level_s[level];
}

///////////////////////////////////////////////////////////////////
// progress for installing a resolvable
struct InstallResolvableReportReceiver : public zypp::callback::ReceiveReport<zypp::target::rpm::InstallResolvableReport>
{
  zypp::Resolvable::constPtr _resolvable;
  std::string _label;
  timespec _last_reported;

  void display_step( zypp::Resolvable::constPtr resolvable, int value )
  {
  }

  virtual void start( zypp::Resolvable::constPtr resolvable )
  {
    clock_gettime(CLOCK_REALTIME, &_last_reported);
    _resolvable = resolvable;
    // TranslatorExplanation This text is a progress display label e.g. "Installing foo-1.1.2 [42%]"
    _label = boost::str(boost::format(_("Installing: %s-%s"))
        % resolvable->name() % resolvable->edition());
    Zypper::instance()->out().progressStart("install-resolvable", _label);
  }

  virtual bool progress(int value, zypp::Resolvable::constPtr resolvable)
  {
    // don't report too often
    if (!report_again(&_last_reported))
      return true;

    Zypper::instance()->out().progress("install-resolvable", _label, value);
    return true;
  }

  virtual Action problem( zypp::Resolvable::constPtr resolvable, Error error, const std::string & description, RpmLevel level )
  {
    if (level < RPM_NODEPS_FORCE)
    {
      DBG << "Install failed, will retry more aggressively"
             " (with --nodeps, --force)." << std::endl;
      return ABORT;
    }

    Zypper::instance()->out().progressEnd("install-resolvable", _label, true);
    std::ostringstream s;
    s << boost::format(_("Installation of %s-%s failed:")) % resolvable->name() % resolvable->edition() << std::endl;
    s << level << " " << zcb_error2str(error, description);
    Zypper::instance()->out().error(s.str());

    return (Action) read_action_ari (PROMPT_ARI_RPM_INSTALL_PROBLEM, ABORT);
  }

  virtual void finish( zypp::Resolvable::constPtr /*resolvable*/, Error error, const std::string & reason, RpmLevel level )
  {
    if (error != NO_ERROR && level < RPM_NODEPS_FORCE)
    {
      DBG << "level < RPM_NODEPS_FORCE: aborting without displaying an error"
          << std::endl;
      return;
    }

    if (error != NO_ERROR)
      // don't write to output, the error should have been reported in problem() (bnc #381203)
      Zypper::instance()->setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    else
    {
      Zypper::instance()->out().progressEnd("install-resolvable", _label);

      // print additional rpm output
      // bnc #369450
      if (!reason.empty())
        Zypper::instance()->out().info(reason);
    }
  }
};


///////////////////////////////////////////////////////////////////
}; // namespace ZyppRecipients
///////////////////////////////////////////////////////////////////

class RpmCallbacks {

  private:
    ZmartRecipients::PatchMessageReportReceiver _messageReceiver;
    ZmartRecipients::PatchScriptReportReceiver _scriptReceiver;
    ZmartRecipients::RemoveResolvableReportReceiver _installReceiver;
    ZmartRecipients::InstallResolvableReportReceiver _removeReceiver;
    int _step_counter;

  public:
    RpmCallbacks()
	: _step_counter( 0 )
    {
      _messageReceiver.connect();
      _scriptReceiver.connect();
      _installReceiver.connect();
      _removeReceiver.connect();
    }

    ~RpmCallbacks()
    {
      _messageReceiver.disconnect();
      _scriptReceiver.disconnect();
      _installReceiver.disconnect();
      _removeReceiver.disconnect();
    }
};

#endif // ZMD_BACKEND_RPMCALLBACKS_H
// Local Variables:
// mode: c++
// c-basic-offset: 2
// End:
