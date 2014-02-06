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

#include <zypp/base/Logger.h>
#include <zypp/base/String.h>
#include <zypp/sat/Queue.h>
#include <zypp/sat/FileConflicts.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/Package.h>
#include <zypp/Patch.h>

#include "Zypper.h"
#include "output/prompt.h"


static bool report_again( timespec & lastTime_r, int & lastValue_r, int currentValue_r )
{
  // Don't report more often than 5 times per sec,
  // but also leaving 0%, more than 20% step and reaching 100%
  timespec now;
  clock_gettime( CLOCK_REALTIME_COARSE, &now );
  if ( now.tv_sec > lastTime_r.tv_sec
    || ( now.tv_sec == lastTime_r.tv_sec && now.tv_nsec > lastTime_r.tv_nsec + 200000000L )
    || ( lastValue_r != currentValue_r && ( lastValue_r + 20 <= currentValue_r || lastValue_r == 0 || currentValue_r == 100 ) ) )
  {
    lastTime_r = now;
    lastValue_r = currentValue_r;
    return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////
namespace out
{
  ///////////////////////////////////////////////////////////////////
  /// \class FileConflictsListFormater
  /// \brief Printing FileConflicts in List
  ///////////////////////////////////////////////////////////////////
  struct FileConflictsListFormater
  {
    typedef out::DefaultGapedListLayout ListLayout;

    struct XmlFormater
    {
      std::string operator()( const sat::FileConflicts::Conflict & val_r ) const
      { str::Str str; dumpAsXmlOn( str.stream(), val_r ); return str; }
    };

    std::string operator()( const sat::FileConflicts::Conflict & val_r ) const
    { return asUserString( val_r ); }
  };
  ///////////////////////////////////////////////////////////////////

  /** \relates SolvableListFormater Conversion to sat::Solvable */
  template <class _Tp>
  sat::Solvable asSolvable( const _Tp & val_r )
  { return sat::asSolvable( val_r ); }

  sat::Solvable asSolvable( int val_r )		// e.g. satQueues use int as SolvabeId
  { return sat::Solvable( val_r ); }

  ///////////////////////////////////////////////////////////////////
  /// \class SolvableListFormater
  /// \brief Printing Solvable based types in List (legacy format used in summary)
  ///////////////////////////////////////////////////////////////////
  struct SolvableListFormater
  {
    typedef out::CompressedListLayout ListLayout;

    struct XmlFormater
    {
      template <class _Tp>
      std::string operator()( const _Tp & val_r ) const
      { return operator()( makeResObject( asSolvable( val_r ) ) ); }

      std::string operator()( ResObject::Ptr val_r, ResObject::Ptr old_r = nullptr ) const
      {
	str::Str ret;
	ret << "<solvable";
	ret << " type=\""	<< val_r->kind() << "\"";
	ret << " name=\""	<< val_r->name() << "\"";
	ret << " edition=\""	<< val_r->edition() << "\"";
	ret << " arch=\""	<< val_r->arch() << "\"";
	{
	  std::string text( val_r->summary() );
	  if ( ! text.empty() )
	    ret << " summary=\"" << xml::escape( text ) << "\"";
	}
	{ // legacy: description as solvable:PCDATA instead of a subnode
	  std::string text( val_r->description() );
	  if ( ! text.empty() )
	    ret << ">\n" << xml::escape( text ) << "</solvable>";
	  else
	    ret << "/>";
	}
	return ret;
      }
    };

    template <class _Tp>
    std::string operator()( const _Tp & val_r ) const
    { return operator()( makeResObject( asSolvable( val_r ) ) ); }

    std::string operator()( ResObject::Ptr val_r, ResObject::Ptr old_r = nullptr ) const
    { return val_r->ident().asString(); }
  };

} // namespace out
///////////////////////////////////////////////////////////////////

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
  zypp::DefaultIntegral<int,0> _last_percent;

  virtual void start( zypp::Resolvable::constPtr resolvable )
  {
    Zypper & zypper = *Zypper::instance();
    ::clock_gettime(CLOCK_REALTIME_COARSE, &_last_reported);
    _last_percent.reset();
    _label = zypp::str::form("(%*d/%d) ", (int)zypp::str::asString(zypper.runtimeData().rpm_pkgs_total).length(),
                             ++zypper.runtimeData().rpm_pkg_current,
                             zypper.runtimeData().rpm_pkgs_total );
    // translators: This text is a progress display label e.g. "Removing packagename-x.x.x [42%]"
    _label += boost::str(boost::format(_("Removing %s-%s"))
        % resolvable->name() % resolvable->edition());
    Zypper::instance()->out().progressStart("remove-resolvable", _label);
  }

  virtual bool progress( int value, zypp::Resolvable::constPtr resolvable )
  {
    // don't report too often
    if ( report_again( _last_reported, _last_percent, value ) )
    {
      Zypper::instance()->out().progress( "remove-resolvable", _label, value );
    }
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

///////////////////////////////////////////////////////////////////
// progress for installing a resolvable
struct InstallResolvableReportReceiver : public zypp::callback::ReceiveReport<zypp::target::rpm::InstallResolvableReport>
{
  zypp::Resolvable::constPtr _resolvable;
  std::string _label;
  timespec _last_reported;
  zypp::DefaultIntegral<int,0> _last_percent;

  virtual void start( zypp::Resolvable::constPtr resolvable )
  {
    Zypper & zypper = *Zypper::instance();
    clock_gettime(CLOCK_REALTIME_COARSE, &_last_reported);
    _last_percent.reset();
    _resolvable = resolvable;
    _label = zypp::str::form("(%*d/%d) ", (int)zypp::str::asString(zypper.runtimeData().rpm_pkgs_total).length(),
                              ++zypper.runtimeData().rpm_pkg_current,
                              zypper.runtimeData().rpm_pkgs_total );
    // TranslatorExplanation This text is a progress display label e.g. "Installing: foo-1.1.2 [42%]"
    _label += boost::str(boost::format(_("Installing: %s-%s"))
        % resolvable->name() % resolvable->edition());
    Zypper::instance()->out().progressStart("install-resolvable", _label);
  }

  virtual bool progress( int value, zypp::Resolvable::constPtr resolvable )
  {
    // don't report too often
    if ( report_again( _last_reported, _last_percent, value ) )
    {
      Zypper::instance()->out().progress( "install-resolvable", _label, value );
    }
    return true;
  }

  virtual Action problem( zypp::Resolvable::constPtr resolvable, Error error, const std::string & description, RpmLevel /*unused*/ )
  {
    Zypper::instance()->out().progressEnd("install-resolvable", _label, true);
    std::ostringstream s;
    s << boost::format(_("Installation of %s-%s failed:")) % resolvable->name() % resolvable->edition() << std::endl;
    s << zcb_error2str(error, description);
    Zypper::instance()->out().error(s.str());

    return (Action) read_action_ari (PROMPT_ARI_RPM_INSTALL_PROBLEM, ABORT);
  }

  virtual void finish( zypp::Resolvable::constPtr /*resolvable*/, Error error, const std::string & reason, RpmLevel /*unused*/ )
  {
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
/// \class FindFileConflictstReportReceive
/// \brief
///////////////////////////////////////////////////////////////////
struct FindFileConflictstReportReceiver : public zypp::callback::ReceiveReport<zypp::target::FindFileConflictstReport>
{
  FindFileConflictstReportReceiver()
  : _progressId( "fileconflict-check" )
  // TranslatorExplanation A progressbar label.
  , _progressLabel(_("Checking for file conflicts:") )\
  {}

  virtual void reportbegin()
  {
    _progress.reset( new Out::ProgressBar( Zypper::instance()->out(), _progressLabel ) );
  }

  virtual bool start( const ProgressData & progress_r )
  {
    (*_progress)->set( progress_r );
    return !Zypper::instance()->exitRequested();
  }

  virtual bool progress( const ProgressData & progress_r, const sat::Queue & noFilelist_r )
  {
    (*_progress)->set( progress_r );
    return !Zypper::instance()->exitRequested();
  }

  virtual bool result( const ProgressData & progress_r, const sat::Queue & noFilelist_r, const sat::FileConflicts & conflicts_r )
  {
    // finsh progress; only conflicts count as error; missing filelists due
    // to download-as-needed are just a warning. Different behavior breaks KIWI.
    (*_progress).error( !conflicts_r.empty() );
    _progress.reset();

    if ( conflicts_r.empty() && noFilelist_r.empty() )
      return !Zypper::instance()->exitRequested();

    // show error result
    Out & out( Zypper::instance()->out() );
    {
      Out::XmlNode guard( out, "fileconflict-summary" );

      if ( ! noFilelist_r.empty() )	// warning
      {
	out.warning( boost::formatNAC(
		       // TranslatorExplanation %1%(commandline option)
		       _("Checking for file conflicts requires not installed packages to be downloaded in advance "
	                 "in order to access their file lists. See option '%1%' in the zypper manual page for details.")
		     ) % "--download-in-advance" );
	out.gap();

	out.list( "no-filelist",
		  // TranslatorExplanation %1%(number of packages); detailed list follows
		  _PL("The following package had to be excluded from file conflicts check because it is not yet downloaded:",
		      "The following %1% packages had to be excluded from file conflicts check because they are not yet downloaded:",
		      noFilelist_r.size() ),
		  noFilelist_r, out::SolvableListFormater() );
	out.gap();
      }

      if ( ! conflicts_r.empty() )	// error + prompt
      {
	out.list( "fileconflicts",
		  // TranslatorExplanation %1%(number of conflicts); detailed list follows
		  _PL("Detected %1% file conflict:",
		      "Detected %1% file conflicts:",
		      conflicts_r.size() ),
		  conflicts_r, out::FileConflictsListFormater() );
	out.gap();

	if ( Zypper::instance()->cOpts().count("replacefiles") )
	{
	  out.info( _("Conflicting files will be replaced."), " [--replacefiles]" );
	}
	else
	{
	  bool cont = read_bool_answer( PROMPT_YN_CONTINUE_ON_FILECONFLICT, str::Str()
		      // TranslatorExplanation Problem description before asking whether to "Continue? [yes/no] (no):"
		      <<_("File conflicts happen when two packages attempt to install files with the same name but different contents. If you continue, conflicting files will be replaced losing the previous content.")
		      << "\n"
		      <<_("Continue?"),
		      false );
	  out.gap();

	  if ( ! cont )
	    return false;		// aborted.
	}
      }
    }

    return !Zypper::instance()->exitRequested();
  }

  virtual void reportend()
  { _progress.reset(); }

private:
  std::string	_progressId;
  std::string	_progressLabel;
  scoped_ptr<Out::ProgressBar>	_progress;
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
    ZmartRecipients::FindFileConflictstReportReceiver _fileConflictsReceiver;
    int _step_counter;

  public:
    RpmCallbacks()
	: _step_counter( 0 )
    {
      _messageReceiver.connect();
      _scriptReceiver.connect();
      _installReceiver.connect();
      _removeReceiver.connect();
      _fileConflictsReceiver.connect();
    }

    ~RpmCallbacks()
    {
      _messageReceiver.disconnect();
      _scriptReceiver.disconnect();
      _installReceiver.disconnect();
      _removeReceiver.disconnect();
      _fileConflictsReceiver.disconnect();
    }
};

#endif // ZMD_BACKEND_RPMCALLBACKS_H
// Local Variables:
// mode: c++
// c-basic-offset: 2
// End:
