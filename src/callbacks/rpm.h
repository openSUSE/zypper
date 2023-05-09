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
#include <zypp/base/IOStream.h>
#include <zypp/base/StringV.h>
#include <zypp/base/Regex.h>
#include <zypp/sat/Queue.h>
#include <zypp/sat/FileConflicts.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/Package.h>
#include <zypp/Patch.h>

#include "Zypper.h"
#include "output/prompt.h"
#include "global-settings.h"

///////////////////////////////////////////////////////////////////
namespace
{
  using loglevel = zypp::target::rpm::SingleTransReport::loglevel;

  /** Print additional rpm outout line and scan for %script errors. */
  inline void processAdditionalRpmOutputLine( const std::string & line_r, loglevel level_r = loglevel::msg, const char *const prefix_r = nullptr )
  {
    Out::Info info( Zypper::instance().out() );
    ColorStream msg( info << "", ColorContext::HIGHLIGHT );

    std::string_view msgline { zypp::strv::rtrim( line_r, "\n" ) };

    if ( prefix_r ) {
      // Checking for ZYPPER_EXIT_INF_RPM_SCRIPT_FAILED (like in the traditional rpv case below)
      // is not needed here. The script callback callback handles this.
      switch ( level_r ) {
        case loglevel::crt: [[fallthrough]];
        case loglevel::err:
          msg << ( ColorContext::MSG_ERROR << prefix_r<<msgline );
          break;
        case loglevel::war:
          msg << ( ColorContext::MSG_WARNING << prefix_r<<msgline );
          break;
        case loglevel::msg:
          msg << prefix_r<<msgline;
          break;
        case loglevel::dbg:
          msg << ( ColorContext::LOWLIGHT << prefix_r<<msgline );
          break;
      }
    }
    else {
      // a single line with prefix (parsed from traditional rpm calls stdout/stderr)
      static str::regex rx("^(warning|error): %.* scriptlet failed, ");
      static str::smatch what;
      if ( str::regex_match( line_r, what, rx ) ) {
        Zypper::instance().setExitInfoCode( ZYPPER_EXIT_INF_RPM_SCRIPT_FAILED );
        msg << ( (line_r[0] == 'w' ? ColorContext::MSG_WARNING : ColorContext::MSG_ERROR) << msgline );
      }
      else
        msg << msgline;
    }
  }

  /** Print additional rpm outout and scan for %script errors. */
  void processAdditionalRpmOutput( const std::string & output_r )
  {
    if ( output_r.empty() )
      return;

    if ( output_r.find( '\n' ) == std::string::npos ) {
      // singleline
      processAdditionalRpmOutputLine( output_r );
    }
    else {
      // multiline
      std::istringstream input( output_r );
      for ( iostr::EachLine in( input ); in; in.next() )
        processAdditionalRpmOutputLine( *in );
    }
  }

} // namespace
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace out
{
  ///////////////////////////////////////////////////////////////////
  /// \class FileConflictsListFormater
  /// \brief Printing FileConflicts in List
  ///////////////////////////////////////////////////////////////////
  struct FileConflictsListFormater
  {
    typedef out::DefaultGapedListLayout NormalLayout;

    std::string xmlListElement( const sat::FileConflicts::Conflict & val_r ) const
    { str::Str str; dumpAsXmlOn( str.stream(), val_r ); return str; }

    std::string listElement( const sat::FileConflicts::Conflict & val_r ) const
    { return asUserString( val_r ); }
  };
  ///////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////
  /// \class SolvableListFormater
  /// \brief Printing Solvable based types in List (legacy format used in summary)
  ///////////////////////////////////////////////////////////////////
  struct SolvableListFormater
  {
    typedef out::CompressedListLayout NormalLayout;

    template <class Tp_>
    std::string xmlListElement( const Tp_ & val_r ) const
    { return xmlListElement( makeResObject( asSolvable( val_r ) ) ); }

    std::string xmlListElement( const ResObject::Ptr & val_r ) const
    {
      str::Str ret;
      ret << "<solvable";
      ret << " type=\""	<< val_r->kind() << "\"";
      ret << " name=\""	<< val_r->name() << "\"";
      ret << " edition=\""	<< val_r->edition() << "\"";
      ret << " arch=\""	<< val_r->arch() << "\"";
      {
        const std::string & text( val_r->summary() );
        if ( ! text.empty() )
          ret << " summary=\"" << xml::escape( text ) << "\"";
      }
      {
        const std::string & text( val_r->description() );
        if ( ! text.empty() )
          ret << ">\n" << "<description>" << xml::escape( text ) << "</description>" << "</solvable>";
        else
          ret << "/>";
      }
      return ret;
    }

    template <class Tp>
    std::string listElement( const Tp & val_r ) const
    { return listElement( makeResObject( asSolvable( val_r ) ) ); }

    std::string listElement( const ResObject::Ptr & val_r ) const
    { return val_r->ident().asString(); }

  private:
    template <class Tp_>
    static sat::Solvable asSolvable( const Tp_ & val_r )
    { return sat::asSolvable()( val_r ); }

    static sat::Solvable asSolvable( int val_r )		// e.g. satQueues use int as SolvabeId
    { return sat::Solvable( val_r ); }
  };

} // namespace out
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
namespace ZmartRecipients
{

// resolvable Message
struct PatchMessageReportReceiver : public callback::ReceiveReport<target::PatchMessageReport>
{

