/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp-curl/parser/MetaLinkParser
 *
*/
#ifndef ZYPP_CURL_PARSER_METALINKPARSER_H_INCLUDED
#define ZYPP_CURL_PARSER_METALINKPARSER_H_INCLUDED

#include <string>

#include <zypp-core/base/Exception.h>
#include <zypp-core/base/NonCopyable.h>
#include <zypp-core/base/InputStream>
#include <zypp-curl/parser/MediaBlockList>
#include <zypp-core/Url.h>
#include <zypp-core/ByteArray.h>

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

  const std::vector<UByteArray> &getZsyncBlockHashes() const;
  const std::vector<UByteArray> &getSHA1BlockHashes() const;

private:
  struct ml_parsedata *pd;
};

UByteArray hexstr2bytes( std::string str );

} // namespace zypp::media

#endif // ZYPP_CURL_PARSER_METALINKPARSER_H_INCLUDED
