#include <iostream>
#include "zypp/source/susetags/PatternTagFileParser.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/Pathname.h"

using namespace std;
using namespace zypp;

int main()
{
  Pattern::Ptr pattern;
  try {
    pattern = zypp::source::susetags::parsePattern (Pathname("patfiles/default.pat"));
    cout << *pattern << endl;
    pattern = zypp::source::susetags::parsePattern (Pathname("patfiles/NOTTHERE.pat"));
    cout << *pattern << endl;
    
    pattern = zypp::source::susetags::parsePattern (Pathname("patfiles/base-10-33.i586.pat"));
    if (pattern->userVisible())
    {
      ERR << "Error parsing userVisible" << std::endl;
      return 1;
    }
    cout << *pattern << endl;
  }
  catch (Exception & excpt_r) {
    ZYPP_CAUGHT (excpt_r);
  }

  return 0;
}
