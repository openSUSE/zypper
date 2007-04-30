/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef YUMPARSER_H_
#define YUMPARSER_H_

#include "zypp/base/Logger.h"
#include "zypp2/cache/CacheStore.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/parser/ParserProgress.h"
#include "zypp/source/yum/YUMResourceType.h"


#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "parser"

using zypp::source::yum::YUMResourceType;

namespace zypp
{
  namespace parser
  {
    namespace yum
    {

  /**
   * 
   */
  struct YUMParserJob
  {
    YUMParserJob(const Pathname & filename, const YUMResourceType & type)
      : _filename(filename), _type(type) {}

    const Pathname & filename() const { return _filename; } 
    const YUMResourceType & type() const { return _type; } 

  private:
    Pathname _filename;
    YUMResourceType _type;
  };


  /**
   * 
   */
  class YUMParser
  {
  public:
    typedef function<bool( int )> Progress;

    YUMParser(const zypp::data::RecordId & catalog_id, zypp::cache::CacheStore & consumer);

    void start(const zypp::Pathname & path, ParserProgress::Ptr progress);
    void doJobs(const zypp::Pathname & path, ParserProgress::Ptr progress);

    bool repomd_CB(const OnMediaLocation & loc, const YUMResourceType & dtype);
    bool primary_CB(const zypp::data::Package & package);
    bool patches_CB(const OnMediaLocation &loc, const std::string & patch_id);
    bool patch_CB(const zypp::data::Patch & patch);

  private:
    zypp::cache::CacheStore & _consumer;
    zypp::data::RecordId _catalog_id;
    std::list<YUMParserJob> _jobs;
  };


    } // ns yum
  } // ns parser
} // ns zypp

#endif /*YUMPARSER_H_*/

// vim: set ts=2 sts=2 sw=2 et ai:
