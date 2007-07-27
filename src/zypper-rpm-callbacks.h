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
#include <string>

#include <boost/format.hpp>

#include <zypp/base/Logger.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/Package.h>
//#include <zypp/target/rpm/RpmCallbacks.h>

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
    cout_v << message << endl; // [message]important-msg-1.0-1
    cout_n << message->text() << endl;
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
    display_done ();
    cerr << description << endl;
  }

  /** Report success. */
  virtual void finish() {
    display_done ();
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
    progress (0);
  }

  virtual bool progress(int value)
  {
    // this is called too often. relax a bit.
    static int last = -1;
    if (last != value)
      display_progress (_("Reading installed packages"), value);
    last = value;
    return true;
  }

  virtual Action problem( zypp::target::rpm::ScanDBReport::Error error, const std::string & description )
  {
    return zypp::target::rpm::ScanDBReport::problem( error, description );
  }

  virtual void finish( Error error, const std::string & reason )
  {
    display_done ();
    display_error (error, reason);
  }
};

///////////////////////////////////////////////////////////////////
 // progress for removing a resolvable
struct RemoveResolvableReportReceiver : public zypp::callback::ReceiveReport<zypp::target::rpm::RemoveResolvableReport>
{
  virtual void start( zypp::Resolvable::constPtr resolvable )
  {
    cout << boost::format(_("Removing: %s-%s"))
        % resolvable->name() % resolvable->edition() << endl;
  }

  virtual bool progress(int value, zypp::Resolvable::constPtr resolvable)
  {
    // TranslatorExplanation This text is a progress display label e.g. "Removing [42%]"
    display_progress (_("Removing ") + to_string (resolvable), value);
    return true;
  }

  virtual Action problem( zypp::Resolvable::constPtr resolvable, Error error, const std::string & description )
  {
    cerr << boost::format(_("Removal of %s failed:")) % resolvable << std::endl;
    display_error (error, description);
    return (Action) read_action_ari (ABORT);
  }

  virtual void finish( zypp::Resolvable::constPtr /*resolvable*/, Error error, const std::string & reason )
  {
    display_done ();
    display_error (error, reason);
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
  
  void display_step( zypp::Resolvable::constPtr /*resolvable*/, int value )
  {
    // TranslatorExplanation This text is a progress display label e.g. "Installing [42%]"
    display_progress (_("Installing ") /* + to_string (resolvable) */,  value);
  }

  virtual void start( zypp::Resolvable::constPtr resolvable )
  {
    _resolvable = resolvable;
    if (gSettings.machine_readable)
      cout << "Installing: " + resolvable->name() << endl;
    else
      cout << boost::format(_("Installing: %s-%s"))
        % resolvable->name() % resolvable->edition() << endl;
  }

  virtual bool progress(int value, zypp::Resolvable::constPtr resolvable)
  {
    display_step( resolvable, value );
    return true;
  }

  virtual Action problem( zypp::Resolvable::constPtr resolvable, Error error, const std::string & description, RpmLevel level )
  {
    if (level < RPM_NODEPS_FORCE)
    {
      std::string msg = "Install failed, will retry more aggressively (with --no-deps, --force).";
      cerr_vv << msg << std::endl;
      DBG << msg << std::endl;
      return ABORT;
    }

    cerr << boost::format(_("Installation of %s failed:")) % resolvable
        << std::endl;
    cerr << level << " "; display_error (error, description);

    return (Action) read_action_ari (ABORT);
  }

  virtual void finish( zypp::Resolvable::constPtr /*resolvable*/, Error error, const std::string & reason, RpmLevel level )
  {
    display_done ();
    if (error != NO_ERROR) {
      cerr << level;
    }
    display_error (error, reason);
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