  /** Display \c patch->message().
   * Return \c true to continue, \c false to abort commit.
   */
  virtual bool show( Patch::constPtr & patch )
  {
    Out & out = Zypper::instance().out();
    std::ostringstream s;
    s << patch; // [patch]important-patch-101 \todo make some meaningful message out of this
    out.info(s.str(), Out::HIGH);
    out.info(patch->message());

    return read_bool_answer(PROMPT_PATCH_MESSAGE_CONTINUE, text::qContinue(), true);
  }
};


struct PatchScriptReportReceiver : public callback::ReceiveReport<target::PatchScriptReport>
{
  std::string _label;
  scoped_ptr<Out::XmlNode> _guard;	// guard script output if Out::TYPE_XML

  void closeNode()
  { if ( _guard ) _guard.reset(); }

  std::ostream & printOut( const std::string & output_r )
  {
    if ( _guard )
      std::cout << xml::escape( output_r );
    else
      std::cout << output_r;
    return std::cout;
  }


  virtual void start( const Package::constPtr & package,
                      const Pathname & path_r ) // script path
  {
    Zypper & zypper = Zypper::instance();
    if ( zypper.out().type() == Out::TYPE_XML )
      _guard.reset( new Out::XmlNode( zypper.out(), "message", { "type", "info" } ) );

    // TranslatorExplanation speaking of a script - "Running: script file name (package name, script dir)"
    _label = str::Format(_("Running: %s  (%s, %s)")) % path_r.basename() % package->name() % path_r.dirname();
    printOut( _label ) << std::endl;
  }

  /**
   * Progress provides the script output. If the script is quiet,
   * from time to time still-alive pings are sent to the ui. (Notify=PING)
   * Returning \c FALSE aborts script execution.
   */
  virtual bool progress( Notify kind, const std::string &output )
  {
    Zypper & zypper = Zypper::instance();
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
      printOut( output );

      was_ping_before = false;
    }

    return !zypper.exitRequested();
  }

  /** Report error. */
  virtual Action problem( const std::string & description )
  {
    closeNode();
    Zypper & zypper = Zypper::instance();

    zypper.out().error(description);

    Action action = (Action) read_action_ari (PROMPT_ARI_PATCH_SCRIPT_PROBLEM, ABORT);
    if (action == target::PatchScriptReport::ABORT)
      zypper.setExitCode(ZYPPER_EXIT_ERR_ZYPP);
    return action;
  }

