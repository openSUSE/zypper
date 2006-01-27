
#include <zypp/source/susetags/SelectionSelFileParser.h> 
#include <zypp/base/Logger.h>
#include <zypp/Pathname.h>

using namespace zypp;
using namespace zypp::source::susetags;

int main()
{
  SelectionSelFileParser parser;
  SelectionSelFileParser::SelectionEntry entry;

  parser.parse(Pathname("selfiles/default.sel"), entry);
  DBG << "==============================================================" << std::endl;
  return 0;
}
