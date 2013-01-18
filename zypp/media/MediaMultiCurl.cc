/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaMultiCurl.cc
 *
*/

#include <ctype.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <vector>
#include <iostream>
#include <algorithm>


#include "zypp/ZConfig.h"
#include "zypp/base/Logger.h"
#include "zypp/base/UserRequestException.h"
#include "zypp/media/MediaMultiCurl.h"
#include "zypp/media/MetaLinkParser.h"

using namespace std;
using namespace zypp::base;

#undef CURLVERSION_AT_LEAST
#define CURLVERSION_AT_LEAST(M,N,O) LIBCURL_VERSION_NUM >= ((((M)<<8)+(N))<<8)+(O)

namespace zypp {
  namespace media {


//////////////////////////////////////////////////////////////////////


class multifetchrequest;

// Hack: we derive from MediaCurl just to get the storage space for
// settings, url, curlerrors and the like

class multifetchworker : MediaCurl {
  friend class multifetchrequest;

public:
  multifetchworker(int no, multifetchrequest &request, const Url &url);
  ~multifetchworker();
  void nextjob();
  void run();
  bool checkChecksum();
  bool recheckChecksum();
  void disableCompetition();

  void checkdns();
  void adddnsfd(fd_set &rset, int &maxfd);
  void dnsevent(fd_set &rset);

  int _workerno;

  int _state;
  bool _competing;

  size_t _blkno;
  off_t _blkstart;
  size_t _blksize;
  bool _noendrange;

  double _blkstarttime;
  size_t _blkreceived;
  off_t  _received;

  double _avgspeed;
  double _maxspeed;

  double _sleepuntil;

private:
  void stealjob();

  size_t writefunction(void *ptr, size_t size);
  static size_t _writefunction(void *ptr, size_t size, size_t nmemb, void *stream);

  size_t headerfunction(char *ptr, size_t size);
  static size_t _headerfunction(void *ptr, size_t size, size_t nmemb, void *stream);

  multifetchrequest *_request;
  int _pass;
  string _urlbuf;
  off_t _off;
  size_t _size;
  Digest _dig;

  pid_t _pid;
  int _dnspipe;
};

#define WORKER_STARTING 0
#define WORKER_LOOKUP   1
#define WORKER_FETCH    2
#define WORKER_DISCARD  3
#define WORKER_DONE     4
#define WORKER_SLEEP    5
#define WORKER_BROKEN   6



class multifetchrequest {
public:
  multifetchrequest(const MediaMultiCurl *context, const Pathname &filename, const Url &baseurl, CURLM *multi, FILE *fp, callback::SendReport<DownloadProgressReport> *report, MediaBlockList *blklist, off_t filesize);
  ~multifetchrequest();

  void run(std::vector<Url> &urllist);

protected:
  friend class multifetchworker;

  const MediaMultiCurl *_context;
  const Pathname _filename;
  Url _baseurl;

  FILE *_fp;
  callback::SendReport<DownloadProgressReport> *_report;
  MediaBlockList *_blklist;
  off_t _filesize;

  CURLM *_multi;

  std::list<multifetchworker *> _workers;
  bool _stealing;
  bool _havenewjob;

  size_t _blkno;
  off_t _blkoff;
  size_t _activeworkers;
  size_t _lookupworkers;
  size_t _sleepworkers;
  double _minsleepuntil;
  bool _finished;
  off_t _totalsize;
  off_t _fetchedsize;
  off_t _fetchedgoodsize;

  double _starttime;
  double _lastprogress;

  double _lastperiodstart;
  double _lastperiodfetched;
  double _periodavg;

public:
  double _timeout;
  double _connect_timeout;
  double _maxspeed;
  int _maxworkers;
};

#define BLKSIZE		131072
#define MAXURLS		10


//////////////////////////////////////////////////////////////////////

static double
currentTime()
{
  struct timeval tv;
  if (gettimeofday(&tv, NULL))
    return 0;
  return tv.tv_sec + tv.tv_usec / 1000000.;
}

size_t
multifetchworker::writefunction(void *ptr, size_t size)
{
  size_t len, cnt;
  if (_state == WORKER_BROKEN)
    return size ? 0 : 1;

  double now = currentTime();

  len = size > _size ? _size : size;
  if (!len)
    {
      // kill this job?
      return size;
    }

  if (_blkstart && _off == _blkstart)
    {
      // make sure that the server replied with "partial content"
      // for http requests
      char *effurl;
      (void)curl_easy_getinfo(_curl, CURLINFO_EFFECTIVE_URL, &effurl);
      if (effurl && !strncasecmp(effurl, "http", 4))
	{
	  long statuscode = 0;
	  (void)curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &statuscode);
	  if (statuscode != 206)
	    return size ? 0 : 1;
	}
    }

  _blkreceived += len;
  _received += len;

  _request->_lastprogress = now;

