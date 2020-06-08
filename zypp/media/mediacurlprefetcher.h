#ifndef MEDIACURLPREFETCHER_H
#define MEDIACURLPREFETCHER_H


#include <zypp/Url.h>
#include <zypp/Pathname.h>
#include <zypp/ByteCount.h>
#include <zypp/media/TransferSettings.h>
#include <zypp/TmpPath.h>
#include <zypp/ZYppCallbacks.h>
#include <zypp/zyppng/thread/Wakeup>

#include <list>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <future>
#include <unordered_map>

namespace zyppng {
class Download;
}

namespace zypp{
namespace media {

class MediaCurlPrefetcher
{
public:

  struct RunningRequest;
  using CacheId  = size_t;
  using ReqQueue = std::vector< std::unique_ptr<RunningRequest> >;

  struct Request {
    CacheId cache;
    Url url;
    ByteCount expectedFileSize = 0;
    TransferSettings settings;
  };

  static MediaCurlPrefetcher &instance ();
  ~MediaCurlPrefetcher();

  CacheId createCache ();
  void closeCache ( const CacheId id );

  void precacheFiles( std::vector<Request> &&files );
  bool requireFile (const CacheId id, const Url &url, const Pathname &targetPath, callback::SendReport<DownloadProgressReport> & report );

private:
  MediaCurlPrefetcher();
  void workerMain ();

  /*!
   * Marks the request at \a reqIndex for cleanup
   * \note does NOT lock the mutex make sure that its locked already when called
   */
  ReqQueue::iterator markRequestForCleanup (const ReqQueue::iterator position );

  zyppng::Wakeup _wakeup;

  std::atomic_bool _stop;
  std::recursive_mutex _lock;

  ReqQueue _requests;
  ReqQueue _requestsToCleanup;
  std::vector<size_t> _cachesToClose;

  size_t _nextCacheId = 0;
  off_t _lastFinishedIndex = -1;
  off_t _firstWaitingIndex = -1;

  std::thread _fetcherThread;
  std::condition_variable _waitCond;
  filesystem::TmpDir _workingDir;
};


}}

#endif // MEDIACURLPREFETCHER_H
