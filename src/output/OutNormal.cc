#include <iostream>
#include <sstream>

#include <unistd.h>

#include "zypp/Pathname.h"
#include "zypp/ByteCount.h" // for download progress reporting
#include "zypp/base/String.h" // for toUpper()

#include "../zypper-main.h"
#include "AliveCursor.h"

#include "OutNormal.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::ostringstream;

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
  cout << msg << endl;
}

void OutNormal::warning(const std::string & msg, Verbosity verbosity, Type mask)
{
  if (infoWarningFilter(verbosity, mask))
    return;
  info(_("Warning: ") + msg, verbosity, mask);
}

void OutNormal::error(const std::string & problem_desc, const std::string & hint)
{
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
  // problem
  cerr << problem_desc << endl;
  // cause
  cerr << zyppExceptionReport(e) << endl;
  // hint
  if (!hint.empty())
    cerr << hint << endl;
}

// ----------------------------------------------------------------------------

static void display_progress ( const std::string & id, const string & s, int percent)
{
  static AliveCursor cursor;

  if (isatty(1))
  {
    cout << CLEARLN << cursor++ << " " << s;
    // dont display percents if invalid
    if (percent >= 0 && percent <= 100)
      cout << " [" << percent << "%]";
    cout << std::flush;
  }
  else
    cout << '.' << std::flush;
}

// ----------------------------------------------------------------------------

static void display_tick ( const std::string & id, const string & s)
{
  static AliveCursor cursor;

  if (isatty(1))
  {
    cursor++;
    cout << CLEARLN << cursor << " " << s;
    cout << std::flush;
  }
  else
  {
    cout << '.';
  }
}

// ----------------------------------------------------------------------------

void OutNormal::progressStart(const std::string & id,
                              const std::string & label,
                              bool is_tick)
{
  if (progressFilter())
    return;

  if (!isatty(1))
    cout << label << ' ';  
  
  if (is_tick)
    display_tick(id, label);
  else
    display_progress(id, label, 0);
}

void OutNormal::progress(const std::string & id, const string & label, int value)
{
  if (progressFilter())
    return;

  if (value)
    display_progress(id, label, value);
  else
    display_tick(id, label);
}

void OutNormal::progressEnd(const std::string & id, const string & label, bool error)
{
  if (progressFilter())
    return;

  static AliveCursor cursor;
  if (isatty(1))
    cout << CLEARLN << cursor.done() << " " << label << std::flush << endl;
  else
   cout << cursor.done() << std::flush << endl;
}

// progress with download rate
void OutNormal::dwnldProgressStart(const zypp::Url & uri)
{
  if (verbosity() < NORMAL)
    return;

  static AliveCursor cursor;
  if (isatty(1))
    cout << CLEARLN << cursor << " " << _("Downloading:") << " ";
  else
    cout << _("Downloading:") << " ";
  if (verbosity() == DEBUG)
    cout << uri; //! \todo shorten to fit the width of the terminal
  else
    cout << zypp::Pathname(uri.getPathName()).basename();
  if (isatty(1))
    cout << " [" << _("starting") << "]"; //! \todo align to the right
  else
    cout << " [" ;
  cout << std::flush;
}

void OutNormal::dwnldProgress(const zypp::Url & uri,
                              int value,
                              long rate) 
{
  if (verbosity() < NORMAL)
    return;
  if (!isatty(1))
  {
    cout << '.' << std::flush;
    return;
  }

  static AliveCursor cursor;
  cout << CLEARLN << cursor++ << " " << _("Downloading:") << " ";
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

  cout << std::flush;
}

void OutNormal::dwnldProgressEnd(const zypp::Url & uri, long rate, bool error)
{
  if (verbosity() < NORMAL)
    return;

  static AliveCursor cursor;
  if (isatty(1))
  {
    cout << CLEARLN << cursor.done() << " " << _("Downloading:") << " ";
    if (verbosity() == DEBUG)
      cout << uri; //! \todo shorten to fit the width of the terminal
    else
      cout << zypp::Pathname(uri.getPathName()).basename();
    cout << " [";
  }
  cout << (error ? _("error") : _("done"));
  if (rate >= 0)
    cout << " (" << zypp::ByteCount(rate) << "/s)";
  cout << "]";
  cout << endl << std::flush;
}

void OutNormal::prompt(PromptId id,
                       const string & prompt,
                       const PromptOptions & poptions,
                       const std::string & startdesc)
{
  string option_str;
  PromptOptions::OptionList::const_iterator it;
  if ((it = poptions.options().begin()) != poptions.options().end())
  {
    option_str += (poptions.defaultOpt() == 0 ? zypp::str::toUpper(*it) : *it);
    ++it;
  }
  for (unsigned int i = 1; it != poptions.options().end(); ++it, i++)
    option_str += "/" + (poptions.defaultOpt() == i ? zypp::str::toUpper(*it) : *it);

  if (startdesc.empty())
    cout << CLEARLN;
  else
    cout << startdesc << endl;
  cout << prompt << " [" << option_str << "]: " << std::flush;
}