  if (_state == WORKER_DISCARD || !_request->_fp)
    {
      // block is no longer needed
      // still calculate the checksum so that we can throw out bad servers
      if (_request->_blklist)
        _dig.update((const char *)ptr, len);
      _off += len;
      _size -= len;
      return size;
    }
  if (fseeko(_request->_fp, _off, SEEK_SET))
    return size ? 0 : 1;
  cnt = fwrite(ptr, 1, len, _request->_fp);
  if (cnt > 0)
    {
      _request->_fetchedsize += cnt;
      if (_request->_blklist)
        _dig.update((const char *)ptr, cnt);
      _off += cnt;
      _size -= cnt;
      if (cnt == len)
	return size;
    }
  return cnt;
}

size_t
multifetchworker::_writefunction(void *ptr, size_t size, size_t nmemb, void *stream)
{
  multifetchworker *me = reinterpret_cast<multifetchworker *>(stream);
  return me->writefunction(ptr, size * nmemb);
}

size_t
multifetchworker::headerfunction(char *p, size_t size)
{
  size_t l = size;
  if (l > 9 && !strncasecmp(p, "Location:", 9))
    {
      string line(p + 9, l - 9);
      if (line[l - 10] == '\r')
	line.erase(l - 10, 1);
      DBG << "#" << _workerno << ": redirecting to" << line << endl;
      return size;
    }
  if (l <= 14 || l >= 128 || strncasecmp(p, "Content-Range:", 14) != 0)
    return size;
  p += 14;
  l -= 14;
  while (l && (*p == ' ' || *p == '\t'))
    p++, l--;
  if (l < 6 || strncasecmp(p, "bytes", 5))
    return size;
  p += 5;
  l -= 5;
  char buf[128];
  memcpy(buf, p, l);
  buf[l] = 0;
  unsigned long long start, off, filesize;
  if (sscanf(buf, "%llu-%llu/%llu", &start, &off, &filesize) != 3)
    return size;
  if (_request->_filesize == (off_t)-1)
    {
      WAR << "#" << _workerno << ": setting request filesize to " << filesize << endl;
      _request->_filesize = filesize;
      if (_request->_totalsize == 0 && !_request->_blklist)
	_request->_totalsize = filesize;
    }
  if (_request->_filesize != (off_t)filesize)
    {
      DBG << "#" << _workerno << ": filesize mismatch" << endl;
      _state = WORKER_BROKEN;
      strncpy(_curlError, "filesize mismatch", CURL_ERROR_SIZE);
    }
  return size;
}

size_t
multifetchworker::_headerfunction(void *ptr, size_t size, size_t nmemb, void *stream)
{
  multifetchworker *me = reinterpret_cast<multifetchworker *>(stream);
  return me->headerfunction((char *)ptr, size * nmemb);
}

multifetchworker::multifetchworker(int no, multifetchrequest &request, const Url &url)
: MediaCurl(url, Pathname())
{
  _workerno = no;
  _request = &request;
  _state = WORKER_STARTING;
  _competing = false;
  _off = _blkstart = 0;
  _size = _blksize = 0;
  _pass = 0;
  _blkno = 0;
  _pid = 0;
  _dnspipe = -1;
  _blkreceived = 0;
  _received = 0;
  _blkstarttime = 0;
  _avgspeed = 0;
  _sleepuntil = 0;
  _maxspeed = _request->_maxspeed;
  _noendrange = false;

  Url curlUrl( clearQueryString(url) );
  _urlbuf = curlUrl.asString();
  _curl = _request->_context->fromEasyPool(_url.getHost());
  if (_curl)
    DBG << "reused worker from pool" << endl;
  if (!_curl && !(_curl = curl_easy_init()))
    {
      _state = WORKER_BROKEN;
      strncpy(_curlError, "curl_easy_init failed", CURL_ERROR_SIZE);
      return;
    }
  try
    {
      setupEasy();
    }
  catch (Exception &ex)
    {
      curl_easy_cleanup(_curl);
      _curl = 0;
      _state = WORKER_BROKEN;
      strncpy(_curlError, "curl_easy_setopt failed", CURL_ERROR_SIZE);
      return;
    }
  curl_easy_setopt(_curl, CURLOPT_PRIVATE, this);
  curl_easy_setopt(_curl, CURLOPT_URL, _urlbuf.c_str());
  curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, &_writefunction);
  curl_easy_setopt(_curl, CURLOPT_WRITEDATA, this);
  if (_request->_filesize == off_t(-1) || !_request->_blklist || !_request->_blklist->haveChecksum(0))
    {
      curl_easy_setopt(_curl, CURLOPT_HEADERFUNCTION, &_headerfunction);
      curl_easy_setopt(_curl, CURLOPT_HEADERDATA, this);
    }
  // if this is the same host copy authorization
  // (the host check is also what curl does when doing a redirect)
  // (note also that unauthorized exceptions are thrown with the request host)
  if (url.getHost() == _request->_context->_url.getHost())
    {
      _settings.setUsername(_request->_context->_settings.username());
      _settings.setPassword(_request->_context->_settings.password());
      _settings.setAuthType(_request->_context->_settings.authType());
      if ( _settings.userPassword().size() )
	{
	  curl_easy_setopt(_curl, CURLOPT_USERPWD, _settings.userPassword().c_str());
	  string use_auth = _settings.authType();
	  if (use_auth.empty())
	    use_auth = "digest,basic";        // our default
	  long auth = CurlAuthData::auth_type_str2long(use_auth);
	  if( auth != CURLAUTH_NONE)
	  {
	    DBG << "#" << _workerno << ": Enabling HTTP authentication methods: " << use_auth
		<< " (CURLOPT_HTTPAUTH=" << auth << ")" << std::endl;
	    curl_easy_setopt(_curl, CURLOPT_HTTPAUTH, auth);
	  }
	}
    }
  checkdns();
}

multifetchworker::~multifetchworker()
{
  if (_curl)
    {
      if (_state == WORKER_FETCH || _state == WORKER_DISCARD)
        curl_multi_remove_handle(_request->_multi, _curl);
      if (_state == WORKER_DONE || _state == WORKER_SLEEP)
	{
#if CURLVERSION_AT_LEAST(7,15,5)
	  curl_easy_setopt(_curl, CURLOPT_MAX_RECV_SPEED_LARGE, (curl_off_t)0);
#endif
	  curl_easy_setopt(_curl, CURLOPT_PRIVATE, (void *)0);
	  curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, (void *)0);
	  curl_easy_setopt(_curl, CURLOPT_WRITEDATA, (void *)0);
	  curl_easy_setopt(_curl, CURLOPT_HEADERFUNCTION, (void *)0);
	  curl_easy_setopt(_curl, CURLOPT_HEADERDATA, (void *)0);
          _request->_context->toEasyPool(_url.getHost(), _curl);
	}
      else
        curl_easy_cleanup(_curl);
      _curl = 0;
    }
  if (_pid)
    {
      kill(_pid, SIGKILL);
      int status;
      while (waitpid(_pid, &status, 0) == -1)
        if (errno != EINTR)
	  break;
      _pid = 0;
    }
  if (_dnspipe != -1)
    {
      close(_dnspipe);
      _dnspipe = -1;
    }
  // the destructor in MediaCurl doesn't call disconnect() if
  // the media is not attached, so we do it here manually
  disconnectFrom();
}

static inline bool env_isset(string name)
{
  const char *s = getenv(name.c_str());
  return s && *s ? true : false;
}

