
#include <iostream>
#include "zypp/Source.h"
#include "zypp/Product.h"
#include "zypp/source/susetags/ProductMetadataParser.h"
#include "zypp/base/Logger.h"
#include "zypp/base/LogControl.h"
#include "zypp/base/Exception.h"
#include "zypp/Pathname.h"
#include "zypp/Product.h"

using namespace zypp;
using namespace zypp::source::susetags;

Product::Ptr parseContentFile(const Pathname &_content_file)
{
  ProductMetadataParser p;
  p.parse( _content_file, Source_Ref::noSource );
  return p.result;
}

int main()
{
  Product::Ptr product;

  zypp::base::LogControl::instance().logfile( "-" );

  try {
  product = parseContentFile(Pathname("products/content.1.txt"));
  product = parseContentFile(Pathname("products/content.2.txt"));
  product = parseContentFile(Pathname("products/content.3.txt"));
  product = parseContentFile(Pathname("products/content.4.txt"));
  //DBG << "arch: " << entry.arch["x86_64"].size() << std::endl;
  product = parseContentFile(Pathname("products/NOTTHERE.txt"));
  //DBG << "arch: " << entry.arch["x86_64"].size() << std::endl;
  }
  catch (Exception & excpt_r) {
	ZYPP_CAUGHT (excpt_r);
  }
return 0;
}
