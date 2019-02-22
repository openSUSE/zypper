/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>
#include <fstream>
#include <sstream>

#include <unistd.h>

#include <zypp/Pathname.h>
#include <zypp/ByteCount.h> // for download progress reporting
#include <zypp/base/Logger.h>
#include <zypp/base/String.h> // for toUpper()

#include "main.h"
#include "utils/colors.h"
#include "AliveCursor.h"

#include "OutNormal.h"

using std::cout;
using std::cerr;
using std::endl;

OutNormal::OutNormal( Verbosity verbosity_r )
: Out( TYPE_NORMAL, verbosity_r )
, _use_colors( false )
, _isatty( do_ttyout() )
, _newline( true )
, _oneup( false )
{}

OutNormal::~OutNormal()
{}

bool OutNormal::mine( Type type )
{ return( type & Out::TYPE_NORMAL ); }

bool OutNormal::infoWarningFilter( Verbosity verbosity_r, Type mask )
{
  if ( !mine( mask ) )
    return true;
  if ( verbosity() < verbosity_r )
    return true;
  return false;
}

void OutNormal::info( const std::string & msg_r, Verbosity verbosity_r, Type mask )
{
  if ( infoWarningFilter( verbosity_r, mask ) )
    return;

  if ( !_newline )
    cout << endl;

  ColorString msg( msg_r, ColorContext::MSG_STATUS );
  if ( verbosity_r == Out::QUIET )
    msg = ColorContext::RESULT;
  else if ( verbosity_r == Out::DEBUG )
    msg = ColorContext::OSDEBUG;

  cout << msg << endl;
  _newline = true;
}

void OutNormal::infoLine( const TermLine & msg, Verbosity verbosity_r, Type mask )
{ info( msg.get( termwidth() ), verbosity_r, mask ); }

void OutNormal::warning( const std::string & msg, Verbosity verbosity_r, Type mask )
{
  if ( infoWarningFilter( verbosity_r, mask ) )
    return;

  if ( !_newline )
    cout << endl;

  cout << ( ColorContext::MSG_WARNING << _("Warning: ") ) << msg << endl;
  _newline = true;
}

void OutNormal::error( const std::string & problem_desc, const std::string & hint )
{
  if ( !_newline )
    cout << endl;

  cerr << ( ColorContext::MSG_ERROR << problem_desc );
  if ( !hint.empty() && verbosity() > Out::QUIET )
    cerr << endl << hint;
  cerr << endl;
  _newline = true;
}

// ----------------------------------------------------------------------------

void OutNormal::error( const Exception & e, const std::string & problem_desc, const std::string & hint )
{
  if ( !_newline )
    cout << endl;

  // problem and cause
  cerr << ( ColorContext::MSG_ERROR << problem_desc << endl << zyppExceptionReport(e) ) << endl;

  // hint
  if ( !hint.empty() && verbosity() > Out::QUIET )
    cerr << hint << endl;

  _newline = true;
}

// ----------------------------------------------------------------------------

void OutNormal::displayProgress ( const std::string & s, int percent )
{
  static AliveCursor cursor;

  if ( _isatty )
  {
    TermLine outstr( TermLine::SF_CRUSH | TermLine::SF_EXPAND, '-' );
    outstr.lhs << s << ' ';

    // dont display percents if invalid
    if ( percent >= 0 && percent <= 100 )
    {
      outstr.percentHint = percent;
    }
    ++cursor;
    outstr.rhs << '[' << cursor.current() << ']';

    if ( _oneup )
      cout << ansi::tty::clearLN << ansi::tty::cursorUP;
    cout << ansi::tty::clearLN;

    std::string outline( outstr.get( termwidth() ) );
    cout << outline << std::flush;
    // no _oneup if CRUSHed // _oneup = ( outline.length() > termwidth() );
  }
  else
    cout << '.' << std::flush;
}

// ----------------------------------------------------------------------------

void OutNormal::displayTick( const std::string & s )
{
  static AliveCursor cursor;

  if ( _isatty )
  {
    TermLine outstr( TermLine::SF_CRUSH | TermLine::SF_EXPAND, '-' );
    ++cursor;
    outstr.lhs << s << ' ';
    outstr.rhs << '[' << cursor.current() << ']';

    if( _oneup )
      cout << ansi::tty::clearLN << ansi::tty::cursorUP;
    cout << ansi::tty::clearLN;

    std::string outline( outstr.get( termwidth() ) );
    cout << outline << std::flush;
    // no _oneup if CRUSHed // _oneup = ( outline.length() > termwidth() );
  }
  else
    cout << '.' << std::flush;
}