void
multifetchworker::checkdns()
{
  string host = _url.getHost();

  if (host.empty())
    return;

  if (_request->_context->isDNSok(host))
    return;

  // no need to do dns checking for numeric hosts
  char addrbuf[128];
  if (inet_pton(AF_INET, host.c_str(), addrbuf) == 1)
    return;
  if (inet_pton(AF_INET6, host.c_str(), addrbuf) == 1)
    return;

  // no need to do dns checking if we use a proxy
  if (!_settings.proxy().empty())
    return;
  if (env_isset("all_proxy") || env_isset("ALL_PROXY"))
    return;
  string schemeproxy = _url.getScheme() + "_proxy";
  if (env_isset(schemeproxy))
    return;
  if (schemeproxy != "http_proxy")
    {
      std::transform(schemeproxy.begin(), schemeproxy.end(), schemeproxy.begin(), ::toupper);
      if (env_isset(schemeproxy))
	return;
    }

  DBG << "checking DNS lookup of " << host << endl;
  int pipefds[2];
  if (pipe(pipefds))
    {
      _state = WORKER_BROKEN;
      strncpy(_curlError, "DNS pipe creation failed", CURL_ERROR_SIZE);
      return;
    }
  _pid = fork();
  if (_pid == pid_t(-1))
    {
      close(pipefds[0]);
      close(pipefds[1]);
      _pid = 0;
      _state = WORKER_BROKEN;
      strncpy(_curlError, "DNS checker fork failed", CURL_ERROR_SIZE);
      return;
    }
  else if (_pid == 0)
    {
      close(pipefds[0]);
      // XXX: close all other file descriptors
      struct addrinfo *ai, aihints;
      memset(&aihints, 0, sizeof(aihints));
      aihints.ai_family = PF_UNSPEC;
      int tstsock = socket(PF_INET6, SOCK_DGRAM | SOCK_CLOEXEC, 0);
      if (tstsock == -1)
	aihints.ai_family = PF_INET;
      else
	close(tstsock);
      aihints.ai_socktype = SOCK_STREAM;
      aihints.ai_flags = AI_CANONNAME;
      unsigned int connecttimeout = _request->_connect_timeout;
      if (connecttimeout)
	alarm(connecttimeout);
      signal(SIGALRM, SIG_DFL);
      if (getaddrinfo(host.c_str(), NULL, &aihints, &ai))
        _exit(1);
      _exit(0);
    }
  close(pipefds[1]);
  _dnspipe = pipefds[0];
  _state = WORKER_LOOKUP;
}

void
multifetchworker::adddnsfd(fd_set &rset, int &maxfd)
{
  if (_state != WORKER_LOOKUP)
    return;
  FD_SET(_dnspipe, &rset);
  if (maxfd < _dnspipe)
    maxfd = _dnspipe;
}

void
multifetchworker::dnsevent(fd_set &rset)
{

  if (_state != WORKER_LOOKUP || !FD_ISSET(_dnspipe, &rset))
    return;
  int status;
  while (waitpid(_pid, &status, 0) == -1)
    {
      if (errno != EINTR)
        return;
    }
  _pid = 0;
  if (_dnspipe != -1)
    {
      close(_dnspipe);
      _dnspipe = -1;
    }
  if (!WIFEXITED(status))
    {
      _state = WORKER_BROKEN;
      strncpy(_curlError, "DNS lookup failed", CURL_ERROR_SIZE);
      _request->_activeworkers--;
      return;
    }
  int exitcode = WEXITSTATUS(status);
  DBG << "#" << _workerno << ": DNS lookup returned " << exitcode << endl;
  if (exitcode != 0)
    {
      _state = WORKER_BROKEN;
      strncpy(_curlError, "DNS lookup failed", CURL_ERROR_SIZE);
      _request->_activeworkers--;
      return;
    }
  _request->_context->setDNSok(_url.getHost());
  nextjob();
}

bool
multifetchworker::checkChecksum()
{
  // DBG << "checkChecksum block " << _blkno << endl;
  if (!_blksize || !_request->_blklist)
    return true;
  return _request->_blklist->verifyDigest(_blkno, _dig);
}

bool
multifetchworker::recheckChecksum()
{
  // DBG << "recheckChecksum block " << _blkno << endl;
  if (!_request->_fp || !_blksize || !_request->_blklist)
    return true;
  if (fseeko(_request->_fp, _blkstart, SEEK_SET))
    return false;
  char buf[4096];
  size_t l = _blksize;
  _request->_blklist->createDigest(_dig);	// resets digest
  while (l)
    {
      size_t cnt = l > sizeof(buf) ? sizeof(buf) : l;
      if (fread(buf, cnt, 1, _request->_fp) != 1)
	return false;
      _dig.update(buf, cnt);
      l -= cnt;
    }
  return _request->_blklist->verifyDigest(_blkno, _dig);
}


