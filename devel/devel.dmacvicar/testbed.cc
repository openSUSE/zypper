#include <sys/time.h>

#include <iostream>
#include <fstream>

#include <zypp/base/Logger.h>
#include <zypp/ZYpp.h>
#include <zypp/ZYppFactory.h>

#include "zypp/Product.h"
#include "zypp/detail/PackageImplIf.h"
#include "zypp/Package.h"
#include "zypp/repo/cached/RepoImpl.h"
#include "zypp/data/ResolvableData.h"

#include "zypp/TmpPath.h"
#include "zypp/ProgressData.h"
#include "zypp/parser/yum/RepoParser.h"
#include "zypp/repo/yum/Downloader.h"

using namespace std;
using namespace zypp;
using namespace zypp::repo;
using namespace zypp::repo::cached;

bool task_receiver( ProgressData::value_type v )
{
  cout << "got ->" << v << endl;
  return( v <= 100 ); // Abort if ( v > 100 )
}

struct ProgressDataWrapper
{
  ProgressDataWrapper( ProgressData &pd,
                        ProgressData::value_type percentage )
    : _percentage(percentage),
      _last_value(0),
      _pd(pd)
      
  {

  }

  bool operator()( ProgressData::value_type v )
  {
    MIL << endl;
    ProgressData::value_type increment = v - _last_value;
    int real_increment = (int)((float)(_percentage*increment)/100);
    MIL << "Value: " << v << ". Increment of " << increment << ". Real of " << real_increment << endl;
    _last_value = v;
    return _pd.incr(real_increment);
  }

  ProgressData::value_type _percentage;
  ProgressData::value_type _last_value;
  ProgressData &_pd;
};

// task one is 80%
void small_task_one( ProgressData::ReceiverFnc rcv )
{
  MIL << "Small task one" << endl;
  ProgressData progress(100);
  progress.sendTo(rcv);
  progress.toMin();
  progress.set(10);
  progress.set(20);
  progress.set(30);
  progress.set(40);
  progress.set(70);
  progress.toMax();
}

// task one is 20%
void small_task_two( ProgressData::ReceiverFnc rcv )
{
  MIL << "Small task two" << endl;
  ProgressData progress(100);
  progress.sendTo(rcv);
  progress.toMin();
  progress.set(10);
  progress.set(20);
  progress.set(70);
  progress.toMax();
}


void big_task( ProgressData::ReceiverFnc rcv )
{
  ProgressData progress(100);
  progress.sendTo(rcv);
  progress.toMin();
  ProgressDataWrapper wrapper1( progress, 80 );
  small_task_one( wrapper1 );
  ProgressDataWrapper wrapper2( progress, 20 );
  small_task_two( wrapper2 );
  progress.toMax();
}

int main(int argc, char **argv)
{
    try
    {
      //ZYpp::Ptr z = getZYpp();
    
      //z->initializeTarget("/");
//       filesystem::TmpDir tmpdir;
//       yum::Downloader downloader(Url("http://ftp.gwdg.de/pub/suse/update/10.2"), "/");
//       downloader.download(tmpdir.path(), &yum_download_receiver);
      big_task(&task_receiver);
      //ResStore res = z->target()->resolvables();
      //MIL << res.size() << " resolvables" << endl;

    }
    catch ( const Exception &e )
    {
      ZYPP_CAUGHT(e);
      cout << e.msg() << endl;
    }
    
    return 0;
}



