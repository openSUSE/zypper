#include <iostream>
#include "zypp/source/susetags/SelectionTagFileParser.h"
#include "zypp/target/store/serialize.h"
#include "zypp/base/Logger.h"
#include "zypp/base/LogControl.h"
#include "zypp/base/Exception.h"
#include "zypp/Pathname.h"
#include "zypp/parser/ParserProgress.h"

using namespace std;
using namespace zypp;

using namespace zypp::parser;

void progress( int p )
{
  cout << p << "%" << endl;
}

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
  
  Selection::constPtr selection;

  //zypp::base::LogControl::instance().logfile( "-" );
  Source_Ref s;

  try
  {
    ParserProgress::Ptr pptr;
    pptr.reset( new ParserProgress( &progress ) );
    
    selection = zypp::source::susetags::parseSelection( pptr, s, Pathname(argv[1]) );
    cout << zypp::storage::toXML(selection) << endl;
  }
  catch (Exception & excpt_r)
  {
    ZYPP_CAUGHT (excpt_r);
  }
  return 0;
}
