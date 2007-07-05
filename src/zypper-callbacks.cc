#include "AliveCursor.h"
#include "zypper.h"

#include <ctype.h>
#include <iostream>
#include <sstream>

using namespace std;

void display_progress (const string& s, int percent) {
  static AliveCursor cursor;

  cout_v << CLEARLN << cursor++ << " " << s << "[" << percent << "%]" << flush;
}

void display_done () {
  cout_v << endl;
}

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

string to_string (zypp::Resolvable::constPtr resolvable) {
  ostringstream ss;
  ss << *resolvable;
  return ss.str ();
}

// Local Variables:
// c-basic-offset: 2
// End:
