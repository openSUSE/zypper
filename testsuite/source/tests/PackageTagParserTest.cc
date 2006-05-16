#include <iostream>
#include "zypp/source/susetags/PackagesParser.h"
#include "zypp/target/store/serialize.h"
#include "zypp/base/Logger.h"
#include "zypp/base/LogControl.h"
#include "zypp/base/Exception.h"
#include "zypp/Pathname.h"
#include "zypp/Source.h"

using namespace std;
using namespace zypp;

void usage() {
  std::cerr << "PackageTagFileParserTest usage: "<< endl
      << "PackageTagFileParserTest file.sel" << endl;
}

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    usage();
    return 2;
  }
  
  Package::constPtr package;

  //zypp::base::LogControl::instance().logfile( "-" );
  Source_Ref s;

  try
  {
    package = zypp::source::susetags::parsePackages( s, Pathname(argv[1]) );
    cout << zypp::storage::toXML(package) << endl;
  }
  catch (Exception & excpt_r)
  {
    ZYPP_CAUGHT (excpt_r);
  }
  return 0;
}