  /** Report success. */
  virtual void finish()
  {
    closeNode();
    Zypper::instance().out().progressEnd("run-script", _label);
  }

  /** Catch unexpected end. */
  virtual void reportend()
  { closeNode(); }
};

///////////////////////////////////////////////////////////////////
 // progress for removing a resolvable
struct RemoveResolvableReportReceiver : public callback::ReceiveReport<target::rpm::RemoveResolvableReport>
{
  virtual void start( Resolvable::constPtr resolvable )
  {
    ++Zypper::instance().runtimeData().rpm_pkg_current;
    showProgress( resolvable );
  }

  virtual bool progress( int value, Resolvable::constPtr resolvable )
  {
    if ( _progress )
      (*_progress)->set( value );
    return !Zypper::instance().exitRequested();
  }

  virtual Action problem( Resolvable::constPtr resolvable, Error error, const std::string & description_r )
  {
    std::string description;  // We just need the 1st line (exception message), the rest is additional rpm output we already reported.
    strv::split( description_r, "\n", [&description]( std::string_view line_r, unsigned nr_r ) {
      if ( nr_r == 0 ) description = line_r;
      return false;
    });

    // finsh progress; indicate error
    if ( _progress )
    {
      (*_progress).error();
      _progress.reset();
    }

    std::ostringstream s;
    s << str::Format(_("Removal of %s failed:")) % resolvable << std::endl;
    s << zcb_error2str(error, description);
    Zypper::instance().out().error(s.str());

    Action ret = (Action) read_action_ari (PROMPT_ARI_RPM_REMOVE_PROBLEM, ABORT);
    if ( ret == RETRY )
      showProgress( resolvable );	// bsc#1131113: need to re-show progress

    return ret;
  }

  virtual void finish( Resolvable::constPtr /*resolvable*/, Error error, const std::string & reason )
  {
    // finsh progress; indicate error
    if ( _progress )
    {
      (*_progress).error( error != NO_ERROR );
      _progress.reset();
    }

    if (error != NO_ERROR)
      // set proper exit code, don't write to output, the error should have been reported in problem()
      Zypper::instance().setExitCode(ZYPPER_EXIT_ERR_ZYPP);
  }

  void report( const UserData & userData_r ) override
  {
    // Any additional rpm output is printed immediately....
    if ( userData_r.type() == ReportType::contentRpmout && userData_r.haskey("line") ) {
      processAdditionalRpmOutput( userData_r.get<std::reference_wrapper<const std::string>>("line").get() );
   }
  }

  virtual void reportend()
  { _progress.reset(); }

private:
  void showProgress( Resolvable::constPtr resolvable_r )
  {
    Zypper & zypper = Zypper::instance();
    _progress.reset( new Out::ProgressBar( zypper.out(),
                                           "remove-resolvable",
                                           // translators: This text is a progress display label e.g. "Removing packagename-x.x.x [42%]"
                                           str::Format(_("Removing: %s") ) % resolvable_r->asString(),
                                           zypper.runtimeData().rpm_pkg_current,
                                           zypper.runtimeData().rpm_pkgs_total ) );
    (*_progress)->range( 100 );	// progress reports percent
  }

private:
  scoped_ptr<Out::ProgressBar>	_progress;
};

///////////////////////////////////////////////////////////////////
// progress for installing a resolvable
struct InstallResolvableReportReceiver : public callback::ReceiveReport<target::rpm::InstallResolvableReport>
{
  virtual void start( Resolvable::constPtr resolvable )
  {
    ++Zypper::instance().runtimeData().rpm_pkg_current;
    showProgress( resolvable );
  }

  virtual bool progress( int value, Resolvable::constPtr resolvable )
  {
    if ( _progress )
      (*_progress)->set( value );
    return !Zypper::instance().exitRequested();
  }

