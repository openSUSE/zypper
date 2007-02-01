#include "AliveCursor.h"
#include "zmart.h"

#include <ctype.h>
#include <iostream>
#include <sstream>

using namespace std;

void display_progress (const string& s, int percent) {
  static AliveCursor cursor;

  cerr_v << CLEARLN << cursor++ << " " << s << "[" << percent << "%]" << flush;
}

void display_done () {
  cerr_v << endl;
}

//template<typename Action>
//Action ...
int read_action_ari (int default_action) {
  cerr << _("(A)bort, (R)etry, (I)gnore?") << " "; // don't translate letters in parentheses!!

	// non-interactive mode
	if (gSettings.non_interactive) {
		// abort if no default has been specified
		if (default_action == -1) {
			// print the answer for convenience
			cout << 'a' << endl;

			return 0; 
		}
		// return the specified default
		else {
			char c;
			switch (default_action) {
				case 0: c = 'a'; break;
    		case 1: c = 'r'; break;
    		case 2: c = 'i'; break;
    		default: c = '?';
			}
			// print the answer for conveniecne
			cout << c << endl;

			return default_action;
		}
	}

	// interactive mode, ask user
  while (true) {
    char c;
    cin >> c;
    c = tolower (c);
    if (c == 'a')
      return 0;
    else if (c == 'r')
      return 1;
    else if (c == 'i')
      return 2;
    cout << "?" << endl;
  }
}

string to_string (zypp::Resolvable::constPtr resolvable) {
  ostringstream ss;
  ss << *resolvable;
  return ss.str ();
}

// Local Variables:
// c-basic-offset: 2
// End:
