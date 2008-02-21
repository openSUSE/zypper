/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZMART_RPM_CALLBACKS_H
#define ZMART_RPM_CALLBACKS_H

#include <iostream>
#include <sstream>

#include <boost/format.hpp>

#include "zypp/base/Logger.h"
#include "zypp/ZYppCallbacks.h"
#include "zypp/Package.h"
//#include "zypp/target/rpm/RpmCallbacks.h"

#include "zypper.h"
#include "zypper-callbacks.h"
#include "AliveCursor.h"

using namespace std;

///////////////////////////////////////////////////////////////////
namespace ZmartRecipients
{


// resolvable Message
struct MessageResolvableReportReceiver : public zypp::callback::ReceiveReport<zypp::target::MessageResolvableReport>
{
  virtual void show( zypp::Message::constPtr message )
  {
    
    if ( !Zypper::instance()->globalOpts().machine_readable )
    {
      cout_v << message << endl; // [message]important-msg-1.0-1
      cout_n << message->text() << endl;
      return;
    }
    
    cout << "<message type=\"info\">" << message->text() << "</message>" << endl;
    
    //! \todo in interactive mode, wait for ENTER?
  }
};

ostream& operator<< (ostream& stm, zypp::target::ScriptResolvableReport::Task task) {
  return stm << (task==zypp::target::ScriptResolvableReport::DO? "DO": "UNDO");
}

struct ScriptResolvableReportReceiver : public zypp::callback::ReceiveReport<zypp::target::ScriptResolvableReport>
{

  /** task: Whether executing do_script on install or undo_script on delete. */
  virtual void start( const zypp::Resolvable::constPtr & script_r,
		      const zypp::Pathname & path_r,
		      Task task) {
    // TranslatorExplanation speaking of a script
    cout_n << boost::format(_("Running: %s  (%s, %s)"))
        % script_r % task % path_r << endl;
  }
  /** Progress provides the script output. If the script is quiet ,
   * from time to time still-alive pings are sent to the ui. (Notify=PING)
   * Returning \c FALSE
   * aborts script execution.
   */
  virtual bool progress( Notify kind, const std::string &output ) {
    if (kind == PING) {
      static AliveCursor cursor;
      cout_v << '\r' << cursor++ << flush;
    }
    else {
      cout_n << output << flush;
    }
    // hmm, how to signal abort in zypper? catch sigint? (document it)
    return true;
  }
  /** Report error. */
  virtual void problem( const std::string & description ) {
    display_done ( "run-script", cout_n);
    cerr << description << endl;
  }

  /** Report success. */
  virtual void finish() {
    display_done ("run-script", cout_n);
  }

};

///////////////////////////////////////////////////////////////////
struct ScanRpmDbReceive : public zypp::callback::ReceiveReport<zypp::target::rpm::ScanDBReport>
{
  int & _step;				// step counter for install & receive steps
  int last_reported;
  
  ScanRpmDbReceive( int & step )
  : _step( step )
  {
  }

  virtual void start()
  {
    last_reported = 0;
    Zypper::instance()->out().progressStart(
      "read-installed-packages", _("Reading installed packages"));
  }

  virtual bool progress(int value)
  {
    // this is called too often. relax a bit.
    static int last = -1;
    if (last != value)
      Zypper::instance()->out().progress(
        "read-installed-packages", _("Reading installed packages"), value);
    last = value;
    return true;
  }

  virtual Action problem( zypp::target::rpm::ScanDBReport::Error error, const std::string & description )
  {
    Zypper::instance()->out().progressEnd(
      "read-installed-packages", _("Reading installed packages"), true);
    return zypp::target::rpm::ScanDBReport::problem( error, description );
  }

  virtual void finish( Error error, const std::string & reason )
  {
    Zypper::instance()->out()
      .progressEnd("read-installed-packages", _("Reading installed packages"));
    if (error != NO_ERROR)
      Zypper::instance()->out().error(zcb_error2str(error, reason));
  }
};

///////////////////////////////////////////////////////////////////
 // progress for removing a resolvable
struct RemoveResolvableReportReceiver : public zypp::callback::ReceiveReport<zypp::target::rpm::RemoveResolvableReport>
{
  std::string _label;

  virtual void start( zypp::Resolvable::constPtr resolvable )
  {
    // translators: This text is a progress display label e.g. "Removing packagename-x.x.x [42%]"
    _label = boost::str(boost::format(_("Removing %s-%s"))
        % resolvable->name() % resolvable->edition()); 
    Zypper::instance()->out().progressStart("remove-resolvable", _label);
  }

