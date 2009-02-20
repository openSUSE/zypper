#include <sys/time.h>

#include <iostream>
#include <fstream>

#include <zypp/base/Logger.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>

#include "zypp/Product.h"
#include "zypp/Package.h"
#include "zypp/Fetcher.h"
#include "zypp/TmpPath.h"
#include "zypp/ProgressData.h"

#include "zypp/sat/Pool.h"

#include "zypp/ZYppCallbacks.h"

using namespace std;
using namespace zypp;
using namespace zypp::repo;
using zypp::media::MediaChangeReport;


bool result_cb( const ResObject::Ptr &r )
{
  cout << r << endl;
}

struct MediaChangeReportReceiver : public zypp::callback::ReceiveReport<MediaChangeReport>
  {
    virtual MediaChangeReport::Action
    requestMedia(zypp::Url & url,
                 unsigned                         mediumNr,
                 const std::string &              label,
                 MediaChangeReport::Error         error,
                 const std::string &              description,
                 const std::vector<std::string> & devices,
                 unsigned int &                   index)
    {
      cout << std::endl;
      MIL << "media problem, url: " << url.asString() << std::endl;
      return MediaChangeReport::IGNORE;
    }
  };


int main(int argc, char **argv)
{
    try
    {
      ZYpp::Ptr z = getZYpp();
    
      MediaChangeReportReceiver report;
      report.connect();
      

      Fetcher fetcher;
      MediaSetAccess access(Url("http://ftp.kernel.org/pub"));
      filesystem::TmpDir tmp;
      
      OnMediaLocation loc;
      loc.setLocation("/README2");
      loc.setOptional(true);
      
      fetcher.enqueue(loc);
      fetcher.start(tmp.path(), access);
      
    }
    catch ( const Exception &e )
    {
      ZYPP_CAUGHT(e);
      cout << e.msg() << endl;
    }
    
    return 0;
}



