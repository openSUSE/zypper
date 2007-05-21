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
#include "zypp/parser/yum/OtherFileReader.h"
#include "zypp/parser/yum/FilelistsFileReader.h"

#include "YUMParser.h"


#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "parser"

namespace zypp
{
  namespace parser
  {
    namespace yum
    {

  // TODO make this through ZYppCallbacks.h 
  bool progress_function(ProgressData::value_type p)
  {
    cout << "Parsing $name_would_come_in_handy [" << p << "%]" << endl;
//    cout << "\rParsing $name_would_come_in_handy [" << p << "%]" << flush;
    return true;
  }


  YUMParser::YUMParser(
      const data::RecordId & catalog_id,
      data::ResolvableDataConsumer & consumer,
      const ProgressData::ReceiverFnc & progress)
    :
      _consumer(consumer), _catalog_id(catalog_id)
  {
    _ticks.name("YUMParser");
    _ticks.sendTo(progress);
  }


  bool YUMParser::repomd_CB( const OnMediaLocation &loc, const YUMResourceType &dtype )
  {
    DBG << "Adding " << dtype
        << " (" << loc.filename() << ") to YUMParser jobs " << endl;

    _jobs.push_back(YUMParserJob(loc.filename(), dtype));

    return true;
  }


  bool YUMParser::primary_CB(const data::Package_Ptr & package_r)
  {
    _consumer.consumePackage( _catalog_id, package_r );

/*    MIL << "got package "
      << package.name << package.edition << " "
      << package.arch
      << endl;
    MIL << "checksum: " << package.checksum << endl;
    MIL << "summary: " << package.summary << endl;*/

    return true;
  }


  bool YUMParser::patches_CB(const OnMediaLocation &loc, const string & patch_id)
  {
    DBG << "Adding patch " << loc.filename() << " to YUMParser jobs " << endl;
    
    _jobs.push_back(YUMParserJob(loc.filename(), YUMResourceType::PATCH));

    return true;
  }


  bool YUMParser::patch_CB(const data::Patch_Ptr & patch)
  {
    _consumer.consumePatch( _catalog_id, patch );

    MIL << "got patch "
      << patch->name << patch->edition << " "
      << patch->arch
      << endl;

    return true;
  }


  bool YUMParser::other_CB(const data::Resolvable_Ptr & res_ptr, const Changelog & changelog)
  {
    _consumer.consumeChangelog(_catalog_id, res_ptr, changelog);
/*
    DBG << "got changelog for "
      << res_ptr->name << res_ptr->edition << " "
      << res_ptr->arch
      << endl;

    DBG << "last entry: " << changelog.front() << endl;
*/
    return true;
  }


  bool YUMParser::filelist_CB(const data::Resolvable_Ptr & res_ptr, const data::Filenames & filenames)
  {
    _consumer.consumeFilelist(_catalog_id, res_ptr, filenames);
/*
    DBG << "got filelist for "
      << res_ptr->name << res_ptr->edition << " "
      << res_ptr->arch
      << endl;

    DBG << "last entry: " << filenames.front() << endl;
*/
    return true;
  }


  void YUMParser::start(const Pathname &cache_dir)
  {
    zypp::parser::yum::RepomdFileReader(
        cache_dir + "/repodata/repomd.xml",
        bind(&YUMParser::repomd_CB, this, _1, _2));


    _ticks.range(_jobs.size());
    _ticks.toMin();

    doJobs(cache_dir);

    _ticks.toMax();
  }


  void YUMParser::doJobs(const Pathname &cache_dir)
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
            &progress_function);
          break;
        }

        case YUMResourceType::PATCHES_e:
        {
          zypp::source::yum::PatchesFileReader(
            cache_dir + job.filename(),
            bind(&YUMParser::patches_CB, this, _1, _2));
          // reset progress reporter max value (number of jobs changed if
          // there are patches to parse)
          _ticks.range(_jobs.size());
          break;
        }

        case YUMResourceType::PATCH_e:
        {
          zypp::parser::yum::PatchFileReader(
            cache_dir + job.filename(),
            bind(&YUMParser::patch_CB, this, _1));
          break;
        }

        case YUMResourceType::OTHER_e:
        {
          zypp::parser::yum::OtherFileReader(
            cache_dir + job.filename(),
            bind(&YUMParser::other_CB, this, _1, _2),
            &progress_function);
          break;
        }

        case YUMResourceType::FILELISTS_e:
        {
          zypp::parser::yum::FilelistsFileReader(
            cache_dir + job.filename(),
            bind(&YUMParser::filelist_CB, this, _1, _2),
            &progress_function);
          break;
        }

        default:
        {
          WAR << "Don't know how to read "
              << job.type() << " file "
              << job.filename() << endl;
        }
      }

      _ticks.incr();
    }
  }


    } // ns yum
  } // ns parser
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:

