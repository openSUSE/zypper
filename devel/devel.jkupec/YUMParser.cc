//#include "zypp/data/ResolvableDataConsumer.h"
#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"
#include "zypp/base/Logger.h"

#include "YUMParser.h"

using std::endl;

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  YUMParser::YUMParser(const zypp::data::RecordId &catalog_id, zypp::cache::CacheStore &consumer)
    : _consumer(consumer), _catalog_id(catalog_id)
  {
    ZYpp::Ptr z = getZYpp();
//    _system_arch = z->architecture();

    MIL << "constructed" << endl;
  }
  
  
  
  bool YUMParser::primary_CB(const zypp::data::Package &package)
  {
//    data::RecordId pkgid = _consumer.appendResolvable( _catalog_id, ResTraits<Package>::kind, nvra, deps );

    MIL << "got package "
      << package.name << package.edition << " "
      << package.arch
      << endl;
/*    MIL << "checksum: " << package.checksum << endl;
    MIL << "summary: " << package.summary << endl;*/
  }

  void YUMParser::start(const Pathname &cache_dir, Progress progress_fnc)
  {
    progress_fnc(0);

    zypp::parser::yum::PrimaryFileReader(
        cache_dir + "/repodata/primary.xml.gz",
        bind(&YUMParser::primary_CB, this, _1));

    progress_fnc(100);
  }


    } // ns yum
  } // ns parser
} // ns zypp