  virtual bool progress(int value, zypp::Resolvable::constPtr resolvable)
  {
    Zypper::instance()->out().progress("remove-resolvable", _label, value);
    return true;
  }

  virtual Action problem( zypp::Resolvable::constPtr resolvable, Error error, const std::string & description )
  {
    Zypper::instance()->out().progressEnd("remove-resolvable", _label, true);
    ostringstream s;
    s << boost::format(_("Removal of %s failed:")) % resolvable << std::endl;
    s << zcb_error2str(error, description);
    Zypper::instance()->out().error(s.str());
    return (Action) read_action_ari (PROMPT_ARI_RPM_REMOVE_PROBLEM, ABORT);
  }

  virtual void finish( zypp::Resolvable::constPtr /*resolvable*/, Error error, const std::string & reason )
  {
    Zypper::instance()->out().progressEnd("remove-resolvable", _label);
    if (error != NO_ERROR)
      Zypper::instance()->out().error(zcb_error2str(error, reason));
  }
};

ostream& operator << (ostream& stm, zypp::target::rpm::InstallResolvableReport::RpmLevel level) {
  static const char * level_s[] = {
    // TranslatorExplanation --nodeps and --force are options of the rpm command, don't translate 
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

  void display_step( zypp::Resolvable::constPtr resolvable, int value )
  {
  }

  virtual void start( zypp::Resolvable::constPtr resolvable )
  {
    _resolvable = resolvable;
    // TranslatorExplanation This text is a progress display label e.g. "Installing foo-1.1.2 [42%]"
    _label = boost::str(boost::format(_("Installing: %s-%s"))
        % resolvable->name() % resolvable->edition());
    Zypper::instance()->out().progressStart("install-resolvable", _label);
  }

  virtual bool progress(int value, zypp::Resolvable::constPtr resolvable)
  {
    Zypper::instance()->out().progress("install-resolvable", _label, value);
    return true;
  }

  virtual Action problem( zypp::Resolvable::constPtr resolvable, Error error, const std::string & description, RpmLevel level )
  {
    if (level < RPM_NODEPS_FORCE)
    {
      DBG << "Install failed, will retry more aggressively"
             " (with --no-deps, --force)." << std::endl;
      return ABORT;
    }

    Zypper::instance()->out().progressEnd("install-resolvable", _label, true);
    ostringstream s;
    s << boost::format(_("Installation of %s failed:")) % resolvable << std::endl;
    s << level << " " << zcb_error2str(error, description);
    Zypper::instance()->out().error(s.str());

    return (Action) read_action_ari (PROMPT_ARI_RPM_INSTALL_PROBLEM, ABORT);
  }

  virtual void finish( zypp::Resolvable::constPtr /*resolvable*/, Error error, const std::string & reason, RpmLevel level )
  {
    if (error != NO_ERROR && level < RPM_NODEPS_FORCE)
    {
      DBG << "level < RPM_NODEPS_FORCE: aborting without displaying an error"
          << endl;
      return;
    }

    Zypper::instance()->out().progressEnd("remove-resolvable", _label);
    if (error != NO_ERROR)
    {
      ostringstream s;
      s << level << " " << zcb_error2str(error, reason);
      Zypper::instance()->out().error(s.str());
    }
  }
};


///////////////////////////////////////////////////////////////////
}; // namespace ZyppRecipients
///////////////////////////////////////////////////////////////////

class RpmCallbacks {

  private:
    ZmartRecipients::MessageResolvableReportReceiver _messageReceiver;
    ZmartRecipients::ScriptResolvableReportReceiver _scriptReceiver;
    ZmartRecipients::ScanRpmDbReceive _readReceiver;
    ZmartRecipients::RemoveResolvableReportReceiver _installReceiver;
    ZmartRecipients::InstallResolvableReportReceiver _removeReceiver;
    int _step_counter;

  public:
    RpmCallbacks()
	: _readReceiver( _step_counter )
	//, _removeReceiver( _step_counter )
	, _step_counter( 0 )
    {
      _messageReceiver.connect();
      _scriptReceiver.connect();
      _installReceiver.connect();
      _removeReceiver.connect();
      _readReceiver.connect();
    }

    ~RpmCallbacks()
    {
      _messageReceiver.disconnect();
      _scriptReceiver.disconnect();
      _installReceiver.disconnect();
      _removeReceiver.disconnect();
      _readReceiver.connect();
    }
};

#endif // ZMD_BACKEND_RPMCALLBACKS_H
// Local Variables:
// mode: c++
// c-basic-offset: 2
// End:
