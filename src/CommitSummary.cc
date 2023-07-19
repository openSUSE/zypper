/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include "CommitSummary.h"

#include "utils/text.h"
#include "utils/colors.h"
#include "utils/misc.h"
#include "Table.h"
#include "Zypper.h"
#include "utils/console.h"

CommitSummary::CommitSummary( const zypp::ZYppCommitResult &result, const ViewOptions options ) :
  _viewop(options),
  _wrap_width( 80 ),
  _result( result )
{ }

void CommitSummary::dumpTo(std::ostream &out)
{
  collectData();
  struct SetColor
  {
    SetColor( bool force )
      : docolors( Zypper::instance().config().do_colors )
    { if (force) Zypper::instance().configNoConst().do_colors = false; }

    ~SetColor()
    { Zypper::instance().configNoConst().do_colors = docolors; }

    bool docolors;
  };
  SetColor setcolor( _force_no_color );

  _wrap_width = get_screen_width();

  if ( !_failedInstalls.empty() )
    writeFailedInstalls( out );
  if ( !_skippedInstalls.empty() )
    writeSkippedInstalls( out );
  if ( !_failedRemovals.empty() )
    writeFailedRemovals( out );
  if ( !_skippedRemovals.empty() )
    writeSkippedRemovals( out );

}

void CommitSummary::dumpAsXmlTo(std::ostream &out)
{

  collectData();
  out << "<commit-summary>" << endl;

  if ( !_failedInstalls.empty() )
  {
    out << "<failed-installs>" << endl;
    writeXmlResolvableList( out, _failedInstalls );
    out << "</failed-installs>" << endl;
  }

  if ( !_skippedInstalls.empty() )
  {
    out << "<skipped-installs>" << endl;
    writeXmlResolvableList( out, _skippedInstalls );
    out << "</skipped-installs>" << endl;
  }

  if ( !_failedRemovals.empty() )
  {
    out << "<failed-removals>" << endl;
    writeXmlResolvableList( out, _failedRemovals );
    out << "</failed-removals>" << endl;
  }

  if ( !_skippedRemovals.empty() )
  {
    out << "<skipped-removals>" << endl;
    writeXmlResolvableList( out, _skippedRemovals );
    out << "</skipped-removals>" << endl;
  }

  out << "</commit-summary>" << endl;
}

void CommitSummary::showBasicErrorMessage( Zypper &zypp )
{
  zypp.out().error(_("Installation has completed with error.") );
  if ( zypp.exitCode() == ZYPPER_EXIT_ERR_COMMIT )
    zypp.out().error( str::Format(_("You may run '%1%' to repair any dependency problems.")) % "zypper verify" );
}

void CommitSummary::writeXmlResolvableList( std::ostream & out, const std::vector< zypp::sat::Solvable> &solvables )
{
  for ( const auto &solvable : solvables )
  {
    out << "<solvable";
    out << " type=\"" << solvable.kind() << "\"";
    out << " name=\"" << solvable.name() << "\"";
    out << " edition=\"" << solvable.edition() << "\"";
    out << " arch=\"" << solvable.arch() << "\"";
    {
      const std::string & text( solvable.summary() );
      if ( !text.empty() )
        out << " summary=\"" << xml::escape(text) << "\"";
    }
    {
      const std::string & text( solvable.description() );
      if ( !text.empty() )
        out << ">\n" << "<description>" << xml::escape( text ) << "</description>" << "</solvable>" << endl;
      else
        out << "/>" << endl;
    }
  }
}


void CommitSummary::writeFailedInstalls( std::ostream & out )
{
  std::string label( "%d" );

  label = PL_(
    "The following package failed to install:",
    "The following %d packages failed to install:",
    _failedInstalls.size() );

  label = str::form( label.c_str(), _failedInstalls.size() );

  out << endl << ( ColorContext::MSG_ERROR << label ) << endl;
  writeResolvableList( out, _failedInstalls, ColorContext::NEGATIVE );
}

void CommitSummary::writeSkippedInstalls( std::ostream & out )
{
  std::string label( "%d" );

  label = PL_(
    "The following package installation was skipped:",
    "The following %d package installations were skipped:",
    _failedInstalls.size() );

  label = str::form( label.c_str(), _failedInstalls.size() );

  out << endl << ( ColorContext::MSG_WARNING << label ) << endl;
  writeResolvableList( out, _skippedInstalls, ColorContext::NEGATIVE );
}

void CommitSummary::writeFailedRemovals( std::ostream & out )
{
  std::string label( "%d" );

  label = PL_(
    "The following package failed to uninstall:",
    "The following %d packages failed to uninstall:",
    _failedInstalls.size() );

  label = str::form( label.c_str(), _failedInstalls.size() );

  out << endl << ( ColorContext::MSG_ERROR << label ) << endl;
  writeResolvableList( out, _failedRemovals, ColorContext::NEGATIVE );
}

