
#include <iostream>
#include "zypp/source/susetags/ProductMetadataParser.h"
#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/Pathname.h"
#include "zypp/Product.h"

using namespace zypp;
using namespace zypp::source::susetags;

int main()
{
  ProductMetadataParser parser;
  Product::Ptr product;

  Source_Ref src;

  try {
  product = parseContentFile(Pathname("products/content.1.txt"), src);
  //DBG << "arch: " << entry.arch["x86_64"].size() << std::endl;
  product = parseContentFile(Pathname("products/NOTTHERE.txt"), src);
  //DBG << "arch: " << entry.arch["x86_64"].size() << std::endl;
  }
  catch (Exception & excpt_r) {
	ZYPP_CAUGHT (excpt_r);
  }
return 0;
}
