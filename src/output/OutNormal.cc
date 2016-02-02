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
#include <zypp/base/String.h> // for toUpper()

#include "main.h"
#include "utils/colors.h"
#include "AliveCursor.h"
#include "Utf8.h"

#include "Zypper.h"
#include "OutNormal.h"

using namespace std;

OutNormal::OutNormal(Verbosity verbosity)
  : Out(TYPE_NORMAL, verbosity),
    _use_colors(false), _isatty(isatty(STDOUT_FILENO)), _newline(true), _oneup(false)
{}

OutNormal::~OutNormal()
{

}

bool OutNormal::mine(Type type)
{
  // Type::TYPE_NORMAL is mine
  if (type & Out::TYPE_NORMAL)
    return true;
  return false;
}

bool OutNormal::infoWarningFilter(Verbosity verbosity, Type mask)
{
  if (!mine(mask))
    return true;
  if (this->verbosity() < verbosity)
    return true;
  return false;
}

void OutNormal::info(const std::string & msg, Verbosity verbosity, Type mask)
{
  if (infoWarningFilter(verbosity, mask))
    return;

  if (!_newline)
    cout << endl;

  if (verbosity == Out::QUIET)
    print_color(msg, COLOR_CONTEXT_RESULT);
  else
    print_color(msg, COLOR_CONTEXT_MSG_STATUS);

  cout << endl;
  _newline = true;
}

void OutNormal::warning(const std::string & msg, Verbosity verbosity, Type mask)
{
  if (infoWarningFilter(verbosity, mask))
    return;

  if (!_newline)
    cout << endl;

  print_color(_("Warning: "), COLOR_CONTEXT_MSG_WARNING);
  cout << msg << endl;
  _newline = true;
}

void OutNormal::error(const std::string & problem_desc, const std::string & hint)
{
  if (!_newline)
    cout << endl;

  fprint_color(cerr, problem_desc, COLOR_CONTEXT_MSG_ERROR);
  if (!hint.empty() && this->verbosity() > Out::QUIET)
    cerr << endl << hint;
  cerr << endl;
  _newline = true;
}

// ----------------------------------------------------------------------------

void OutNormal::error(const zypp::Exception & e,
                      const string & problem_desc,
                      const string & hint)
{
  if (!_newline)
    cout << endl;

  // problem
  fprint_color(cerr, problem_desc, COLOR_CONTEXT_MSG_ERROR);
  cerr << endl;
  // cause
  fprint_color(cerr, zyppExceptionReport(e), COLOR_CONTEXT_MSG_ERROR);
  cerr << endl;

  // hint
  if (!hint.empty() && this->verbosity() > Out::QUIET)
    cerr << hint << endl;

  _newline = true;
}

// ----------------------------------------------------------------------------

void OutNormal::displayProgress (const string & s, int percent)
{
  static AliveCursor cursor;

  if ( _isatty )
  {
    zypp::str::Str outstr;
    outstr << s << " [";
    // dont display percents if invalid
    if ( percent >= 0 && percent <= 100 )
    {
      outstr << percent << "%";
    }
    else
    {
      ++cursor;
      outstr << cursor.current();
    }
    outstr << "]";

    if(_oneup)
      cout << CLEARLN << CURSORUP(1);
    cout << CLEARLN;

    utf8::string outline( outstr );
    cout << outline << std::flush;
    _oneup = (outline.length() > termwidth());
  }
  else
    cout << '.' << std::flush;
}

// ----------------------------------------------------------------------------

void OutNormal::displayTick (const string & s)
{
  static AliveCursor cursor;

  if (_isatty)
  {
    zypp::str::Str outstr;
    ++cursor;
    outstr << s << " [" << cursor.current() << "]";

    if(_oneup)
      cout << CLEARLN << CURSORUP(1);
    cout << CLEARLN;

    utf8::string outline( outstr );
    cout << outline << std::flush;
    _oneup = (outline.length() > termwidth());
  }
  else
    cout << '.';
  cout << std::flush;
}

// ----------------------------------------------------------------------------

void OutNormal::progressStart(const std::string & id,
                              const std::string & label,
                              bool is_tick)
{
  if (progressFilter())
    return;

  if (!_isatty)
    cout << label << " [";

  if (is_tick)
    displayTick(label);
  else
    displayProgress(label, 0);

  _newline = false;
}

void OutNormal::progress(const std::string & id, const string & label, int value)
{
  if (progressFilter())
    return;

  if (value)
    displayProgress(label, value);
  else
    displayTick(label);

  _newline = false;
}

void OutNormal::progressEnd(const std::string & id, const string & label, bool error)
{
  if (progressFilter())
    return;

  if (!error && _use_colors)
    cout << get_color(COLOR_CONTEXT_MSG_STATUS);

  if (_isatty)
  {
    if(_oneup)
    {
      cout << CLEARLN << CURSORUP(1) << CLEARLN;
      _oneup = false;
    }
    else
      cout << CLEARLN;
    cout << label << " [";
  }

  if (error)
    print_color(_("error"), COLOR_CONTEXT_NEGATIVE);
  else
    cout << _("done");

  cout << "]";

  if (!error && _use_colors)
    cout << COLOR_RESET;

  cout << endl << std::flush;
  _newline = true;
}

