#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"
#include "zypp/base/Logger.h"
#include "zypp/base/LogControl.h"
#include "zypp/parser/yum/PrimaryFileReader.h"
#include "YUMParser.h"


#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "yumparsertest"

using namespace std;
using namespace zypp;
using namespace zypp::parser::yum;

bool progress_function(int p)
{
  MIL << p << "%" << endl;
}

int main(int argc, char **argv)
{
  base::LogControl::instance().logfile("yumparsertest.log");

  try
  {
    ZYpp::Ptr z = getZYpp();
//, bind( &YUMDownloader::patches_Callback, this, _1, _2));

//    Pathname dbfile = Pathname(getenv("PWD")) + "data.db";
    cache::CacheStore store(getenv("PWD"));
    data::RecordId catalog_id = store.lookupOrAppendCatalog( Url("http://www.google.com"), "/");

    MIL << "creating PrimaryFileParser" << endl;
    parser::yum::YUMParser parser( catalog_id, store);
    parser.start(argv[1], &progress_function);

/*
      YUMDownloader downloader(Url(argv[1]), "/");
      downloader.download(argv[2]);*/
  }
  catch ( const Exception &e )
  {
    cout << "Oops! " << e.msg() << std::endl;
  }

  return 0;
}