// ----------------------------------------------------------------------------

void OutNormal::progressStart( const std::string & id, const std::string & label, bool is_tick )
{
  if ( progressFilter() )
    return;

  if ( !_isatty )
    cout << label << " [";

  if ( is_tick )
    displayTick( label );
  else
    displayProgress( label, 0 );

  _newline = false;
}

void OutNormal::progress( const std::string & id, const std::string & label, int value )
{
  if (progressFilter())
    return;

  if (value)
    displayProgress(label, value);
  else
    displayTick(label);

  _newline = false;
}

void OutNormal::progressEnd( const std::string & id, const std::string & label, bool error )
{
  if ( progressFilter() )
    return;

  if ( !error && _use_colors )
    cout << ColorContext::MSG_STATUS;

  TermLine outstr( TermLine::SF_CRUSH | TermLine::SF_EXPAND, '.' );
  if ( _isatty )
  {
    if ( _oneup )
    {
      cout << ansi::tty::clearLN << ansi::tty::cursorUP;
      _oneup = false;
    }
    cout << ansi::tty::clearLN;

    outstr.lhs << label << ' ';
    outstr.rhs << '[';
    if ( error )
      outstr.rhs << NEGATIVEString(_("error") );
    else
      outstr.rhs << _("done");
  }
  else
    outstr.rhs << (error ? _("error") : _("done"));

  outstr.rhs << ']';

  std::string outline( outstr.get( termwidth() ) );
  cout << outline << endl << std::flush;
  _newline = true;

  if ( !error && _use_colors )
    cout << ColorContext::DEFAULT;
}

// progress with download rate
void OutNormal::dwnldProgressStart( const Url & uri )
{
  if ( verbosity() < NORMAL )
    return;

  if ( _isatty )
    cout << ansi::tty::clearLN;

  TermLine outstr( TermLine::SF_CRUSH | TermLine::SF_EXPAND, '-' );
  outstr.lhs << _("Retrieving:") << ' ';
  if ( verbosity() == DEBUG )
    outstr.lhs << uri;
  else
    outstr.lhs << Pathname(uri.getPathName()).basename();
  outstr.lhs << ' ';
  if (_isatty)
    outstr.rhs << '[' << _("starting") << ']';
  else
    outstr.rhs << '[' ;

  std::string outline( outstr.get( termwidth() ) );
  cout << outline << std::flush;
  // no _oneup if CRUSHed // _oneup = (outline.length() > termwidth());

  _newline = false;
}

void OutNormal::dwnldProgress( const Url & uri, int value, long rate )
{
  if ( verbosity() < NORMAL )
    return;

  if ( !_isatty )
  {
    cout << '.' << std::flush;
    return;
  }

  if( _oneup )
    cout << ansi::tty::clearLN << ansi::tty::cursorUP;
  cout << ansi::tty::clearLN;

  TermLine outstr( TermLine::SF_CRUSH | TermLine::SF_EXPAND, '-' );
  outstr.lhs << _("Retrieving:") << " ";
  if ( verbosity() == DEBUG )
    outstr.lhs << uri;
  else
    outstr.lhs << Pathname(uri.getPathName()).basename();
   outstr.lhs << ' ';

  // dont display percents if invalid
  if ( value >= 0 && value <= 100 )
    outstr.percentHint = value;

  static AliveCursor cursor;
  ++cursor;
  outstr.rhs << '[' << cursor.current();
  if ( rate > 0 )
    outstr.rhs << " (" << ByteCount(rate) << "/s)";
  outstr.rhs << ']';

  std::string outline( outstr.get( termwidth() ) );
  cout << outline << std::flush;
  // no _oneup if CRUSHed // _oneup = (outline.length() > termwidth());
  _newline = false;
}

