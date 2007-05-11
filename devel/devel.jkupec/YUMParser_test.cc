#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"
#include "zypp/base/Logger.h"
#include "zypp/base/LogControl.h"
#include "zypp/parser/yum/PrimaryFileReader.h"
#include "YUMParser.h"
#include "zypp/parser/ParserProgress.h"
#include "zypp/base/Measure.h"



#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "yumparsertest"

using namespace std;
using namespace zypp;
using namespace zypp::parser::yum;
using zypp::debug::Measure;

bool progress_function(ProgressData::value_type p)
{
  cout << "Parsing YUM source [" << p << "%]" << endl;
//  cout << "\rParsing YUM source [" << p << "%]" << flush;
  return true;
}

int main(int argc, char **argv)
{
  base::LogControl::instance().logfile("yumparsertest.log");
  
  if (argc < 2)
  {
    cout << "usage: yumparsertest path/to/yumsourcedir" << endl << endl;
    return 1;
  }

  try
  {
    ZYpp::Ptr z = getZYpp();

    Measure open_catalog_timer("CacheStore: lookupOrAppendCatalog");

    cache::CacheStore store(getenv("PWD"));
    data::RecordId catalog_id = store.lookupOrAppendCatalog( Url("http://some.url"), "/");

    open_catalog_timer.stop();

    MIL << "creating PrimaryFileParser" << endl;
    Measure parse_primary_timer("primary.xml.gz parsing");

    parser::yum::YUMParser parser( catalog_id, store, &progress_function);
    parser.start(argv[1]);

    parse_primary_timer.stop();

    cout << endl;
  }
  catch ( const Exception &e )
  {
    cout << "Oops! " << e.msg() << std::endl;
  }

  return 0;
}

// vim: set ts=2 sts=2 sw=2 et ai:
