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


  bool YUMParser::primary_CB(const zypp::data::Package &package, const zypp::data::Dependencies &deps)
  {
    NVRA nvra(package.name, package.edition, package.arch);
    data::RecordId pkgid =
      _consumer.appendResolvable(
        _catalog_id, ResTraits<Package>::kind, nvra, deps);

/*    MIL << "got package "
      << package.name << package.edition << " "
      << package.arch
      << endl;
    MIL << "checksum: " << package.checksum << endl;
    MIL << "summary: " << package.summary << endl;*/
  }

  void YUMParser::start(const Pathname &cache_dir, ParserProgress::Ptr progress)
  {
    zypp::parser::yum::PrimaryFileReader(
        cache_dir + "/repodata/primary.xml.gz",
        bind(&YUMParser::primary_CB, this, _1, _2),
        progress);
  }


    } // ns yum
  } // ns parser
} // ns zypp
