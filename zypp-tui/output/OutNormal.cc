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
, _isatty( isatty(STDOUT_FILENO) )
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
      cout << CLEARLN << CURSORUP(1);
    cout << CLEARLN;

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
      cout << CLEARLN << CURSORUP(1);
    cout << CLEARLN;

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
      cout << CLEARLN << CURSORUP(1);
      _oneup = false;
    }
    cout << CLEARLN;

    outstr.lhs << label << ' ';
    outstr.rhs << '[';
    if ( error )
      outstr.rhs << ColorString( _("error"), ColorContext::NEGATIVE );
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
    cout << CLEARLN;

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

  if ( !isatty(STDOUT_FILENO) )
  {
    cout << '.' << std::flush;
    return;
  }

  if( _oneup )
    cout << CLEARLN << CURSORUP(1);
  cout << CLEARLN;

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

void OutNormal::dwnldProgressEnd( const Url & uri, long rate, bool error )
{
  if ( verbosity() < NORMAL )
    return;

  if ( !error && _use_colors )
    cout << ColorContext::MSG_STATUS;

  TermLine outstr( TermLine::SF_CRUSH | TermLine::SF_EXPAND, '.' );
  if ( _isatty )
  {
    if( _oneup )
      cout << CLEARLN << CURSORUP(1);
    cout << CLEARLN;
    outstr.lhs << _("Retrieving:") << " ";
    if ( verbosity() == DEBUG )
      outstr.lhs << uri;
    else
      outstr.lhs << Pathname(uri.getPathName()).basename();
    outstr.lhs << ' ';
    outstr.rhs << '[';
    if ( error )
      outstr.rhs << ColorString( _("error"), ColorContext::NEGATIVE );
    else
      outstr.rhs << _("done");
  }
  else
    outstr.rhs << (error ? _("error") : _("done"));

  if ( rate > 0 )
    outstr.rhs << " (" << ByteCount(rate) << "/s)";
  outstr.rhs << ']';

  std::string outline( outstr.get( termwidth() ) );
  cout << outline << endl << std::flush;
  _newline = true;

  if ( !error && _use_colors )
    cout << ColorContext::DEFAULT;
}

void OutNormal::prompt( PromptId id, const std::string & prompt, const PromptOptions & poptions, const std::string & startdesc )
{
  if ( !_newline )
    cout << endl;

  if ( startdesc.empty() )
  {
    if ( _isatty )
      cout << CLEARLN;
  }
  else
    cout << startdesc << endl;

  ColorStream cout( std::cout, ColorContext::PROMPT ); // scoped color on std::cout
  cout << prompt;
  if ( ! poptions.empty() )
    cout << " " << ColorString( poptions.optionString() );
  cout << ": " << std::flush;
  // prompt ends with newline (user hits <enter>) unless exited abnormaly
  _newline = true;
}

void OutNormal::promptHelp( const PromptOptions & poptions )
{
  cout << endl;
  if ( poptions.helpEmpty() )
    cout << _("No help available for this prompt.") << endl;
  else
  {
    unsigned pos = 0;
    for( PromptOptions::StrVector::const_iterator it = poptions.options().begin(); it != poptions.options().end(); ++it, ++pos )
    {
      if ( poptions.isDisabled( pos ) )
        continue;
      cout << *it << " - ";
      const std::string & hs_r = poptions.optionHelp( pos );
      if ( hs_r.empty() )
        cout << "(" << _("no help available for this option") << ")";
      else
        cout << hs_r;
      cout << endl;
    }
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
