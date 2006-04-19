
#include <zypp/source/susetags/SelectionSelFileParser.h> 
#include <zypp/base/Logger.h>
#include <zypp/Pathname.h>

using namespace zypp;
using namespace zypp::source::susetags;

int main()
{
  parseSelection(Pathname("selfiles/packages"));
  DBG << "==============================================================" << std::endl;
  return 0;
}
