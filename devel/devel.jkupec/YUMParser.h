#ifndef YUMPARSER_H_
#define YUMPARSER_H_

#include "zypp/base/Logger.h"
#include "zypp2/cache/CacheStore.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/parser/yum/PrimaryFileReader.h"
#include "zypp/parser/ParserProgress.h"

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "parser"


namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  class YUMParser
  {
  public:
    typedef function<bool( int )> Progress;

    YUMParser(const zypp::data::RecordId &catalog_id, zypp::cache::CacheStore &consumer);

    void start(const zypp::Pathname &path, ParserProgress::Ptr progress);

    bool primary_CB(const zypp::data::Package &package, const zypp::data::Dependencies &deps);
    bool test() { return true; }

  private:
    zypp::cache::CacheStore &_consumer;
    zypp::data::RecordId _catalog_id;
  };


    } // ns yum
  } // ns parser
} // ns zypp

#endif /*YUMPARSER_H_*/