void
multifetchworker::stealjob()
{
  if (!_request->_stealing)
    {
      DBG << "start stealing!" << endl;
      _request->_stealing = true;
    }
  multifetchworker *best = 0;
  std::list<multifetchworker *>::iterator workeriter = _request->_workers.begin();
  double now = 0;
  for (; workeriter != _request->_workers.end(); ++workeriter)
    {
      multifetchworker *worker = *workeriter;
      if (worker == this)
	continue;
      if (worker->_pass == -1)
	continue;	// do not steal!
      if (worker->_state == WORKER_DISCARD || worker->_state == WORKER_DONE || worker->_state == WORKER_SLEEP || !worker->_blksize)
	continue;	// do not steal finished jobs
      if (!worker->_avgspeed && worker->_blkreceived)
	{
	  if (!now)
	    now = currentTime();
	  if (now > worker->_blkstarttime)
	    worker->_avgspeed = worker->_blkreceived / (now - worker->_blkstarttime);
	}
      if (!best || best->_pass > worker->_pass)
	{
          best = worker;
	  continue;
	}
      if (best->_pass < worker->_pass)
	continue;
      // if it is the same block, we want to know the best worker, otherwise the worst
      if (worker->_blkstart == best->_blkstart)
	{
	  if ((worker->_blksize - worker->_blkreceived) * best->_avgspeed < (best->_blksize - best->_blkreceived) * worker->_avgspeed)
	    best = worker;
	}
      else
	{
	  if ((worker->_blksize - worker->_blkreceived) * best->_avgspeed > (best->_blksize - best->_blkreceived) * worker->_avgspeed)
	    best = worker;
	}
    }
  if (!best)
    {
      _state = WORKER_DONE;
      _request->_activeworkers--;
      _request->_finished = true;
      return;
    }
  // do not sleep twice
  if (_state != WORKER_SLEEP)
    {
      if (!_avgspeed && _blkreceived)
	{
	  if (!now)
	    now = currentTime();
	  if (now > _blkstarttime)
	    _avgspeed = _blkreceived / (now - _blkstarttime);
	}

      // lets see if we should sleep a bit
      DBG << "me #" << _workerno << ": " << _avgspeed << ", size " << best->_blksize << endl;
      DBG << "best #" << best->_workerno << ": " << best->_avgspeed << ", size " << (best->_blksize - best->_blkreceived) << endl;
      if (_avgspeed && best->_avgspeed && best->_blksize - best->_blkreceived > 0 &&
          (best->_blksize - best->_blkreceived) * _avgspeed < best->_blksize * best->_avgspeed)
	{
	  if (!now)
	    now = currentTime();
	  double sl = (best->_blksize - best->_blkreceived) / best->_avgspeed * 2;
	  if (sl > 1)
	    sl = 1;
	  DBG << "#" << _workerno << ": going to sleep for " << sl * 1000 << " ms" << endl;
	  _sleepuntil = now + sl;
	  _state = WORKER_SLEEP;
	  _request->_sleepworkers++;
	  return;
	}
    }

  _competing = true;
  best->_competing = true;
  _blkstart = best->_blkstart;
  _blksize = best->_blksize;
  best->_pass++;
  _pass = best->_pass;
  _blkno = best->_blkno;
  run();
}

void
multifetchworker::disableCompetition()
{
  std::list<multifetchworker *>::iterator workeriter = _request->_workers.begin();
  for (; workeriter != _request->_workers.end(); ++workeriter)
    {
      multifetchworker *worker = *workeriter;
      if (worker == this)
	continue;
      if (worker->_blkstart == _blkstart)
	{
	  if (worker->_state == WORKER_FETCH)
	    worker->_state = WORKER_DISCARD;
	  worker->_pass = -1;	/* do not steal this one, we already have it */
	}
    }
}


void
multifetchworker::nextjob()
{
  _noendrange = false;
  if (_request->_stealing)
    {
      stealjob();
      return;
    }

  MediaBlockList *blklist = _request->_blklist;
  if (!blklist)
    {
      _blksize = BLKSIZE;
      if (_request->_filesize != off_t(-1))
	{
	  if (_request->_blkoff >= _request->_filesize)
	    {
	      stealjob();
	      return;
	    }
	  _blksize = _request->_filesize - _request->_blkoff;
	  if (_blksize > BLKSIZE)
	    _blksize = BLKSIZE;
	}
    }
  else
    {
      MediaBlock blk = blklist->getBlock(_request->_blkno);
      while (_request->_blkoff >= (off_t)(blk.off + blk.size))
	{
	  if (++_request->_blkno == blklist->numBlocks())
	    {
	      stealjob();
	      return;
	    }
	  blk = blklist->getBlock(_request->_blkno);
	  _request->_blkoff = blk.off;
	}
      _blksize = blk.off + blk.size - _request->_blkoff;
      if (_blksize > BLKSIZE && !blklist->haveChecksum(_request->_blkno))
	_blksize = BLKSIZE;
    }
  _blkno = _request->_blkno;
  _blkstart = _request->_blkoff;
  _request->_blkoff += _blksize;
  run();
}

void
multifetchworker::run()
{
  char rangebuf[128];

  if (_state == WORKER_BROKEN || _state == WORKER_DONE)
     return;	// just in case...
  if (_noendrange)
    sprintf(rangebuf, "%llu-", (unsigned long long)_blkstart);
  else
    sprintf(rangebuf, "%llu-%llu", (unsigned long long)_blkstart, (unsigned long long)_blkstart + _blksize - 1);
  DBG << "#" << _workerno << ": BLK " << _blkno << ":" << rangebuf << " " << _url << endl;
  if (curl_easy_setopt(_curl, CURLOPT_RANGE, !_noendrange || _blkstart != 0 ? rangebuf : (char *)0) != CURLE_OK)
    {
      _request->_activeworkers--;
      _state = WORKER_BROKEN;
      strncpy(_curlError, "curl_easy_setopt range failed", CURL_ERROR_SIZE);
      return;
    }
  if (curl_multi_add_handle(_request->_multi, _curl) != CURLM_OK)
    {
      _request->_activeworkers--;
      _state = WORKER_BROKEN;
      strncpy(_curlError, "curl_multi_add_handle failed", CURL_ERROR_SIZE);
      return;
    }
  _request->_havenewjob = true;
  _off = _blkstart;
  _size = _blksize;
  if (_request->_blklist)
    _request->_blklist->createDigest(_dig);	// resets digest
  _state = WORKER_FETCH;

  double now = currentTime();
  _blkstarttime = now;
  _blkreceived = 0;
}


//////////////////////////////////////////////////////////////////////


multifetchrequest::multifetchrequest(const MediaMultiCurl *context, const Pathname &filename, const Url &baseurl, CURLM *multi, FILE *fp, callback::SendReport<DownloadProgressReport> *report, MediaBlockList *blklist, off_t filesize) : _context(context), _filename(filename), _baseurl(baseurl)
{
  _fp = fp;
  _report = report;
  _blklist = blklist;
  _filesize = filesize;
  _multi = multi;
  _stealing = false;
  _havenewjob = false;
  _blkno = 0;
  if (_blklist)
    _blkoff = _blklist->getBlock(0).off;
  else
    _blkoff = 0;
  _activeworkers = 0;
  _lookupworkers = 0;
  _sleepworkers = 0;
  _minsleepuntil = 0;
  _finished = false;
  _fetchedsize = 0;
  _fetchedgoodsize = 0;
  _totalsize = 0;
  _lastperiodstart = _lastprogress = _starttime = currentTime();
  _lastperiodfetched = 0;
  _periodavg = 0;
  _timeout = 0;
  _connect_timeout = 0;
  _maxspeed = 0;
  _maxworkers = 0;
  if (blklist)
    {
      for (size_t blkno = 0; blkno < blklist->numBlocks(); blkno++)
	{
	  MediaBlock blk = blklist->getBlock(blkno);
	  _totalsize += blk.size;
	}
    }
  else if (filesize != off_t(-1))
    _totalsize = filesize;
}

