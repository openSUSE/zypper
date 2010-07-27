/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/ZsyncParser.h
 *
*/
#ifndef ZYPP_MEDIA_ZSYNCPARSER_H
#define ZYPP_MEDIA_ZSYNCPARSER_H

#include <string>

#include "zypp/base/Exception.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/media/MediaBlockList.h"
#include "zypp/Url.h"

namespace zypp {
  namespace media {

class ZsyncParser : private zypp::base::NonCopyable {
public:
  ZsyncParser();

  /**
   * parse a file consisting of zlink data
   * \throws Exception
   **/
  void parse(std::string filename);
  /**
   * return the download urls from the parsed metalink data
   **/
  std::vector<Url> getUrls();
  /**
   * return the block list from the parsed metalink data
   **/
  MediaBlockList getBlockList();

private:
  off_t filesize;
  size_t blksize;
  int sql;
  int rsl;
  int csl;
  MediaBlockList bl;
  std::vector<std::string> urls;
};

  } // namespace media
} // namespace zypp

#endif // ZYPP_MEDIA_ZSYNCPARSER_H
