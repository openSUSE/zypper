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

bool progress_function(int p)
{
//  cout << "\r                                       " << flush;
  cout << "\rParsing primary.xml.gz [" << p << "%]" << flush;
//  MIL << p << "%" << endl;
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
    parser::ParserProgress::Ptr progress;
    progress.reset(new parser::ParserProgress(&progress_function));
    Measure parse_primary_timer("primary.xml.gz parsing");

    parser::yum::YUMParser parser( catalog_id, store);
    parser.start(argv[1], progress);

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