multifetchrequest::~multifetchrequest()
{
  for (std::list<multifetchworker *>::iterator workeriter = _workers.begin(); workeriter != _workers.end(); ++workeriter)
    {
      multifetchworker *worker = *workeriter;
      *workeriter = NULL;
      delete worker;
    }
  _workers.clear();
}

void
multifetchrequest::run(std::vector<Url> &urllist)
{
  int workerno = 0;
  std::vector<Url>::iterator urliter = urllist.begin();
  for (;;)
    {
      fd_set rset, wset, xset;
      int maxfd, nqueue;

      if (_finished)
	{
	  DBG << "finished!" << endl;
	  break;
	}

      if ((int)_activeworkers < _maxworkers && urliter != urllist.end() && _workers.size() < MAXURLS)
	{
	  // spawn another worker!
	  multifetchworker *worker = new multifetchworker(workerno++, *this, *urliter);
	  _workers.push_back(worker);
	  if (worker->_state != WORKER_BROKEN)
	    {
	      _activeworkers++;
	      if (worker->_state != WORKER_LOOKUP)
		{
		  worker->nextjob();
		}
	      else
	        _lookupworkers++;
	    }
	  ++urliter;
	  continue;
	}
      if (!_activeworkers)
	{
	  WAR << "No more active workers!" << endl;
	  // show the first worker error we find
	  for (std::list<multifetchworker *>::iterator workeriter = _workers.begin(); workeriter != _workers.end(); ++workeriter)
	    {
	      if ((*workeriter)->_state != WORKER_BROKEN)
		continue;
	      ZYPP_THROW(MediaCurlException(_baseurl, "Server error", (*workeriter)->_curlError));
	    }
	  break;
	}

      FD_ZERO(&rset);
      FD_ZERO(&wset);
      FD_ZERO(&xset);

      curl_multi_fdset(_multi, &rset, &wset, &xset, &maxfd);

      if (_lookupworkers)
        for (std::list<multifetchworker *>::iterator workeriter = _workers.begin(); workeriter != _workers.end(); ++workeriter)
	  (*workeriter)->adddnsfd(rset, maxfd);

      timeval tv;
      // if we added a new job we have to call multi_perform once
      // to make it show up in the fd set. do not sleep in this case.
      tv.tv_sec = 0;
      tv.tv_usec = _havenewjob ? 0 : 200000;
      if (_sleepworkers && !_havenewjob)
	{
	  if (_minsleepuntil == 0)
	    {
	      for (std::list<multifetchworker *>::iterator workeriter = _workers.begin(); workeriter != _workers.end(); ++workeriter)
	        {
		  multifetchworker *worker = *workeriter;
		  if (worker->_state != WORKER_SLEEP)
		    continue;
		  if (!_minsleepuntil || _minsleepuntil > worker->_sleepuntil)
		    _minsleepuntil = worker->_sleepuntil;
		}
	    }
	  double sl = _minsleepuntil - currentTime();
	  if (sl < 0)
	    {
	      sl = 0;
	      _minsleepuntil = 0;
	    }
	  if (sl < .2)
	    tv.tv_usec = sl * 1000000;
	}
      int r = select(maxfd + 1, &rset, &wset, &xset, &tv);
      if (r == -1 && errno != EINTR)
	ZYPP_THROW(MediaCurlException(_baseurl, "select() failed", "unknown error"));
      if (r != 0 && _lookupworkers)
	for (std::list<multifetchworker *>::iterator workeriter = _workers.begin(); workeriter != _workers.end(); ++workeriter)
	  {
	    multifetchworker *worker = *workeriter;
	    if (worker->_state != WORKER_LOOKUP)
	      continue;
	    (*workeriter)->dnsevent(rset);
	    if (worker->_state != WORKER_LOOKUP)
	      _lookupworkers--;
	  }
      _havenewjob = false;

      // run curl
      for (;;)
        {
          CURLMcode mcode;
	  int tasks;
          mcode = curl_multi_perform(_multi, &tasks);
          if (mcode == CURLM_CALL_MULTI_PERFORM)
            continue;
	  if (mcode != CURLM_OK)
	    ZYPP_THROW(MediaCurlException(_baseurl, "curl_multi_perform", "unknown error"));
	  break;
        }

      double now = currentTime();

      // update periodavg
      if (now > _lastperiodstart + .5)
	{
	  if (!_periodavg)
	    _periodavg = (_fetchedsize - _lastperiodfetched) / (now - _lastperiodstart);
	  else
	    _periodavg = (_periodavg + (_fetchedsize - _lastperiodfetched) / (now - _lastperiodstart)) / 2;
	  _lastperiodfetched = _fetchedsize;
	  _lastperiodstart = now;
	}

      // wake up sleepers
      if (_sleepworkers)
	{
	  for (std::list<multifetchworker *>::iterator workeriter = _workers.begin(); workeriter != _workers.end(); ++workeriter)
	    {
	      multifetchworker *worker = *workeriter;
	      if (worker->_state != WORKER_SLEEP)
	        continue;
	      if (worker->_sleepuntil > now)
		continue;
	      if (_minsleepuntil == worker->_sleepuntil)
		_minsleepuntil = 0;
	      DBG << "#" << worker->_workerno << ": sleep done, wake up" << endl;
	      _sleepworkers--;
	      // nextjob chnages the state
	      worker->nextjob();
	    }
	}

      // collect all curl results, reschedule new jobs
      CURLMsg *msg;
      while ((msg = curl_multi_info_read(_multi, &nqueue)) != 0)
	{
	  if (msg->msg != CURLMSG_DONE)
	    continue;
	  CURL *easy = msg->easy_handle;
	  CURLcode cc = msg->data.result;
	  multifetchworker *worker;
	  if (curl_easy_getinfo(easy, CURLINFO_PRIVATE, &worker) != CURLE_OK)
	    ZYPP_THROW(MediaCurlException(_baseurl, "curl_easy_getinfo", "unknown error"));
	  if (worker->_blkreceived && now > worker->_blkstarttime)
	    {
	      if (worker->_avgspeed)
		worker->_avgspeed = (worker->_avgspeed + worker->_blkreceived / (now - worker->_blkstarttime)) / 2;
	      else
		worker->_avgspeed = worker->_blkreceived / (now - worker->_blkstarttime);
	    }
	  DBG << "#" << worker->_workerno << ": BLK " << worker->_blkno << " done code " << cc << " speed " << worker->_avgspeed << endl;
	  curl_multi_remove_handle(_multi, easy);
	  if (cc == CURLE_HTTP_RETURNED_ERROR)
	    {
	      long statuscode = 0;
	      (void)curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &statuscode);
	      DBG << "HTTP status " << statuscode << endl;
	      if (statuscode == 416 && !_blklist)	/* Range error */
		{
		  if (_filesize == off_t(-1))
		    {
		      if (!worker->_noendrange)
			{
			  DBG << "#" << worker->_workerno << ": retrying with no end range" << endl;
			  worker->_noendrange = true;
			  worker->run();
			  continue;
			}
		      worker->_noendrange = false;
		      worker->stealjob();
		      continue;
		    }
		  if (worker->_blkstart >= _filesize)
		    {
		      worker->nextjob();
		      continue;
		    }
		}
	    }
	  if (cc == 0)
	    {
	      if (!worker->checkChecksum())
		{
		  WAR << "#" << worker->_workerno << ": checksum error, disable worker" << endl;
		  worker->_state = WORKER_BROKEN;
		  strncpy(worker->_curlError, "checksum error", CURL_ERROR_SIZE);
		  _activeworkers--;
		  continue;
		}
	      if (worker->_state == WORKER_FETCH)
		{
		  if (worker->_competing)
		    {
		      worker->disableCompetition();
		      // multiple workers wrote into this block. We already know that our
		      // data was correct, but maybe some other worker overwrote our data
		      // with something broken. Thus we have to re-check the block.
		      if (!worker->recheckChecksum())
			{
			  DBG << "#" << worker->_workerno << ": recheck checksum error, refetch block" << endl;
			  // re-fetch! No need to worry about the bad workers,
			  // they will now be set to DISCARD. At the end of their block
			  // they will notice that they wrote bad data and go into BROKEN.
			  worker->run();
			  continue;
			}
		    }
		  _fetchedgoodsize += worker->_blksize;
		}

	      // make bad workers sleep a little
	      double maxavg = 0;
	      int maxworkerno = 0;
	      int numbetter = 0;
	      for (std::list<multifetchworker *>::iterator workeriter = _workers.begin(); workeriter != _workers.end(); ++workeriter)
		{
		  multifetchworker *oworker = *workeriter;
		  if (oworker->_state == WORKER_BROKEN)
		    continue;
		  if (oworker->_avgspeed > maxavg)
		    {
		      maxavg = oworker->_avgspeed;
		      maxworkerno = oworker->_workerno;
		    }
		  if (oworker->_avgspeed > worker->_avgspeed)
		    numbetter++;
		}
	      if (maxavg && !_stealing)
		{
		  double ratio = worker->_avgspeed / maxavg;
		  ratio = 1 - ratio;
		  if (numbetter < 3)	// don't sleep that much if we're in the top two
		    ratio = ratio * ratio;
		  if (ratio > .01)
		    {
		      DBG << "#" << worker->_workerno << ": too slow ("<< ratio << ", " << worker->_avgspeed << ", #" << maxworkerno << ": " << maxavg << "), going to sleep for " << ratio * 1000 << " ms" << endl;
		      worker->_sleepuntil = now + ratio;
		      worker->_state = WORKER_SLEEP;
		      _sleepworkers++;
		      continue;
		    }
		}

	      // do rate control (if requested)
	      // should use periodavg, but that's not what libcurl does
	      if (_maxspeed && now > _starttime)
		{
		  double avg = _fetchedsize / (now - _starttime);
		  avg = worker->_maxspeed * _maxspeed / avg;
		  if (avg < _maxspeed / _maxworkers)
		    avg = _maxspeed / _maxworkers;
		  if (avg > _maxspeed)
		    avg = _maxspeed;
		  if (avg < 1024)
		    avg = 1024;
		  worker->_maxspeed = avg;
#if CURLVERSION_AT_LEAST(7,15,5)
		  curl_easy_setopt(worker->_curl, CURLOPT_MAX_RECV_SPEED_LARGE, (curl_off_t)(avg));
#endif
		}

	      worker->nextjob();
	    }
	  else
	    {
	      worker->_state = WORKER_BROKEN;
	      _activeworkers--;
	      if (!_activeworkers && !(urliter != urllist.end() && _workers.size() < MAXURLS))
		{
		  // end of workers reached! goodbye!
		  worker->evaluateCurlCode(Pathname(), cc, false);
		}
	    }
	}

      // send report
      if (_report)
	{
	  int percent = _totalsize ? (100 * (_fetchedgoodsize + _fetchedsize)) / (_totalsize + _fetchedsize) : 0;
	  double avg = 0;
	  if (now > _starttime)
	    avg = _fetchedsize / (now - _starttime);
	  if (!(*(_report))->progress(percent, _baseurl, avg, _lastperiodstart == _starttime ? avg : _periodavg))
	    ZYPP_THROW(MediaCurlException(_baseurl, "User abort", "cancelled"));
	}

      if (_timeout && now - _lastprogress > _timeout)
	break;
    }

  if (!_finished)
    ZYPP_THROW(MediaTimeoutException(_baseurl));

  // print some download stats
  WAR << "overall result" << endl;
  for (std::list<multifetchworker *>::iterator workeriter = _workers.begin(); workeriter != _workers.end(); ++workeriter)
    {
      multifetchworker *worker = *workeriter;
      WAR << "#" << worker->_workerno << ": state: " << worker->_state << " received: " << worker->_received << " url: " << worker->_url << endl;
    }
}