  virtual Action problem( Resolvable::constPtr resolvable, Error error, const std::string & description_r, RpmLevel /*unused*/ )
  {
    std::string description;  // We just need the 1st line (exception message), the rest is additional rpm output we already reported.
    strv::split( description_r, "\n", [&description]( std::string_view line_r, unsigned nr_r ) {
      if ( nr_r == 0 ) description = line_r;
      return false;
    });

    // finsh progress; indicate error
    if ( _progress )
    {
      (*_progress).error();
      _progress.reset();
    }

    std::ostringstream s;
    s << ( str::Format(_("Installation of %s failed:") ) % resolvable->asString() ) << std::endl;
    s << zcb_error2str(error, description);
    Zypper::instance().out().error(s.str());

    Action ret = (Action) read_action_ari (PROMPT_ARI_RPM_INSTALL_PROBLEM, ABORT);
    if ( ret == RETRY )
      showProgress( resolvable );	// bsc#1131113: need to re-show progress

    return ret;
  }

  virtual void finish( Resolvable::constPtr /*resolvable*/, Error error, const std::string & reason, RpmLevel /*unused*/ )
  {
    // finsh progress; indicate error
    if ( _progress )
    {
      (*_progress).error( error != NO_ERROR );
      _progress.reset();
    }

    if ( error != NO_ERROR )
      // don't write to output, the error should have been reported in problem() (bnc #381203)
      Zypper::instance().setExitCode(ZYPPER_EXIT_ERR_ZYPP);
  }

  void report( const UserData & userData_r ) override
  {
    // Any additional rpm output is printed immediately....
    if ( userData_r.type() == ReportType::contentRpmout && userData_r.haskey("line") ) {
      processAdditionalRpmOutput( userData_r.get<std::reference_wrapper<const std::string>>("line").get() );
   }
  }

  virtual void reportend()
  { _progress.reset(); }

private:
  void showProgress( Resolvable::constPtr resolvable_r )
  {
    Zypper & zypper = Zypper::instance();
    _progress.reset( new Out::ProgressBar( zypper.out(),
                                           "install-resolvable",
                                           // TranslatorExplanation This text is a progress display label e.g. "Installing: foo-1.1.2 [42%]"
                                           str::Format(_("Installing: %s") ) % resolvable_r->asString(),
                                           zypper.runtimeData().rpm_pkg_current,
                                           zypper.runtimeData().rpm_pkgs_total ) );
    (*_progress)->range( 100 );
  }

private:
  scoped_ptr<Out::ProgressBar>	_progress;
};

///////////////////////////////////////////////////////////////////
/// \class FindFileConflictstReportReceive
/// \brief
///////////////////////////////////////////////////////////////////
struct FindFileConflictstReportReceiver : public callback::ReceiveReport<target::FindFileConflictstReport>
{
  static std::string mkProgressBarLabel( unsigned skipped_r = 0 )
  {
    // translators: A progressbar label
    std::string ret { _("Checking for file conflicts:") };
    if ( skipped_r ) {
      // translators: progressbar label extension; %1% is the number of skipped items
      static str::Format fmt { MSG_WARNINGString(" (%1% skipped)" ).str() };
      ret += fmt % skipped_r;
    }
    return ret;
  }

  virtual void reportbegin()
  {
    Zypper::instance().out().gap();
    _lastskip = 0;
    _progress.reset( new Out::ProgressBar( Zypper::instance().out(),
                                           "fileconflict-check",
                                           mkProgressBarLabel() ) );
  }

  virtual bool start( const ProgressData & progress_r )
  {
    (*_progress)->set( progress_r );
    return !Zypper::instance().exitRequested();
  }

  virtual bool progress( const ProgressData & progress_r, const sat::Queue & noFilelist_r )
  {
    (*_progress)->set( progress_r );
    if ( noFilelist_r.size() != _lastskip ) {
      (*_progress).print( mkProgressBarLabel( (_lastskip = noFilelist_r.size()) ) );
    }
    return !Zypper::instance().exitRequested();
  }

