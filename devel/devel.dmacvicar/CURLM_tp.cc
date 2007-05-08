#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include <curl/curl.h>
}
#include <list>
#include <sstream>
#include "zypp/base/Exception.h"
#include "zypp/base/Logger.h"
#include "zypp/Pathname.h"
#include "zypp/ExternalProgram.cc"
//#include 

using namespace zypp;
using namespace std;

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
  MIL << "got data : " << size*nmemb << " bytes" << endl;
  return size*nmemb;
}

int socket_callback(CURL *easy, curl_socket_t s, int what, void *userp,  void *socketp)
{
  MIL << "socket : " << s << " : " << what << endl;
  return 0;
}

struct Range
{
  Range( off_t f, off_t t)
    : from(t), to(t)
  {}

  off_t from;
  off_t to;
};

int main()
{
   FILE *f = fopen("piece", "w" );
  curl_global_init(CURL_GLOBAL_ALL);
  CURLM *curlm;
  curlm = curl_multi_init();
  //curl_multi_setopt( curlm, CURLMOPT_PIPELINING, 1);
  curl_multi_setopt( curlm, CURLMOPT_SOCKETFUNCTION, socket_callback);
  
//   0-1000
//   1001-2000
//   2001-3000
//   
//   3000-4000
//   4001-5000

  int i=1;
  for ( ; i < 10; i++ ) {
    CURL *curl;
    curl = curl_easy_init();
    CURLcode success;
    // http://download.opensuse.org/distribution/10.2/repo/oss/suse/setup/descr/packages
    if ( (success = curl_easy_setopt(curl, CURLOPT_URL, "http://ftp5.gwdg.de/pub/opensuse/distribution/10.2/repo/oss/suse/setup/descr/packages")) != CURLE_OK)
      ZYPP_THROW(Exception("url"));

     if ( (success = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data)) != CURLE_OK)
       ZYPP_THROW(Exception("write data"));
    //curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
    stringstream rs;
    int k=1;
    for(; k<2; k++) {
      rs << ( k!=1 ? "," : "") << (i*k)*1000 << "-" << ((i*k)*1000 + 1000);
    }
    MIL << "range: " << rs.str() << endl;
    if ( (success = curl_easy_setopt(curl, CURLOPT_RANGE, rs.str().c_str())) != CURLE_OK)
      ZYPP_THROW(Exception("write data"));

    CURLMcode code;
    if ( (code = curl_multi_add_handle( curlm, curl)) != CURLM_OK)
      ZYPP_THROW(Exception("write data"));
    
  }
  int still_running = 0;
  /* we start some action by calling perform right away */
  while(CURLM_CALL_MULTI_PERFORM ==
        curl_multi_perform(curlm, &still_running));

  while(still_running) {
    struct timeval timeout;
    int rc; /* select() return code */

    fd_set fdread;
    fd_set fdwrite;
    fd_set fdexcep;
    int maxfd;

    FD_ZERO(&fdread);
    FD_ZERO(&fdwrite);
    FD_ZERO(&fdexcep);

    /* set a suitable timeout to play around with */
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    /* get file descriptors from the transfers */
    curl_multi_fdset(curlm, &fdread, &fdwrite, &fdexcep, &maxfd);

    /* In a real-world program you OF COURSE check the return code of the
       function calls, *and* you make sure that maxfd is bigger than -1 so
       that the call to select() below makes sense! */

    rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);

    switch(rc) {
    case -1:
      /* select error */
      still_running = 0;
      ERR << "select() returns error, this is badness" << endl;
      break;
    case 0:
    default:
      /* timeout or readable/writable sockets */
      while(CURLM_CALL_MULTI_PERFORM ==
            curl_multi_perform(curlm, &still_running));
      break;
    }
  }

  int c=999;
  CURLMsg *m;
  while ( m = curl_multi_info_read( curlm, &c) )
  {
    MIL<< m->msg << " : " << curl_easy_strerror(m->data.result) << endl;
  }
  Pathname root("/home/duncan/suse/metadata-diff");
  

 curl_multi_cleanup(curlm);
 //curl_easy_cleanup(http_handle);

  return 0;
}