// progress with download rate
void OutNormal::dwnldProgressStart(const zypp::Url & uri)
{
  if (verbosity() < NORMAL)
    return;

  if (_isatty)
    cout << CLEARLN;

  zypp::str::Str outstr;
  outstr << _("Retrieving:") << " ";
  if (verbosity() == DEBUG)
    outstr << uri; //! \todo shorten to fit the width of the terminal
  else
    outstr << zypp::Pathname(uri.getPathName()).basename();
  if (_isatty)
    outstr << " [" << _("starting") << "]"; //! \todo align to the right
  else
    outstr << " [" ;

  utf8::string outline( outstr );
  cout << outline << std::flush;
  _oneup = (outline.length() > termwidth());

  _newline = false;
}

void OutNormal::dwnldProgress(const zypp::Url & uri,
                              int value,
                              long rate)
{
  if (verbosity() < NORMAL)
    return;

  if (!isatty(STDOUT_FILENO))
  {
    cout << '.' << std::flush;
    return;
  }

  if(_oneup)
    cout << CLEARLN << CURSORUP(1);
  cout << CLEARLN;

  zypp::str::Str outstr;
  outstr << _("Retrieving:") << " ";
  if (verbosity() == DEBUG)
    outstr << uri; //! \todo shorten to fit the width of the terminal
  else
    outstr << zypp::Pathname(uri.getPathName()).basename();
  // dont display percents if invalid
  if ((value >= 0 && value <= 100) || rate >= 0)
  {
    outstr << " [";
    if (value >= 0 && value <= 100)
      outstr << value << "%";
    if (rate >= 0)
      outstr << " (" << zypp::ByteCount(rate) << "/s)";
    outstr << "]";
  }

  utf8::string outline( outstr );
  cout << outline << std::flush;
  _oneup = (outline.length() > termwidth());
  _newline = false;
}

void OutNormal::dwnldProgressEnd(const zypp::Url & uri, long rate, bool error)
{
  if (verbosity() < NORMAL)
    return;

  if (!error && _use_colors)
    cout << get_color(COLOR_CONTEXT_MSG_STATUS);

  zypp::str::Str outstr;
  if (_isatty)
  {
    if(_oneup)
      outstr << CLEARLN << CURSORUP(1);
    outstr << CLEARLN;
    outstr << _("Retrieving:") << " ";
    if (verbosity() == DEBUG)
      outstr << uri; //! \todo shorten to fit the width of the terminal
    else
      outstr << zypp::Pathname(uri.getPathName()).basename();
    outstr << " [";
    if (error)
      fprint_color(outstr._str, _("error"), COLOR_CONTEXT_NEGATIVE);
    else
      outstr << _("done");
  }
  else
    outstr << (error ? _("error") : _("done"));

  if (rate >= 0)
    outstr << " (" << zypp::ByteCount(rate) << "/s)";
  outstr << "]";

  std::string outline( outstr );
  cout << outline << endl << std::flush;
  _newline = true;

  if (!error && _use_colors)
    outstr << COLOR_RESET;
}

void OutNormal::prompt(PromptId id,
                       const string & prompt,
                       const PromptOptions & poptions,
                       const std::string & startdesc)
{
  if (!_newline)
    cout << endl;

  if (startdesc.empty())
  {
    if (_isatty)
      cout << CLEARLN;
  }
  else
    cout << startdesc << endl;

  std::ostringstream pstr;
  std::ostream & cout( pstr );

  cout << prompt;
  if (!poptions.empty())
    cout << " " << poptions.optionString();
  cout << ": ";

  if ( Zypper::instance()->config().do_colors )
  {
    // bsc#948566: Terminal is dumb and simply counts the amount of printable
    // characters. If the number of printable characters within ansi SGR sequences
    // is not a multiple of 8, tab stops are not computed correctly. We use
    // superfluous resets ("\033[0m"; 3 printable chars) to fill up.
    // Better ideas are welcome.
    size_t invis = 0;
    bool   insgr = false;
    const std::string & pstrstr( pstr.str() );
    for_( chit, pstrstr.begin(), pstrstr.end() )
    {

      if ( insgr )
      {
        ++invis;
        if ( *chit == 'm' )
          insgr = false;
      }
      else if ( *chit == '\033' )
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
        cout << "\033[0m";
    }
  }

  std::cout << pstr.str() << std::flush;

  // prompt ends with newline (user hits <enter>) unless exited abnormaly
  _newline = true;
}

void OutNormal::promptHelp(const PromptOptions & poptions)
{
  cout << endl;
  if (poptions.helpEmpty())
    cout << _("No help available for this prompt.") << endl;
  else
  {
    unsigned int pos = 0;
    for(PromptOptions::StrVector::const_iterator it = poptions.options().begin();
        it != poptions.options().end(); ++it, ++pos)
    {
      if (poptions.isDisabled(pos))
        continue;
      cout << *it << " - ";
      const string & hs_r = poptions.optionHelp(pos);
      if (hs_r.empty())
        cout << "(" << _("no help available for this option") << ")";
      else
        cout << hs_r;
      cout << endl;
    }
  }

  cout << endl << poptions.optionString() << ": " << std::flush;
  // prompt ends with newline (user hits <enter>) unless exited abnormaly
  _newline = true;
}

unsigned int OutNormal::termwidth() const {
  if(!_isatty)
    return 10000;
  else
  {
    struct winsize wns;
    if (!ioctl(1, TIOCGWINSZ, &wns))
      return wns.ws_col;
    else
      return 10000;
  }
}