  virtual bool result( const ProgressData & progress_r, const sat::Queue & noFilelist_r, const sat::FileConflicts & conflicts_r )
  {
    // finsh progress; only conflicts count as error; missing filelists due
    // to download-as-needed are just a warning. Different behavior breaks KIWI.
    (*_progress).error( !conflicts_r.empty() );
    _progress.reset();

    if ( conflicts_r.empty() && noFilelist_r.empty() )
      return !Zypper::instance().exitRequested();

    // show error result
    Out & out( Zypper::instance().out() );
    {
      Out::XmlNode guard( out, "fileconflict-summary" );

      if ( ! noFilelist_r.empty() )	// warning
      {
        out.warning( str::Format( // TranslatorExplanation %1%(number of packages); detailed list follows
                                  PL_( "%1% package had to be excluded from file conflicts check because it is not yet download.",
                                       "%1% packages had to be excluded from file conflicts check because they are not yet downloaded.",
                                       noFilelist_r.size() ) ) % noFilelist_r.size() );

        out.notePar( 4, str::Format(
                       // TranslatorExplanation %1%(commandline option)
                       _("Checking for file conflicts requires not installed packages to be downloaded in advance "
                         "in order to access their file lists. See option '%1%' in the zypper manual page for details.")
                     ) % "--download-in-advance / --dry-run --download-only" );
        out.gap();
      }

      if ( ! conflicts_r.empty() )	// error + prompt
      {
        out.list( "fileconflicts",
                  // TranslatorExplanation %1%(number of conflicts); detailed list follows
                  PL_("Detected %1% file conflict:",
                      "Detected %1% file conflicts:",
                      conflicts_r.size() ),
                  conflicts_r, out::FileConflictsListFormater() );
        out.gap();

        if ( FileConflictPolicy::instance()._replaceFiles )
        {
          out.info( _("Conflicting files will be replaced."), " [--replacefiles]" );
        }
        else
        {
          bool cont = read_bool_answer( PROMPT_YN_CONTINUE_ON_FILECONFLICT, str::Str()
                      // TranslatorExplanation Problem description before asking whether to "Continue? [yes/no] (no):"
                      <<_("File conflicts happen when two packages attempt to install files with the same name but different contents. If you continue, conflicting files will be replaced losing the previous content.")
                      << "\n"
                      << text::qContinue(),
                      false );
          out.gap();

          if ( ! cont )
            return false;		// aborted.
        }
      }
    }

    return !Zypper::instance().exitRequested();
  }

  virtual void reportend()
  { _progress.reset(); }

private:
  scoped_ptr<Out::ProgressBar>	_progress;
  unsigned _lastskip = 0;
};


///////////////////////////////////////////////////////////////////
/// \brief Report active throughout the whole rpm transaction.
///////////////////////////////////////////////////////////////////
struct SingleTransReportReceiver : public callback::ReceiveReport<target::rpm::SingleTransReport>
{
  void report( const UserData & userData_r ) override
  {
    if ( userData_r.type() == ReportType::contentLogline ) {
      const std::string & line { userData_r.get<std::reference_wrapper<const std::string>>("line").get() };
      ReportType::loglevel level { userData_r.get<ReportType::loglevel>("level") };
      processAdditionalRpmOutputLine( line, level, ReportType::loglevelPrefix( level ) );
    }
  }
};

///////////////////////////////////////////////////////////////////
 // progress for removing a resolvable during a single transaction
struct RemoveResolvableSAReportReceiver : public callback::ReceiveReport<target::rpm::RemoveResolvableReportSA>
{
  void start(
          Resolvable::constPtr resolvable,
          const UserData & /*userdata*/ ) override
  {
    ++Zypper::instance().runtimeData().rpm_pkg_current;
    showProgress( resolvable );
  }

  void progress(
          int value,
          Resolvable::constPtr resolvable,
          const UserData & /*userdata*/  ) override
  {
    if ( _progress )
      (*_progress)->set( value );
  }

