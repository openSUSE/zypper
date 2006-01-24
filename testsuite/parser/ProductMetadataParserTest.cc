
#include <zypp/parser/tagfile/ProductMetadataParser.h> 
#include <zypp/base/Logger.h>
#include <zypp/Pathname.h>

using namespace zypp;
using namespace zypp::parser::tagfile;

int main()
{
  ProductMetadataParser parser;
  ProductMetadataParser::ProductEntry entry;

  parser.parse(Pathname("tagfiles/products/content.1.txt"), entry);
  DBG << "arch: " << entry.arch["x86_64"].size() << std::endl;
return 0;
}
