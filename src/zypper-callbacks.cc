#include <ctype.h>
#include <sstream>

#include <boost/format.hpp>

#include "AliveCursor.h"
#include "zmart.h"

using namespace std;
using namespace boost;

void display_progress (const string& s, int percent) {
  static AliveCursor cursor;

  cerr_v << CLEARLN << cursor++ << " " << s << "[" << percent << "%]" << flush;
}

// ----------------------------------------------------------------------------

void display_done () {
  cerr_v << endl;
}

// ----------------------------------------------------------------------------

//template<typename Action>
//Action ...
int read_action_ari (int default_action) {
  // TranslatorExplanation don't translate letters in parentheses!!
  cout << _("(A)bort, (R)etry, (I)gnore?") << " ";

  // abort if no default has been specified
  if (default_action == -1) {
    default_action = 0;
  }
  
  // non-interactive mode
  if (gSettings.non_interactive) {
    // return the specified default
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
  while (cin.good()) {         // #269263
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
    cout << "?" << endl;
    DBG << "invalid answer" << endl;
  }

  return default_action;
}

// ----------------------------------------------------------------------------

bool read_bool_answer(const string & question, bool default_answer)
{
  cout << CLEARLN << question << " [y/n]: " << flush;

  // non-interactive mode: print the answer for convenience and return default
  if (gSettings.non_interactive)
  {
    cout << (default_answer ? 'y' : 'n') << endl;
    MIL << "answer (default): " << (default_answer ? 'y' : 'n') << endl;
    return default_answer;
  }

  istream & stm = cin;

  string c = "";
  bool been_here_before = false;
  while (stm.good () && c != "y" && c != "Y" && c != "N" && c != "n")
  {
    if (been_here_before)
      cerr << format(_("Invalid answer '%s', enter 'y' or 'n':")) % c << " ";
    c = zypp::str::getline (stm, zypp::str::TRIM);
    been_here_before = true;
  }

  MIL << "answer: " << c << endl;
  if (c == "y" || c == "Y")
    return true;
  else if (c == "n" || c == "N")
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