  void finish( Resolvable::constPtr /*resolvable*/, Error error, const UserData & /*userdata*/ ) override
  {
    // finsh progress; indicate error
    if ( _progress )
    {
      (*_progress).error( error != NO_ERROR );
      _progress.reset();
    }

    if (error != NO_ERROR)
      // set proper exit code, don't write to output, the error should have been reported in problem()
      Zypper::instance().setExitCode(ZYPPER_EXIT_ERR_ZYPP);
  }

  void report( const UserData & userData_r ) override
  {
    if ( userData_r.type() == ReportType::contentRpmout
          &&  userData_r.haskey("line") ) {
            std::string line;
            if ( userData_r.get("line", line) ) {
              processAdditionalRpmOutput( line );
            }
    }
  }

  void reportend() override
  { _progress.reset(); }

private:
  void showProgress( Resolvable::constPtr resolvable_r )
  {
    Zypper & zypper = Zypper::instance();
    _progress.reset( new Out::ProgressBar( zypper.out(),
                                           "remove-resolvable",
                                           // translators: This text is a progress display label e.g. "Removing packagename-x.x.x [42%]"
                                           str::Format(_("Removing: %s") ) % resolvable_r->asString(),
                                           zypper.runtimeData().rpm_pkg_current,
                                           zypper.runtimeData().rpm_pkgs_total ) );
    (*_progress)->range( 100 );	// progress reports percent
  }

private:
  scoped_ptr<Out::ProgressBar>	_progress;
};

///////////////////////////////////////////////////////////////////
// progress for installing a resolvable during a single transaction
struct InstallResolvableSAReportReceiver : public callback::ReceiveReport<target::rpm::InstallResolvableReportSA>
{
  void start( Resolvable::constPtr resolvable, const UserData & /*userdata*/ ) override
  {
    ++Zypper::instance().runtimeData().rpm_pkg_current;
    showProgress( resolvable );
  }

  void progress( int value, Resolvable::constPtr resolvable, const UserData & /*userdata*/ ) override
  {
    if ( _progress )
      (*_progress)->set( value );
  }

  void finish( Resolvable::constPtr /*resolvable*/, Error error, const UserData & /*userdata*/ ) override
  {
    // finsh progress; indicate error
    if ( _progress )
    {
      (*_progress).error( error != NO_ERROR );
      _progress.reset();
    }

    if ( error != NO_ERROR )
      // don't write to output, the error should have been reported in problem() (bnc #381203)
      Zypper::instance().setExitCode(ZYPPER_EXIT_ERR_ZYPP);
  }

  void report( const UserData & userData_r ) override
  {
    if ( userData_r.type() == ReportType::contentRpmout
          &&  userData_r.haskey("line") ) {
      std::string line;
      if ( userData_r.get("line", line) ) {
        processAdditionalRpmOutput( line );
      }
    }
  }

  void reportend() override
  { _progress.reset(); }

private:
  void showProgress( Resolvable::constPtr resolvable_r )
  {
    Zypper & zypper = Zypper::instance();
    _progress.reset( new Out::ProgressBar( zypper.out(),
                                           "install-resolvable",
                                           // TranslatorExplanation This text is a progress display label e.g. "Installing: foo-1.1.2 [42%]"
                                           str::Format(_("Installing: %s") ) % resolvable_r->asString(),
                                           zypper.runtimeData().rpm_pkg_current,
                                           zypper.runtimeData().rpm_pkgs_total ) );
    (*_progress)->range( 100 );
  }

private:
  scoped_ptr<Out::ProgressBar>	_progress;
};

///////////////////////////////////////////////////////////////////
// progress for executing a commit script during a transaction
struct CommitScriptReportSAReportReceiver : public callback::ReceiveReport<target::rpm::CommitScriptReportSA>
{
  void start(
          const std::string &  scriptType,
          const std::string &  packageName,
          Resolvable::constPtr resolvable,
          const UserData & /*userdata*/ ) override
  {
    showProgress( scriptType, packageName, resolvable );
  }

