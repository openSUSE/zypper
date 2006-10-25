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
int read_action_ari () {
  // TODO: respect --yes (abort)
  while (true) {
    cerr << "(A)bort, (R)etry, (I)gnore?" << endl;
    char c;
    cin >> c;
    c = tolower (c);
    if (c == 'a')
      return 0;
    else if (c == 'r')
      return 1;
    else if (c == 'i')
      return 2;
    cerr << "?" << endl;
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
