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
using zypp::media::DownloadProgressReport;


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
      cout << label << " " <<description << std::endl;
      MIL << "media problem, url: " << url.asString() << std::endl;
      return MediaChangeReport::IGNORE;
    }
  };

struct DownloadProgressReportReceiver : public zypp::callback::ReceiveReport<DownloadProgressReport>
{

    virtual void start( const Url &/*file*/, Pathname /*localfile*/ )
    {
    }
    
    virtual bool progress(int value, const Url &file,
                          double dbps_avg,
                          double dbps_current)
    { 
        cout << file << " " << value << "% speed:" << dbps_current << " avg:" << dbps_avg << endl;
        return true; 
    }
    
    virtual Action problem( const Url &/*file*/
                            , Error /*error*/
                            , const std::string &description )
    {
        cout << "PROBLEM: " << description << endl;
        return ABORT; 
    }
    
    virtual void finish(
        const Url &/*file*/
        , Error /*error*/
        , const std::string &reason
        )
        {
            cout << "finish:" << endl;            
            cout << reason << endl;
        }
};

int main(int argc, char **argv)
{
    try
    {
      ZYpp::Ptr z = getZYpp();
    
      MediaChangeReportReceiver change_report;
      DownloadProgressReportReceiver progress_report;
      change_report.connect();
      progress_report.connect();
      
      MediaSetAccess access(Url("http://download.opensuse.org/update/11.1/rpm/x86_64"));
      OnMediaLocation loc;
      loc.setLocation("java-1_5_0-sun-1.5.0_update17-1.1.x86_64.rpm");
      //loc.setOptional(true);

      Fetcher fetcher;
      fetcher.enqueue(loc);
      fetcher.start("./", access);
      
    }
    catch ( const Exception &e )
    {
      ZYPP_CAUGHT(e);
      cout << e.msg() << endl;
      cout << e.historyAsString();
    }
    
    return 0;
}



