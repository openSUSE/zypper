/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "zypp/ZYpp.h"
#include "zypp/ZYppFactory.h"
#include "zypp/base/Logger.h"

#include "zypp/parser/yum/RepomdFileReader.h"
#include "zypp/parser/yum/PrimaryFileReader.h"
#include "zypp/parser/yum/PatchesFileReader.h"
#include "zypp/parser/yum/PatchFileReader.h"

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


  bool YUMParser::repomd_CB( const OnMediaLocation &loc, const YUMResourceType &dtype )
  {
    DBG << "Adding " << dtype
        << " (" << loc.filename() << ") to YUMParser jobs " << endl;

    _jobs.push_back(YUMParserJob(loc.filename(), dtype));

    return true;
  }

  bool YUMParser::primary_CB(const zypp::data::Package & package)
  {
    NVRA nvra(package.name, package.edition, package.arch);
    data::RecordId pkgid =
      _consumer.appendResolvable(
        _catalog_id, ResTraits<Package>::kind, nvra, package.deps);

/*    MIL << "got package "
      << package.name << package.edition << " "
      << package.arch
      << endl;
    MIL << "checksum: " << package.checksum << endl;
    MIL << "summary: " << package.summary << endl;*/
  }


  bool YUMParser::patches_CB(const OnMediaLocation &loc, const string & patch_id)
  {
    DBG << "Adding patch " << loc.filename() << " to YUMParser jobs " << endl;

    _jobs.push_back(YUMParserJob(loc.filename(), YUMResourceType::PATCH));

    return true;
  }


  bool YUMParser::patch_CB(const zypp::data::Patch & patch)
  {
    NVRA nvra(patch.name, patch.edition, patch.arch);
    data::RecordId pkgid =
      _consumer.appendResolvable(
        _catalog_id, ResTraits<Patch>::kind, nvra, patch.deps);

    MIL << "got patch "
      << patch.name << patch.edition << " "
      << patch.arch
      << endl;
  }


  void YUMParser::start(const Pathname &cache_dir, ParserProgress::Ptr progress)
  {
    zypp::parser::yum::RepomdFileReader(
        cache_dir + "/repodata/repomd.xml",
        bind(&YUMParser::repomd_CB, this, _1, _2));

    doJobs(cache_dir, progress);
  }


  void YUMParser::doJobs(const Pathname &cache_dir, ParserProgress::Ptr progress)
  {
    for(list<YUMParserJob>::const_iterator it = _jobs.begin(); it != _jobs.end(); ++it)
    {
      YUMParserJob job = *it;

      MIL << "going to parse " << job.type() << " file " << job.filename() << endl;

      switch(job.type().toEnum())
      {
        // parse primary.xml.gz
        case YUMResourceType::PRIMARY_e:
        {
          zypp::parser::yum::PrimaryFileReader(
            cache_dir + job.filename(),
            bind(&YUMParser::primary_CB, this, _1),
            progress);
          break;
        }
        case YUMResourceType::PATCHES_e:
        {
          zypp::source::yum::PatchesFileReader(
            cache_dir + job.filename(),
            bind(&YUMParser::patches_CB, this, _1, _2));
          break;
        }
        case YUMResourceType::PATCH_e:
        {
          zypp::parser::yum::PatchFileReader(
            cache_dir + job.filename(),
            bind(&YUMParser::patch_CB, this, _1));
          break;
        }
        default:
        {
          DBG << "oh yeah, time will come, when we will parse "
              << job.type() << " file "
              << job.filename() << endl;
        }
      }
    }
  }


    } // ns yum
  } // ns parser
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
