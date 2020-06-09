/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file	zypp/base/CleanerThread.cc
 */

#include <zypp/base/CleanerThread_p.h>
#include <zypp/zyppng/base/private/threaddata_p.h>
#include <zypp/zyppng/base/private/linuxhelpers_p.h>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>

struct CleanerData
{
  static CleanerData &instance ()
  {
    // C++11 requires that this is thread safe "magic statics"
    // this is a intentional leak and will live until the application exits
    static CleanerData *data( new CleanerData );
    return *data;
  }

  CleanerData ()
  {
    std::thread t ( [&](){
      this->run();
    } );
    t.detach(); //we will control the thread otherwise
  }

  void run ()
  {
    // force the kernel to pick another thread to handle those signals
    zyppng::blockSignalsForCurrentThread( { SIGTERM, SIGINT, SIGPIPE, } );

    zyppng::ThreadData::current().setName("Zypp-Cleaner");

    std::unique_lock<std::mutex> lk( _m );

    while ( true )
    {
      auto filter = []( pid_t pid ){
        int status = 0;
        int res = waitpid( pid, &status, WNOHANG );
         // we either got an error, or the child has exited, remove it from list
        bool removeMe = ( res == -1 || res == pid  );
        return removeMe;
      };
      _watchedPIDs.erase( std::remove_if( _watchedPIDs.begin(), _watchedPIDs.end(), filter ), _watchedPIDs.end() );

      if ( _watchedPIDs.size() )
        _cv.wait_for( lk, std::chrono::milliseconds(100) );
      else
        _cv.wait( lk );
    }
  }

  std::mutex _m; // < locks all data in CleanerData, do not access it without owning the mutex
  std::condition_variable _cv;

  std::vector<pid_t> _watchedPIDs;
};


void zypp::CleanerThread::watchPID( pid_t pid_r )
{
  CleanerData &data = CleanerData::instance();
  {
    std::lock_guard<std::mutex> guard( data._m );
    data._watchedPIDs.push_back( pid_r );
  }
  //wake the thread up
  data._cv.notify_one();
}
