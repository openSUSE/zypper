
#include <zypp/parser/tagfile/ProductMetadataParser.h> 

int main()
{
  zypp::parser::tagfile::ProductMetadataParser parser;
  parser.parse("tagfiles/products/content.1.txt");

  return 0;
}
