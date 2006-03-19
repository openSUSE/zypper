#include <iostream>
#include "zypp/source/susetags/PatternTagFileParser.h"
#include "zypp/base/Logger.h"
#include "zypp/base/LogControl.h"
#include "zypp/base/Exception.h"
#include "zypp/Pathname.h"

using namespace std;
using namespace zypp;

int main()
{
  zypp::base::LogControl::instance().logfile( "-" );

  Pattern::Ptr pattern;
  Source_Ref s;

  try {
    pattern = zypp::source::susetags::parsePattern( s, Pathname("patfiles/default.pat"));
    cout << *pattern << endl;
    pattern = zypp::source::susetags::parsePattern( s, Pathname("patfiles/NOTTHERE.pat"));
    cout << *pattern << endl;
    
    pattern = zypp::source::susetags::parsePattern( s, Pathname("patfiles/base-10-33.i586.pat"));
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
