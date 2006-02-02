#include <iostream>
#include "zypp/source/susetags/SelectionTagFileParser.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/Pathname.h"

using namespace std;
using namespace zypp;

int main()
{
  Selection::Ptr selection;

  try {
  selection = zypp::source::susetags::parseSelection (Pathname("selfiles/default.sel"));
  cout << *selection << endl;
  selection = zypp::source::susetags::parseSelection (Pathname("selfiles/Office.sel"));
  cout << *selection << endl;
  selection = zypp::source::susetags::parseSelection (Pathname("selfiles/X11.sel"));
  cout << *selection << endl;
  selection = zypp::source::susetags::parseSelection (Pathname("selfiles/NOTTHERE.sel"));
  cout << *selection << endl;
  }
  catch (Exception & excpt_r) {
    ZYPP_CAUGHT (excpt_r);
  }
  return 0;
}