void CommitSummary::writeSkippedRemovals( std::ostream & out )
{
  std::string label( "%d" );

  label = PL_(
    "The following package removal was skipped:",
    "The following %d package removal were skipped:",
    _failedInstalls.size() );

  label = str::form( label.c_str(), _failedInstalls.size() );

  out << endl << ( ColorContext::MSG_WARNING << label ) << endl;
  writeResolvableList( out, _skippedRemovals, ColorContext::NEGATIVE );
}

bool CommitSummary::writeResolvableList( std::ostream & out,
  const std::vector< zypp::sat::Solvable > & solvables,
  ansi::Color color,
  unsigned maxEntires_r )
{
  bool ret = true;	// whether the complete list was written, or maxEntires_r clipped

  if ( (_viewop & DETAILS) == 0 )
  {
    static const HIGHLIGHTString quoteCh( "\"" );

    TriBool pkglistHighlight = Zypper::instance().config().color_pkglistHighlight;
    ansi::Color pkglistHighlightAttribute = Zypper::instance().config().color_pkglistHighlightAttribute;
    char firstCh = 0;

    std::ostringstream s;
    unsigned relevant_entries = 0;
    for ( const auto &solvable : solvables ) {
      // name
      const std::string & name( solvable.name() );
      ++relevant_entries;
      if ( maxEntires_r && relevant_entries > maxEntires_r )
        continue;

      // quote names with spaces
      bool quote = name.find_first_of( " " ) != std::string::npos;

      // quote?
      if ( quote ) s << quoteCh;

      // highlight 1st char?
      if ( pkglistHighlight || ( indeterminate(pkglistHighlight) && name[0] != firstCh ) )
      {
        s << ( color << pkglistHighlightAttribute << name[0] ) << name.c_str()+1;
        if ( indeterminate(pkglistHighlight) )
          firstCh = name[0];
      }
      else
      {
        s << name;
      }

      // quote?
      if ( quote ) s << quoteCh;
      s << " ";
    }
    if ( maxEntires_r && relevant_entries > maxEntires_r )
    {
      relevant_entries -= maxEntires_r;
      // translators: Appended when clipping a long enumeration:
      // "ConsoleKit-devel ConsoleKit-doc ... and 20828 more items."
      s << ( color << str::Format(PL_( "... and %1% more item.",
                        "... and %1% more items.",
                        relevant_entries) ) % relevant_entries );
      ret = false;
    }
    mbs_write_wrapped( out, s.str(), 2, _wrap_width );
    out << endl;
    return ret;
  }

  Table t;
  t.lineStyle(TableLineStyle::none);
  t.margin(2);
  t.wrap(0);

  unsigned relevant_entries = 0;
  for ( const auto & solvable : solvables )
  {
    ++relevant_entries;
    if ( maxEntires_r && relevant_entries > maxEntires_r )
      continue;

    const std::string &name = solvable.name();

    TableRow tr;
    tr << name;
    if ( _viewop & SHOW_VERSION ) {
        tr << solvable.edition().asString();
    }
    if ( _viewop & SHOW_ARCH ) {
        tr << solvable.arch().asString();
    }
    if ( _viewop & SHOW_REPO ) {
      tr << solvable.repoInfo().asUserString();
    }
    if ( _viewop & SHOW_VENDOR ) {
        tr << solvable.vendor();
    }
    t << std::move(tr);
  }
  out << t;
  if ( maxEntires_r && relevant_entries > maxEntires_r )
  {
    relevant_entries -= maxEntires_r;
    // translators: Appended when clipping a long enumeration:
    // "ConsoleKit-devel ConsoleKit-doc ... and 20828 more items."
    out << ( color << str::Format(PL_( "... and %1% more item.",
                        "... and %1% more items.",
                        relevant_entries) ) % relevant_entries ) << endl;
    ret = false;
  }

  return ret;
}

void CommitSummary::collectData()
{
  if ( _dataCollected )
    return;

  _dataCollected = true;

  // collect all results we are interested in)
  for ( const auto &step : _result.transactionStepList() ) {

    const auto stage = step.stepStage();
    if ( stage == zypp::sat::Transaction::STEP_DONE )
      continue;

    switch ( step.stepType() ) {
      case zypp::sat::Transaction::TRANSACTION_IGNORE:
        continue;
      case zypp::sat::Transaction::TRANSACTION_ERASE: {

        if ( stage == zypp::sat::Transaction::STEP_ERROR )
          _failedRemovals.push_back( step.satSolvable() );
        else
          _skippedRemovals.push_back( step.satSolvable() );
        break;
      }

      case zypp::sat::Transaction::TRANSACTION_INSTALL:
      case zypp::sat::Transaction::TRANSACTION_MULTIINSTALL: {

        if ( stage == zypp::sat::Transaction::STEP_ERROR )
          _failedInstalls.push_back( step.satSolvable() );
        else
          _skippedInstalls.push_back( step.satSolvable() );
        break;
      }
    }
  }
}