//////////////////////////////////////////////////////////////////////


MediaMultiCurl::MediaMultiCurl(const Url &url_r, const Pathname & attach_point_hint_r)
    : MediaCurl(url_r, attach_point_hint_r)
{
  MIL << "MediaMultiCurl::MediaMultiCurl(" << url_r << ", " << attach_point_hint_r << ")" << endl;
  _multi = 0;
  _customHeadersMetalink = 0;
}

MediaMultiCurl::~MediaMultiCurl()
{
  if (_customHeadersMetalink)
    {
      curl_slist_free_all(_customHeadersMetalink);
      _customHeadersMetalink = 0;
    }
  if (_multi)
    {
      curl_multi_cleanup(_multi);
      _multi = 0;
    }
  std::map<std::string, CURL *>::iterator it;
  for (it = _easypool.begin(); it != _easypool.end(); it++)
    {
      CURL *easy = it->second;
      if (easy)
	{
	  curl_easy_cleanup(easy);
	  it->second = NULL;
	}
    }
}

void MediaMultiCurl::setupEasy()
{
  MediaCurl::setupEasy();

  if (_customHeadersMetalink)
    {
      curl_slist_free_all(_customHeadersMetalink);
      _customHeadersMetalink = 0;
    }
  struct curl_slist *sl = _customHeaders;
  for (; sl; sl = sl->next)
    _customHeadersMetalink = curl_slist_append(_customHeadersMetalink, sl->data);
  _customHeadersMetalink = curl_slist_append(_customHeadersMetalink, "Accept: */*, application/metalink+xml, application/metalink4+xml");
}