  void progress( int value, Resolvable::constPtr resolvable, const UserData & /*userdata*/ ) override
  {
    if ( _progress )
      (*_progress)->set( value );
  }

  void finish( Resolvable::constPtr /*resolvable*/, Error error, const UserData & /*userdata*/ ) override
  {
    // finsh progress; indicate error
    if ( _progress )
    {
      ProgressEnd donetag { error==NO_ERROR ? ProgressEnd::done : error==CRITICAL ? ProgressEnd::error : ProgressEnd::attention };
      (*_progress).error( donetag );
      _progress.reset();
    }

    if ( error == WARN )
      // don't write to output, the error should have been reported in problem() (bnc #381203)
      Zypper::instance().setExitInfoCode( ZYPPER_EXIT_INF_RPM_SCRIPT_FAILED );
  }

  void report( const UserData & userData_r ) override
  {
    if ( userData_r.type() == ReportType::contentRpmout
          &&  userData_r.haskey("line") ) {
      std::string line;
      if ( userData_r.get("line", line) ) {
        processAdditionalRpmOutput( line );
      }
    }
  }

  void reportend() override
  { _progress.reset(); }

private:
  void showProgress( const std::string &scriptType, const std::string &packageName, Resolvable::constPtr resolvable_r )
  {
    Zypper & zypper = Zypper::instance();

    if ( resolvable_r ) {
      _progress.reset( new Out::ProgressBar( zypper.out(),
                                           "execute-resolvable-script",
                                           // TranslatorExplanation This text is a progress display label e.g. "Installing: foo-1.1.2 [42%]"
                                           str::Format(_("Executing %s script for: %s") ) % scriptType % resolvable_r->asString(),
                                           zypper.runtimeData().rpm_pkg_current,
                                           zypper.runtimeData().rpm_pkgs_total ) );
    } else if ( packageName.size() ) {
      _progress.reset( new Out::ProgressBar( zypper.out(),
                                           "execute-resolvable-script",
                                           // TranslatorExplanation This text is a progress display label e.g. "Installing: foo-1.1.2 [42%]"
                                           str::Format(_("Executing %s script for: %s") ) % scriptType % packageName,
                                           zypper.runtimeData().rpm_pkg_current,
                                           zypper.runtimeData().rpm_pkgs_total ) );
    } else  {
      _progress.reset( new Out::ProgressBar( zypper.out(),
                                           "execute-script",
                                           // TranslatorExplanation This text is a progress display label e.g. "Installing: foo-1.1.2 [42%]"
                                           str::Format( _("Executing %s script") ) % scriptType, zypper.runtimeData().rpm_pkg_current, zypper.runtimeData().rpm_pkgs_total ) );
    }
    (*_progress)->range( 100 );
  }

private:
  scoped_ptr<Out::ProgressBar>	_progress;
};

///////////////////////////////////////////////////////////////////
// progress for generic tasks during a transaction ( used for verifying and preparing )
struct TransactionReportSAReceiver : public callback::ReceiveReport<target::rpm::TransactionReportSA>
{
  void start(
          const std::string &name,
          const UserData & /*userdata*/ ) override
  {
    showProgress( name );
  }

  void progress( int value, const UserData & /*userdata*/ ) override
  {
    if ( _progress )
      (*_progress)->set( value );
  }

  void finish( Error error, const UserData & /*userdata*/ ) override
  {
    // finsh progress; indicate error
    if ( _progress )
    {
      (*_progress).error( error != NO_ERROR );
      _progress.reset();
    }

    if ( error != NO_ERROR )
      // don't write to output, the error should have been reported in problem() (bnc #381203)
      Zypper::instance().setExitCode(ZYPPER_EXIT_ERR_ZYPP);
  }

  void report( const UserData & userData_r ) override
  {
    if ( userData_r.type() == ReportType::contentRpmout
          &&  userData_r.haskey("line") ) {
      std::string line;
      if ( userData_r.get("line", line) ) {
        processAdditionalRpmOutput( line );
      }
    }
  }


