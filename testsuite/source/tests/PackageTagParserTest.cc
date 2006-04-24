#include <iostream>
#include "zypp/source/susetags/PackageTagFileParser.h"
#include "zypp/target/store/serialize.h"
#include "zypp/base/Logger.h"
#include "zypp/base/LogControl.h"
#include "zypp/base/Exception.h"
#include "zypp/Pathname.h"

using namespace std;
using namespace zypp;

void usage() {
  std::cerr << "SelectionTagFileParserTest usage: "<< endl
      << "SelectionTagFileParserTest file.sel" << endl;
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
    selection = zypp::source::susetags::parseSelection( s, Pathname(argv[1]) );
    cout << zypp::storage::toXML(selection) << endl;
  }
  catch (Exception & excpt_r)
  {
    ZYPP_CAUGHT (excpt_r);
  }
  return 0;
}