static bool looks_like_metalink(const Pathname & file)
{
  char buf[256], *p;
  int fd, l;
  if ((fd = open(file.asString().c_str(), O_RDONLY|O_CLOEXEC)) == -1)
    return false;
  while ((l = read(fd, buf, sizeof(buf) - 1)) == -1 && errno == EINTR)
    ;
  close(fd);
  if (l == -1)
    return 0;
  buf[l] = 0;
  p = buf;
  while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
    p++;
  if (!strncasecmp(p, "<?xml", 5))
    {
      while (*p && *p != '>')
	p++;
      if (*p == '>')
	p++;
      while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
	p++;
    }
  bool ret = !strncasecmp(p, "<metalink", 9) ? true : false;
  DBG << "looks_like_metalink(" << file << "): " << ret << endl;
  return ret;
}

void MediaMultiCurl::doGetFileCopy( const Pathname & filename , const Pathname & target, callback::SendReport<DownloadProgressReport> & report, RequestOptions options ) const
{
  Pathname dest = target.absolutename();
  if( assert_dir( dest.dirname() ) )
  {
    DBG << "assert_dir " << dest.dirname() << " failed" << endl;
    Url url(getFileUrl(filename));
    ZYPP_THROW( MediaSystemException(url, "System error on " + dest.dirname().asString()) );
  }
  string destNew = target.asString() + ".new.zypp.XXXXXX";
  char *buf = ::strdup( destNew.c_str());
  if( !buf)
  {
    ERR << "out of memory for temp file name" << endl;
    Url url(getFileUrl(filename));
    ZYPP_THROW(MediaSystemException(url, "out of memory for temp file name"));
  }

  int tmp_fd = ::mkostemp( buf, O_CLOEXEC );
  if( tmp_fd == -1)
  {
    free( buf);
    ERR << "mkstemp failed for file '" << destNew << "'" << endl;
    ZYPP_THROW(MediaWriteException(destNew));
  }
  destNew = buf;
  free( buf);

  FILE *file = ::fdopen( tmp_fd, "we" );
  if ( !file ) {
    ::close( tmp_fd);
    filesystem::unlink( destNew );
    ERR << "fopen failed for file '" << destNew << "'" << endl;
    ZYPP_THROW(MediaWriteException(destNew));
  }
  DBG << "dest: " << dest << endl;
  DBG << "temp: " << destNew << endl;

  // set IFMODSINCE time condition (no download if not modified)
  if( PathInfo(target).isExist() && !(options & OPTION_NO_IFMODSINCE) )
  {
    curl_easy_setopt(_curl, CURLOPT_TIMECONDITION, CURL_TIMECOND_IFMODSINCE);
    curl_easy_setopt(_curl, CURLOPT_TIMEVALUE, (long)PathInfo(target).mtime());
  }
  else
  {
    curl_easy_setopt(_curl, CURLOPT_TIMECONDITION, CURL_TIMECOND_NONE);
    curl_easy_setopt(_curl, CURLOPT_TIMEVALUE, 0L);
  }
  // change header to include Accept: metalink
  curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, _customHeadersMetalink);
  try
    {
      MediaCurl::doGetFileCopyFile(filename, dest, file, report, options);
    }
  catch (Exception &ex)
    {
      ::fclose(file);
      filesystem::unlink(destNew);
      curl_easy_setopt(_curl, CURLOPT_TIMECONDITION, CURL_TIMECOND_NONE);
      curl_easy_setopt(_curl, CURLOPT_TIMEVALUE, 0L);
      curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, _customHeaders);
      ZYPP_RETHROW(ex);
    }
  curl_easy_setopt(_curl, CURLOPT_TIMECONDITION, CURL_TIMECOND_NONE);
  curl_easy_setopt(_curl, CURLOPT_TIMEVALUE, 0L);
  curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, _customHeaders);
  long httpReturnCode = 0;
  CURLcode infoRet = curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &httpReturnCode);
  if (infoRet == CURLE_OK)
  {
    DBG << "HTTP response: " + str::numstring(httpReturnCode) << endl;
    if ( httpReturnCode == 304
	 || ( httpReturnCode == 213 && _url.getScheme() == "ftp" ) ) // not modified
    {
      DBG << "not modified: " << PathInfo(dest) << endl;
      return;
    }
  }
  else
  {
    WAR << "Could not get the reponse code." << endl;
  }

  bool ismetalink = false;

  char *ptr = NULL;
  if (curl_easy_getinfo(_curl, CURLINFO_CONTENT_TYPE, &ptr) == CURLE_OK && ptr)
    {
      string ct = string(ptr);
      if (ct.find("application/metalink+xml") == 0 || ct.find("application/metalink4+xml") == 0)
	ismetalink = true;
    }

  if (!ismetalink)
    {
      // some proxies do not store the content type, so also look at the file to find
      // out if we received a metalink (bnc#649925)
      fflush(file);
      if (looks_like_metalink(Pathname(destNew)))
	ismetalink = true;
    }

  if (ismetalink)
    {
      bool userabort = false;
      fclose(file);
      file = NULL;
      Pathname failedFile = ZConfig::instance().repoCachePath() / "MultiCurl.failed";
      try
	{
	  MetaLinkParser mlp;
	  mlp.parse(Pathname(destNew));
	  MediaBlockList bl = mlp.getBlockList();
	  vector<Url> urls = mlp.getUrls();
	  DBG << bl << endl;
	  file = fopen(destNew.c_str(), "w+e");
	  if (!file)
	    ZYPP_THROW(MediaWriteException(destNew));
	  if (PathInfo(target).isExist())
	    {
	      DBG << "reusing blocks from file " << target << endl;
	      bl.reuseBlocks(file, target.asString());
	      DBG << bl << endl;
	    }
	  if (bl.haveChecksum(1) && PathInfo(failedFile).isExist())
	    {
	      DBG << "reusing blocks from file " << failedFile << endl;
	      bl.reuseBlocks(file, failedFile.asString());
	      DBG << bl << endl;
	      filesystem::unlink(failedFile);
	    }
	  Pathname df = deltafile();
	  if (!df.empty())
	    {
	      DBG << "reusing blocks from file " << df << endl;
	      bl.reuseBlocks(file, df.asString());
	      DBG << bl << endl;
	    }
	  try
	    {
	      multifetch(filename, file, &urls, &report, &bl);
	    }
	  catch (AbortRequestException &ex)
	    {
	      userabort = true;
	      ZYPP_RETHROW(ex);
	    }
	}
      catch (Exception &ex)
	{
	  // something went wrong. fall back to normal download
	  if (file)
	    fclose(file);
	  file = NULL;
	  if (PathInfo(destNew).size() >= 63336)
	    {
	      ::unlink(failedFile.asString().c_str());
	      filesystem::hardlinkCopy(destNew, failedFile);
	    }
	  if (userabort)
	    {
	      filesystem::unlink(destNew);
	      ZYPP_RETHROW(ex);
	    }
	  file = fopen(destNew.c_str(), "w+e");
	  if (!file)
	    ZYPP_THROW(MediaWriteException(destNew));
	  MediaCurl::doGetFileCopyFile(filename, dest, file, report, options | OPTION_NO_REPORT_START);
	}
    }

  if (::fchmod( ::fileno(file), filesystem::applyUmaskTo( 0644 )))
    {
      ERR << "Failed to chmod file " << destNew << endl;
    }
  if (::fclose(file))
    {
      filesystem::unlink(destNew);
      ERR << "Fclose failed for file '" << destNew << "'" << endl;
      ZYPP_THROW(MediaWriteException(destNew));
    }
  if ( rename( destNew, dest ) != 0 )
    {
      ERR << "Rename failed" << endl;
      ZYPP_THROW(MediaWriteException(dest));
    }
  DBG << "done: " << PathInfo(dest) << endl;
}

