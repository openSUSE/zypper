#include <iostream>
#include <sstream>

#include "OutNormal.h"
#include "../AliveCursor.h"

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
  info(msg, verbosity, mask);
}

void OutNormal::error(const std::string & problem_desc, const std::string & hint)
{
  cerr << problem_desc;
  if (!hint.empty() && this->verbosity() > Out::QUIET)
    cerr << endl << hint;
  cerr << endl;
}

// ----------------------------------------------------------------------------

string OutNormal::reportZyppException(const zypp::Exception & e)
{
  ostringstream s;
  if (e.historySize())
  {
    if (this->verbosity() > Out::NORMAL)
    {
      // print the whole history
      s << e.historyAsString();
      // this exception
      s << " - " << e.asUserString();
    }
    else
      // print the root cause only
      s << *(--e.historyEnd());
  }
  else
    s << e.asUserString();

  return s.str();
}

// ----------------------------------------------------------------------------

void OutNormal::error(const zypp::Exception & e,
                      const string & problem_desc,
                      const string & hint)
{
  // problem
  cerr << problem_desc << endl;

  // cause
  cerr << reportZyppException(e) << endl;

  // hint
  if (!hint.empty())
    cerr << hint << endl;
}

// ----------------------------------------------------------------------------

static void display_progress ( const std::string & id, const string & s, int percent)
{
  static AliveCursor cursor;

  if ( percent == 100 )
    cout << CLEARLN << cursor.done() << " " << s;
  else
    cout << CLEARLN << cursor++ << " " << s;
  // dont display percents if invalid
  if (percent >= 0 && percent <= 100)
    cout << " [" << percent << "%]";
  cout << std::flush;
}

// ----------------------------------------------------------------------------

static void display_tick ( const std::string & id, const string & s)
{
  static AliveCursor cursor;

  cursor++;
  cout << CLEARLN << cursor << " " << s;
  cout << std::flush;
}

// ----------------------------------------------------------------------------

void OutNormal::progressStart(const std::string & id,
                              const std::string & label,
                              bool is_tick)
{
  if (progressFilter())
    return;
  
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

void OutNormal::progressEnd(const std::string & id, const string& label)
{
  static AliveCursor cursor;

  if (progressFilter())
    return;

  cout << CLEARLN << cursor.done() << " " << label << std::flush << endl;
}

void OutNormal::dwnldProgressStart(){}
void OutNormal::dwnldProgress(){}
void OutNormal::dwnldProgressEnd(){}