void OutNormal::dwnldProgressEnd( const Url & uri, long rate, TriBool error )
{
  if ( verbosity() < NORMAL )
    return;

  if ( bool(!error) && _use_colors )
    cout << ColorContext::MSG_STATUS;

  TermLine outstr( TermLine::SF_CRUSH | TermLine::SF_EXPAND, '.' );
  if ( _isatty )
  {
    if( _oneup )
      cout << ansi::tty::clearLN << ansi::tty::cursorUP;
    cout << ansi::tty::clearLN;
    outstr.lhs << _("Retrieving:") << " ";
    if ( verbosity() == DEBUG )
      outstr.lhs << uri;
    else
      outstr.lhs << Pathname(uri.getPathName()).basename();
    outstr.lhs << ' ';
    outstr.rhs << '[';
    if ( indeterminate( error ) )
      // Translator: download progress bar result: "........[not found]"
      outstr.rhs << CHANGEString(_("not found") );
    else if ( error )
      // Translator: download progress bar result: "............[error]"
      outstr.rhs << NEGATIVEString(_("error") );
    else
      // Translator: download progress bar result: ".............[done]"
      outstr.rhs << _("done");
  }
  else
    outstr.rhs << ( indeterminate( error ) ? _("not found") : ( error ? _("error") : _("done") ) );

  if ( rate > 0 )
    outstr.rhs << " (" << ByteCount(rate) << "/s)";
  outstr.rhs << ']';

  std::string outline( outstr.get( termwidth() ) );
  cout << outline << endl << std::flush;
  _newline = true;

  if ( bool(!error) && _use_colors )
    cout << ColorContext::DEFAULT;
}

void OutNormal::prompt( PromptId id, const std::string & prompt, const PromptOptions & poptions, const std::string & startdesc )
{
  if ( !_newline )
    cout << endl;

  if ( startdesc.empty() )
  {
    if ( _isatty )
      cout << ansi::tty::clearLN;
  }
  else
    cout << startdesc << endl;

  std::ostringstream pstr;
  ColorStream cout( pstr, ColorContext::PROMPT ); // scoped color on std::cout

  cout << prompt;
  if ( ! poptions.empty() )
    cout << text::optBlankAfter(prompt) << ColorString( poptions.optionString() );
  cout << ": ";

  if ( do_colors() )
  {
    // bsc#948566: Terminal is dumb and simply counts the amount of printable
    // characters. If the number of printable characters within ansi SGR sequences
    // is not a multiple of 8, tab stops are not computed correctly. We use
    // superfluous resets ("\033[0m"; 3 printable chars) to fill up.
    // Better ideas are welcome.
    size_t invis = 0;
    bool   insgr = false;
    for ( char ch : pstr.str() )
    {
      if ( insgr )
      {
	++invis;
	if ( ch == 'm' )
	  insgr = false;
      }
      else if (  ch == '\033' )
	insgr = true;
    }
    invis %= 8;

    if ( invis )
    {
      // "\033[0m" has 3 printable chars:
      // ( resets[to fill] * 3 ) % 8 == to fill
      //                               0 1 2 3 4 5 6 7
      static const size_t resets[] = { 0,3,6,1,4,7,2,5 };
      for ( size_t i = resets[8-invis]; i; --i )
	cout << ansi::Color::SGRReset();
    }
  }

  std::cout << pstr.str() << std::flush;
  // prompt ends with newline (user hits <enter>) unless exited abnormaly
  _newline = true;
}

void OutNormal::promptHelp( const PromptOptions & poptions )
{
  cout << endl;

  if ( poptions.helpEmpty() )
    cout << _("No help available for this prompt.") << endl;

  // Nevertheless list all option names and their '#NUM' shortcut
  unsigned pos = 0;	// Userland counter #NUM  (starts with #1)

  str::Format fopt { "#%-2d: %-10s" };
  for ( unsigned idx = 0; idx < poptions.options().size(); ++idx )
  {
    if ( poptions.isDisabled(idx) )
      continue;

    cout << ( fopt % ++pos % poptions.options()[idx] );
    if ( ! poptions.helpEmpty() )
    {
      const std::string & help { poptions.optionHelp(idx) };
      cout << " - ";
      if ( help.empty() )
	cout << ( ColorContext::LOWLIGHT << "(" << _("no help available for this option") << ")" );
      else
	cout << help;
    }
    cout << endl;
  }

  ColorStream cout( std::cout, ColorContext::PROMPT ); // scoped color on std::cout
  cout << endl << ColorString( poptions.optionString() ) << ": " << std::flush;
  // prompt ends with newline (user hits <enter>) unless exited abnormaly
  _newline = true;
}

unsigned OutNormal::termwidth() const
{
  if ( _isatty )
  {
    struct winsize wns;
    if ( !ioctl( 1, TIOCGWINSZ, &wns ) )
      return wns.ws_col;
  }
  return Out::termwidth();	// unlimited
}