void MediaMultiCurl::multifetch(const Pathname & filename, FILE *fp, std::vector<Url> *urllist, callback::SendReport<DownloadProgressReport> *report, MediaBlockList *blklist, off_t filesize) const
{
  Url baseurl(getFileUrl(filename));
  if (blklist && filesize == off_t(-1) && blklist->haveFilesize())
    filesize = blklist->getFilesize();
  if (blklist && !blklist->haveBlocks() && filesize != 0)
    blklist = 0;
  if (blklist && (filesize == 0 || !blklist->numBlocks()))
    {
      checkFileDigest(baseurl, fp, blklist);
      return;
    }
  if (filesize == 0)
    return;
  if (!_multi)
    {
      _multi = curl_multi_init();
      if (!_multi)
	ZYPP_THROW(MediaCurlInitException(baseurl));
    }
  multifetchrequest req(this, filename, baseurl, _multi, fp, report, blklist, filesize);
  req._timeout = _settings.timeout();
  req._connect_timeout = _settings.connectTimeout();
  req._maxspeed = _settings.maxDownloadSpeed();
  req._maxworkers = _settings.maxConcurrentConnections();
  if (req._maxworkers > MAXURLS)
    req._maxworkers = MAXURLS;
  if (req._maxworkers <= 0)
    req._maxworkers = 1;
  std::vector<Url> myurllist;
  for (std::vector<Url>::iterator urliter = urllist->begin(); urliter != urllist->end(); ++urliter)
    {
      try
	{
	  string scheme = urliter->getScheme();
	  if (scheme == "http" || scheme == "https" || scheme == "ftp")
	    {
	      checkProtocol(*urliter);
	      myurllist.push_back(*urliter);
	    }
	}
      catch (...)
	{
	}
    }
  if (!myurllist.size())
    myurllist.push_back(baseurl);
  req.run(myurllist);
  checkFileDigest(baseurl, fp, blklist);
}

void MediaMultiCurl::checkFileDigest(Url &url, FILE *fp, MediaBlockList *blklist) const
{
  if (!blklist || !blklist->haveFileChecksum())
    return;
  if (fseeko(fp, off_t(0), SEEK_SET))
    ZYPP_THROW(MediaCurlException(url, "fseeko", "seek error"));
  Digest dig;
  blklist->createFileDigest(dig);
  char buf[4096];
  size_t l;
  while ((l = fread(buf, 1, sizeof(buf), fp)) > 0)
    dig.update(buf, l);
  if (!blklist->verifyFileDigest(dig))
    ZYPP_THROW(MediaCurlException(url, "file verification failed", "checksum error"));
}

bool MediaMultiCurl::isDNSok(const string &host) const
{
  return _dnsok.find(host) == _dnsok.end() ? false : true;
}

void MediaMultiCurl::setDNSok(const string &host) const
{
  _dnsok.insert(host);
}

CURL *MediaMultiCurl::fromEasyPool(const string &host) const
{
  if (_easypool.find(host) == _easypool.end())
    return 0;
  CURL *ret = _easypool[host];
  _easypool.erase(host);
  return ret;
}

void MediaMultiCurl::toEasyPool(const std::string &host, CURL *easy) const
{
  CURL *oldeasy = _easypool[host];
  _easypool[host] = easy;
  if (oldeasy)
    curl_easy_cleanup(oldeasy);
}

  } // namespace media
} // namespace zypp

