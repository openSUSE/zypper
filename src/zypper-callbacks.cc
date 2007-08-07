#include <ctype.h>
#include <sstream>

#include <boost/format.hpp>

#include "AliveCursor.h"
#include "zypper.h"

using namespace std;
using namespace boost;

void display_progress ( const std::string &id, ostream & out, const string& s, int percent) {
  static AliveCursor cursor;

  if (gSettings.machine_readable)
  {
    cout << "<progress id=\"" << id << "\" type=\"percentage\" value=\"" << percent << "\" name=\"" << s << "\"/>" << endl;
    return;
  }

  if ( percent == 100 )
    out << CLEARLN << cursor.done() << " " << s;
  else
    out << CLEARLN << cursor++ << " " << s;
  // dont display percents if invalid
  if (percent >= 0 && percent <= 100)
    out << " [" << percent << "%]";
  out << flush;
}

// ----------------------------------------------------------------------------

void display_tick ( const std::string &id, ostream & out, const string& s) {
  static AliveCursor cursor;

  if (gSettings.machine_readable)
  {
    cout << "<progress id=\"" << id << "\" type=\"tick\" value=\"" << -1 << "\" name=\"" << s << "\"/>" << endl;
    return;
  }

  cursor++;
  out << CLEARLN << cursor << " " << s;
  out << flush;
}

// ----------------------------------------------------------------------------

void display_done ( const std::string &id, ostream & out, const string& s) {
  static AliveCursor cursor;

  if (gSettings.machine_readable)
  {
    cout << "<progress id=\"" << id << "\" type=\"done\" name=\"" << s << "\"/>" << endl;
    return;
  }

  out << CLEARLN << cursor.done() << " " << s;
  out << flush;
  out << endl;
}

// ----------------------------------------------------------------------------

void display_done (const std::string &id, ostream & out) {

  if (gSettings.machine_readable)
  {
    display_done( id, cout, "");
    return;
  }


  out << endl;
}

// ----------------------------------------------------------------------------

//template<typename Action>
//Action ...
int read_action_ari (int default_action) {
  // TranslatorExplanation don't translate letters in parentheses!! (yet)
  cout << _("(A)bort, (R)etry, (I)gnore?") << " "; 

  // choose abort if no default has been specified
  if (default_action == -1) {
    default_action = 0;
  }

  // non-interactive mode
  if (gSettings.non_interactive) {
      char c;
      switch (default_action) {
	  case 0: c = 'a'; break;
	  case 1: c = 'r'; break;
	  case 2: c = 'i'; break;
	  default: c = '?';
      }
      // print the answer for conveniecne
      cout << c << endl;
      MIL << "answer: " << c << endl;
      return default_action;
  }

  // interactive mode, ask user
  while (cin.good()) {		// #269263
    char c;
    cin >> c;
    c = tolower (c);
    MIL << "answer: " << c << endl;
    if (c == 'a')
      return 0;
    else if (c == 'r')
      return 1;
    else if (c == 'i')
      return 2;
    cerr << _("Invalid answer. Choose letter a, r, or i.") << endl;
    DBG << "invalid answer" << endl;
  }

  return default_action;
}

// ----------------------------------------------------------------------------

bool read_bool_answer(const string & question, bool default_answer)
{
  if (!gSettings.machine_readable)
  	cout << CLEARLN << question << " [y/n]: " << flush;

  // non-interactive mode: print the answer for convenience and return default
  if (gSettings.non_interactive)
  {
  	if (!gSettings.machine_readable)
	    cout << (default_answer ? 'y' : 'n') << endl;
    MIL << "answer (default): " << (default_answer ? 'y' : 'n') << endl;
    return default_answer;
  }

  istream & stm = cin;

  string c = "";
  bool been_here_before = false;
  while (stm.good ()
    // TranslatorExplanation This is a Lowercase 'y' for "yes" used as an
    // answer in y/n prompts. Translate to the same letter as you translated
    // it in the "[y/n]" string!.
    && c != _("y")
    // TranslatorExplanation Uppercase 'Y' for "yes" used as an
    // answer in y/n prompts. Translate to the same letter as you translated
    // it in the "[y/n]" string!.
    && c != _("Y")
    // TranslatorExplanation Lowercase 'n' for "no" used as an
    // answer in y/n prompts. Translate to the same letter as you translated
    // it in the "[y/n]" string!.
    && c != _("n")
    // TranslatorExplanation Uppercase 'N' for "no" used as an
    // answer in y/n prompts. Translate to the same letter as you translated
    // it in the "[y/n]" string!.
    && c != _("N"))
  {
    if (been_here_before)
      // TranslatorExplanation Example: Invalid answer 'x', enter 'y' or 'n':
      cerr << format(_("Invalid answer '%s', enter '%s' or '%s':"))
          % c % _("y") % _("n") << " ";
    c = zypp::str::getline (stm, zypp::str::TRIM);
    been_here_before = true;
  }

  MIL << "answer: " << c << endl;
  if (c == _("y") || c == _("Y"))
    return true;
  else if (c == _("n") || c == _("N"))
    return false;
  else
  {
    WAR << "could not read answer, returning default: "
        << (default_answer ? 'y' : 'n') << endl;
    return default_answer;
  }
}

// ----------------------------------------------------------------------------

string to_string (zypp::Resolvable::constPtr resolvable) {
  ostringstream ss;
  ss << *resolvable;
  return ss.str ();
}

// Local Variables:
// c-basic-offset: 2
// End:
