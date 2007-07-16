#include "AliveCursor.h"
#include "zypper.h"

#include <ctype.h>
#include <iostream>
#include <sstream>

using namespace std;

void display_progress (const string& s, int percent) {
  static AliveCursor cursor;

  if ( percent == 100 )
    cout_v << CLEARLN << cursor.done() << " " << s;
  else
    cout_v << CLEARLN << cursor++ << " " << s;
  // dont display percents if invalid
  if (percent >= 0 && percent <= 100)
    cout_v << " [" << percent << "%]";
  cout_v << flush;
}

// ----------------------------------------------------------------------------

void display_tick (const string& s) {
  static AliveCursor cursor;

  cout_v << CLEARLN << cursor++ << " " << s;
  cout_v << flush;
}

// ----------------------------------------------------------------------------

void display_done (const string& s) {
  static AliveCursor cursor;

  cout_v << CLEARLN << cursor.done() << " " << s;
  cout_v << flush;
}

// ----------------------------------------------------------------------------

void display_done () {
  cout_v << endl;
}

// ----------------------------------------------------------------------------

//template<typename Action>
//Action ...
int read_action_ari (int default_action) {
  if (gSettings.verbosity >= 0 || !gSettings.non_interactive)
    // TranslatorExplanation don't translate letters in parentheses!!
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
      cout_n << c << endl;

      return default_action;
  }

  // interactive mode, ask user
  while (cin.good()) {		// #269263
    char c;
    cin >> c;
    c = tolower (c);
    if (c == 'a')
      return 0;
    else if (c == 'r')
      return 1;
    else if (c == 'i')
      return 2;
    cerr << _("Invalid answer. Choose letter a, r, or i.") << endl;
  }

  return default_action;
}

// ----------------------------------------------------------------------------

// return the default value on input failure
// TODO make this locale dependent?
bool read_bool_with_default (bool defval) {
  istream & stm = cin;

  string c = "";
  while (stm.good () && c != "y" && c != "Y" && c != "N" && c != "n")
    c = zypp::str::getline (stm, zypp::str::TRIM);
      
  if (c == "y" || c == "Y")
    return true;
  else if (c == "n" || c == "N")
    return false;
  else
    return defval;
}

// ----------------------------------------------------------------------------

// Read an answer (ynYN)
// Defaults to 'false'
bool readBoolAnswer()
{
  return read_bool_with_default (false);
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
