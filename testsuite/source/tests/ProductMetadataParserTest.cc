
#include "zypp/source/susetags/ProductMetadataParser.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/Pathname.h"

using namespace zypp;
using namespace zypp::source::susetags;

int main()
{
  ProductMetadataParser parser;
  ProductMetadataParser::ProductEntry entry;

  try {
  parser.parse(Pathname("products/content.1.txt"), entry);
  DBG << "arch: " << entry.arch["x86_64"].size() << std::endl;
  parser.parse(Pathname("products/NOTTHERE.txt"), entry);
  DBG << "arch: " << entry.arch["x86_64"].size() << std::endl;
  }
  catch (Exception & excpt_r) {
	ZYPP_CAUGHT (excpt_r);
  }
return 0;
}
