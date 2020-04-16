/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/media/MetaLinkParser.h
 *
*/
#ifndef ZYPP_MEDIA_METALINKPARSER_H
#define ZYPP_MEDIA_METALINKPARSER_H

#include <string>

#include <zypp/base/Exception.h>
#include <zypp/base/NonCopyable.h>
#include <zypp/base/InputStream.h>
#include <zypp/media/MediaBlockList.h>
#include <zypp/Url.h>
#include <zypp/ByteArray.h>

namespace zypp::media {

struct ml_parsedata;

struct MetalinkMirror {
  int priority = 0;
  int maxConnections = -1; //< How many connections can be opened to that mirror, -1 means no limit was defined.
  Url url;
};

class MetaLinkParser : private zypp::base::NonCopyable {
public:
  MetaLinkParser();
  ~MetaLinkParser();

  /**
   * parse a file consisting of metalink xml data
   * \throws Exception
   **/
  void parse(const Pathname &filename);

  /**
   * parse an InputStream consisting of metalink xml data
   * \throws Exception
   **/
  void parse(const InputStream &is);

  /**
   * parse a chunk of a file consisting of metalink xml data.
   * \throws Exception
   **/
  void parseBytes(const char* bytes, size_t len);
  /**
   * tells the parser that all chunks are now processed
   * \throws Exception
   **/
  void parseEnd();

  /**
   * return the download urls from the parsed metalink data
   **/
  std::vector<Url> getUrls() const;

  /**
   * return the mirrors from the parsed metalink data
   */
  const std::vector<MetalinkMirror> &getMirrors() const;

  /**
   * return the block list from the parsed metalink data
   **/
  MediaBlockList getBlockList() const;

  const std::vector<ByteArray> &getZsyncBlockHashes () const;
  const std::vector<ByteArray> &getSHA1BlockHashes () const;

private:
  struct ml_parsedata *pd;
};

ByteArray hexstr2bytes( std::string str );

} // namespace zypp::media

#endif // ZYPP_MEDIA_METALINKPARSER_H
