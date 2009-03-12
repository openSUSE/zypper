/*---------------------------------------------------------------------------*\
                          ____  _ _ __ _ __  ___ _ _
                         |_ / || | '_ \ '_ \/ -_) '_|
                         /__|\_, | .__/ .__/\___|_|
                             |__/|_|  |_|
\*---------------------------------------------------------------------------*/

#include <iostream>
#include <sstream>

#include <unistd.h>

#include "zypp/Pathname.h"
#include "zypp/ByteCount.h" // for download progress reporting
#include "zypp/base/String.h" // for toUpper()

#include "main.h"
#include "utils/colors.h"
#include "AliveCursor.h"

#include "OutNormal.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::ostringstream;

OutNormal::OutNormal(Verbosity verbosity)
  : Out(TYPE_NORMAL, verbosity),
    _use_colors(false), _isatty(isatty(STDOUT_FILENO))
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

  if (_use_colors && verbosity > Out::QUIET)
    cout << COLOR_WHITE << msg << COLOR_RESET << endl;
  else
    cout << msg << endl;
}

void OutNormal::warning(const std::string & msg, Verbosity verbosity, Type mask)
{
  if (infoWarningFilter(verbosity, mask))
    return;

  if (_use_colors)
    cout << COLOR_YELLOW_BOLD << _("Warning: ") << COLOR_RESET << msg << endl;
  else
    cout << msg << endl;
}

void OutNormal::error(const std::string & problem_desc, const std::string & hint)
{
  if (_use_colors)
    cerr << COLOR_RED_BOLD << problem_desc << COLOR_RESET;
  else
    cerr << problem_desc;
  if (!hint.empty() && this->verbosity() > Out::QUIET)
    cerr << endl << hint;
  cerr << endl;
}

// ----------------------------------------------------------------------------

void OutNormal::error(const zypp::Exception & e,
                      const string & problem_desc,
                      const string & hint)
{
  if (_use_colors)
    cerr << COLOR_RED_BOLD;

  // problem
  cerr << problem_desc << endl;
  // cause
  cerr << zyppExceptionReport(e) << endl;

  if (_use_colors)
    cerr << COLOR_RESET;

  // hint
  if (!hint.empty() && this->verbosity() > Out::QUIET)
    cerr << hint << endl;
}

// ----------------------------------------------------------------------------

static void display_progress ( const std::string & id, const string & s, int percent)
{
  static AliveCursor cursor;

  if (isatty(STDOUT_FILENO))
  {
    cout << CLEARLN << s << " [";
    // dont display percents if invalid
    if (percent >= 0 && percent <= 100)
      cout << percent << "%";
    else
      cout << ++cursor;
    cout << "]";
  }
  else
    cout << '.';
  cout << std::flush;
}

// ----------------------------------------------------------------------------

static void display_tick ( const std::string & id, const string & s)
{
  static AliveCursor cursor;

  if (isatty(STDOUT_FILENO))
  {
    cout << CLEARLN << s << " [" << ++cursor << "]";
    cout << std::flush;
  }
  else
    cout << '.' << std::flush;
}

// ----------------------------------------------------------------------------

void OutNormal::progressStart(const std::string & id,
                              const std::string & label,
                              bool is_tick)
{
  if (progressFilter())
    return;

  if (_use_colors)
    cerr << COLOR_WHITE;

  if (!_isatty)
    cout << label << " [";

  if (is_tick)
    display_tick(id, label);
  else
    display_progress(id, label, 0);

  if (_use_colors)
    cerr << COLOR_RESET;
}

void OutNormal::progress(const std::string & id, const string & label, int value)
{
  if (progressFilter())
    return;

  if (_use_colors)
    cerr << COLOR_WHITE;

  if (value)
    display_progress(id, label, value);
  else
    display_tick(id, label);

  if (_use_colors)
    cerr << COLOR_RESET;
}

void OutNormal::progressEnd(const std::string & id, const string & label, bool error)
{
  if (progressFilter())
    return;

  if (_use_colors)
    cerr << COLOR_WHITE;

  if (_isatty)
  {
    cout << CLEARLN << label << " [";
    if (error)
      print_color(_("error"), COLOR_RED, COLOR_WHITE);
    else
      cout << _("done");
  }
  cout << "]";

  if (_use_colors)
    cerr << COLOR_RESET;

  cout << endl << std::flush;
}

// progress with download rate
void OutNormal::dwnldProgressStart(const zypp::Url & uri)
{
  if (verbosity() < NORMAL)
    return;

  if (_use_colors)
    cerr << COLOR_WHITE;

  if (isatty(STDOUT_FILENO))
    cout << CLEARLN;
  cout << _("Retrieving:") << " ";
  if (verbosity() == DEBUG)
    cout << uri; //! \todo shorten to fit the width of the terminal
  else
    cout << zypp::Pathname(uri.getPathName()).basename();
  if (isatty(STDOUT_FILENO))
    cout << " [" << _("starting") << "]"; //! \todo align to the right
  else
    cout << " [" ;

  if (_use_colors)
    cerr << COLOR_RESET;

  cout << std::flush;
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

  if (_use_colors)
    cerr << COLOR_WHITE;

  cout << CLEARLN << _("Retrieving:") << " ";
  if (verbosity() == DEBUG)
    cout << uri; //! \todo shorten to fit the width of the terminal
  else
    cout << zypp::Pathname(uri.getPathName()).basename();
  // dont display percents if invalid
  if ((value >= 0 && value <= 100) || rate >= 0)
  {
    cout << " [";
    if (value >= 0 && value <= 100)
      cout << value << "%";
    if (rate >= 0)
      cout << " (" << zypp::ByteCount(rate) << "/s)";
    cout << "]";
  }

  if (_use_colors)
    cerr << COLOR_RESET;

  cout << std::flush;
}

void OutNormal::dwnldProgressEnd(const zypp::Url & uri, long rate, bool error)
{
  if (verbosity() < NORMAL)
    return;

  if (_use_colors)
    cerr << COLOR_WHITE;

  if (_isatty)
  {
    cout << CLEARLN << _("Retrieving:") << " ";
    if (verbosity() == DEBUG)
      cout << uri; //! \todo shorten to fit the width of the terminal
    else
      cout << zypp::Pathname(uri.getPathName()).basename();
    cout << " [";
    if (error)
      print_color(_("error"), COLOR_RED, COLOR_WHITE);
    else
      cout << _("done");
  }
  else
    cout << (error ? _("error") : _("done"));

  if (rate >= 0)
    cout << " (" << zypp::ByteCount(rate) << "/s)";
  cout << "]";

  if (_use_colors)
    cerr << COLOR_RESET;

  cout << endl << std::flush;
}

void OutNormal::prompt(PromptId id,
                       const string & prompt,
                       const PromptOptions & poptions,
                       const std::string & startdesc)
{
  if (startdesc.empty())
  {
    if (_isatty)
      cout << CLEARLN;
  }
  else
    cout << startdesc << endl;
  cout << prompt;
  if (!poptions.empty())
    cout << " [" << (_use_colors ? poptions.optionStringColored() : poptions.optionString()) << "]";
  cout << ": " << std::flush;
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

  cout << endl << "[" << (_use_colors ? poptions.optionStringColored() : poptions.optionString()) << "]: " << std::flush;
}
