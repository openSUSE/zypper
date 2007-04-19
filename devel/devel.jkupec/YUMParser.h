#ifndef YUMPARSER_H_
#define YUMPARSER_H_

#include "zypp/base/Logger.h"
#include "zypp2/cache/CacheStore.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/parser/yum/PrimaryFileReader.h"

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

    void start(const zypp::Pathname &path, Progress progress_fnc);

    bool primary_CB(const zypp::data::Package &package);
    bool test() { return true; }

  private:
    zypp::cache::CacheStore &_consumer;
    zypp::data::RecordId _catalog_id;
  };


    }
  }
} // ns zypp

#endif /*YUMPARSER_H_*/