  void reportend() override
  { _progress.reset(); }

private:
  void showProgress( const std::string &name )
  {
    Zypper & zypper = Zypper::instance();
    _progress.reset( new Out::ProgressBar( zypper.out(),
        "transaction-prepare", name ) );
    (*_progress)->range( 100 );
  }

private:
  scoped_ptr<Out::ProgressBar>	_progress;
};


///////////////////////////////////////////////////////////////////
// progress for generic tasks during a transaction ( used for verifying and preparing )
struct CleanupPackageReportSAReceiver : public callback::ReceiveReport<target::rpm::CleanupPackageReportSA>
{
  void start(
    const std::string &nvra,
    const UserData & /*userdata*/ ) override
  {
    showProgress( nvra );
  }

  void progress( int value, const UserData & /*userdata*/ ) override
  {
    if ( _progress )
      (*_progress)->set( value );
  }

  void finish( Error error, const UserData & /*userdata*/ ) override
  {
    // finsh progress; indicate error
    if ( _progress )
    {
      (*_progress).error( error != NO_ERROR );
      _progress.reset();
    }

    if ( error != NO_ERROR )
      // don't write to output, the error should have been reported in problem() (bnc #381203)
      Zypper::instance().setExitCode(ZYPPER_EXIT_ERR_ZYPP);
  }

  void report( const UserData & userData_r ) override
  {
    if ( userData_r.type() == ReportType::contentRpmout
         &&  userData_r.haskey("line") ) {
      std::string line;
      if ( userData_r.get("line", line) ) {
        processAdditionalRpmOutput( line );
      }
    }
  }


  void reportend() override
  { _progress.reset(); }

private:
  void showProgress( const std::string &name )
  {
    Zypper & zypper = Zypper::instance();
    _progress.reset( new Out::ProgressBar( zypper.out(),
      "cleanup-task",
      // TranslatorExplanation This text is a progress display label e.g. "Installing: foo-1.1.2 [42%]"
      str::Format(_("Cleaning up: %s") ) % name,
             zypper.runtimeData().rpm_pkg_current,
                                           zypper.runtimeData().rpm_pkgs_total ) );
    (*_progress)->range( 100 );
  }

private:
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

    ZmartRecipients::SingleTransReportReceiver _singleTransaReceiver;  // active throughout the whole rpm transaction
    ZmartRecipients::RemoveResolvableSAReportReceiver _installSaReceiver;
    ZmartRecipients::InstallResolvableSAReportReceiver _removeSaReceiver;
    ZmartRecipients::CommitScriptReportSAReportReceiver _scriptSaReceiver;
    ZmartRecipients::TransactionReportSAReceiver _transReceiver;
    ZmartRecipients::CleanupPackageReportSAReceiver _cleanupReceiver;

  public:
    RpmCallbacks()
    {
      _messageReceiver.connect();
      _scriptReceiver.connect();
      _installReceiver.connect();
      _removeReceiver.connect();
      _fileConflictsReceiver.connect();

      _singleTransaReceiver.connect();
      _installSaReceiver.connect();
      _removeSaReceiver.connect();
      _scriptSaReceiver.connect();
      _transReceiver.connect();
      _cleanupReceiver.connect();
    }

    ~RpmCallbacks()
    {
      _messageReceiver.disconnect();
      _scriptReceiver.disconnect();
      _installReceiver.disconnect();
      _removeReceiver.disconnect();
      _fileConflictsReceiver.disconnect();

      _singleTransaReceiver.disconnect();
      _installSaReceiver.disconnect();
      _removeSaReceiver.disconnect();
      _scriptSaReceiver.disconnect();
      _transReceiver.disconnect();
      _cleanupReceiver.disconnect();
    }
};

#endif // ZMART_RPM_CALLBACKS_H
// Local Variables:
// mode: c++
// c-basic-offset: 2
// End:
