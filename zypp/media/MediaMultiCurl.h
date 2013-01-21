/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MediaMultiCurl.h
 *
*/
#ifndef ZYPP_MEDIA_MEDIAMULTICURL_H
#define ZYPP_MEDIA_MEDIAMULTICURL_H

#include <string>
#include <vector>
#include <list>
#include <set>

#include "zypp/media/MediaHandler.h"
#include "zypp/media/MediaCurl.h"
#include "zypp/media/MediaBlockList.h"
#include "zypp/media/TransferSettings.h"
#include "zypp/ZYppCallbacks.h"

namespace zypp {
  namespace media {

/**
 * @short Implementation class for FTP, HTTP and HTTPS MediaHandler
 *
 * @author Michael Schroeder <mls@suse.de>
 *
 * @see MediaHandler
 **/

class multifetchrequest;
class multifetchworker;

class MediaMultiCurl : public MediaCurl {
public:
  friend class multifetchrequest;
  friend class multifetchworker;

  MediaMultiCurl(const Url &url_r, const Pathname & attach_point_hint_r);
  ~MediaMultiCurl();

  virtual void doGetFileCopy( const Pathname & srcFilename, const Pathname & targetFilename, callback::SendReport<DownloadProgressReport> & _report, RequestOptions options = OPTION_NONE ) const;

  void multifetch(const Pathname &filename, FILE *fp, std::vector<Url> *urllist, callback::SendReport<DownloadProgressReport> *report = 0, MediaBlockList *blklist = 0, off_t filesize = off_t(-1)) const;

protected:

  bool isDNSok(const std::string &host) const;
  void setDNSok(const std::string &host) const;

  CURL *fromEasyPool(const std::string &host) const;
  void toEasyPool(const std::string &host, CURL *easy) const;

  virtual void setupEasy();
  void checkFileDigest(Url &url, FILE *fp, MediaBlockList *blklist) const;
  static int progressCallback( void *clientp, double dltotal, double dlnow, double ultotal, double ulnow );

private:
  // the custom headers from MediaCurl plus a "Accept: metalink" header
  curl_slist *_customHeadersMetalink;
  mutable CURLM *_multi;	// reused for all fetches so we can make use of the dns cache
  mutable std::set<std::string> _dnsok;
  mutable std::map<std::string, CURL *> _easypool;
};

///////////////////////////////////////////////////////////////////

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_MEDIAMULTICURL_H
