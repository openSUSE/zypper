/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/RepoParser.cc
 * YUM repository metadata parser implementation.
 */
#include <iostream>

#include "zypp/ZConfig.h"
#include "zypp/base/Logger.h"
#include "zypp/PathInfo.h"
#include "zypp/base/UserRequestException.h"
#include "zypp/repo/yum/ResourceType.h"
#include "zypp/data/ResolvableData.h"

#include "zypp/parser/yum/RepomdFileReader.h"
#include "zypp/parser/yum/PrimaryFileReader.h"
#include "zypp/parser/yum/PatchesFileReader.h"
#include "zypp/parser/yum/PatchFileReader.h"
#include "zypp/parser/yum/PatternFileReader.h"
#include "zypp/parser/yum/ProductFileReader.h"
#include "zypp/parser/yum/OtherFileReader.h"
#include "zypp/parser/yum/FilelistsFileReader.h"

#include "zypp/parser/yum/RepoParser.h"


#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "parser::yum"

using namespace std;
using zypp::repo::yum::ResourceType;

namespace zypp
{
  namespace parser
  {
    namespace yum
    {

  /**
   * Structure encapsulating YUM parser data type and filename.
   */
  struct RepoParserJob
  {
    RepoParserJob(const Pathname & filename, const ResourceType & type)
      : _filename(filename), _type(type) {}

    const Pathname & filename() const { return _filename; }
    const ResourceType & type() const { return _type; }

  private:
    /** File to be processed */
    Pathname _filename;
    /** Type of YUM file */
    ResourceType _type;
  };


  ///////////////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : RepoParser::Impl
  //
  class RepoParser::Impl : private base::NonCopyable
  {
  public:
    /** CTOR */
    Impl(
      const std::string &repo_id,
      data::ResolvableDataConsumer & consumer,
      const RepoParserOpts & options = RepoParserOpts(),
      const ProgressData::ReceiverFnc & progress = ProgressData::ReceiverFnc()
    );

    /** Implementation of \ref RepoParser::parse(Pathname) */
    void parse(const Pathname &cache_dir);

    /**
     * Iterates through parser \ref _jobs and executes them using
     * *FileReader classes.
     *
     * \param path location of the raw repository cache
     */
    void doJobs(const Pathname & path);

    /**
     * Callback for processing data returned from \ref RepomdFileReader.
     * Adds returned files to parser job list (\ref _jobs).
     *
     * \param loc location of discovered data file
     * \param dtype YUM data type
     */
    bool repomd_CB(const OnMediaLocation & loc, const ResourceType & dtype);

    /**
     * Callback for processing packages returned from \ref PrimaryFileReader.
     * Uses \ref _consumer to process read package data.
     *
     * \param package_r pointer to package data
     */
    bool primary_CB(const data::Packagebase_Ptr & package_r);

    /**
     * Callback for processing data returned from \ref PatchesFileReader.
     * Adds discovered patch*.xml files to parser \ref _jobs.
     *
     * \param loc location of discovered patch file
     * \param patch_id (not used so far)
     */
    bool patches_CB(const OnMediaLocation &loc, const std::string & patch_id);

    /**
     * Callback for processing data returned from \ref PatchFileReader.
     * Uses \ref _consumer to process read patch data.
     *
     * \param patch pointer to patch data
     */
    bool patch_CB(const data::Patch_Ptr & patch);

    /**
     * Callback for processing data returned from \ref OtherFileReader.
     * Uses \ref _consumer to process read changelog data.
     *
     * \param res_ptr resolvable to which the changelog belongs
     * \param changelog read changelog
     */
    bool other_CB(const data::Resolvable_Ptr & res_ptr, const Changelog & changelog);

    /**
     * Callback for processing data returned from \ref FilelistsFileReader.
     * Uses \ref _consumer to process read filelist.
     *
     * \param res_ptr resolvable to which the filelist belongs.
     * \param filenames the read filelist
     */
    bool filelist_CB(const data::Resolvable_Ptr & res_ptr, const data::Filenames & filenames);

    /**
     * Callback for processing data returned from \ref PatternFileReader.
     * Uses \ref _consumer to process read pattern.
     *
     * \param pattern_ptr pointer to pattern data object
     */
    bool pattern_CB(const data::Pattern_Ptr & pattern_ptr);

    /**
     * Callback for processing data returned from \ref ProductFileReader.
     * Uses \ref _consumer to process read product.
     *
     * \param product_ptr pointer to product data object
     */
    bool product_CB(const data::Product_Ptr & product_ptr);

  private:
    /** ID of the repository record in the DB (repositories.id) */
    string _repository_id;

    /** Object for processing the read data */
    data::ResolvableDataConsumer & _consumer;

    /** List of parser jobs read from repomd.xml and patches.xml files. */
    std::list<RepoParserJob> _jobs;

    /** Progress reporting object for overall YUM parser progress. */
    ProgressData _ticks;

    /** */
    const RepoParserOpts & _options;
  };
  ///////////////////////////////////////////////////////////////////////////


  RepoParser::Impl::Impl(
      const std::string & repository_id,
      data::ResolvableDataConsumer & consumer,
      const RepoParserOpts & options,
      const ProgressData::ReceiverFnc & progress)
    :
      _repository_id(repository_id), _consumer(consumer), _options(options)
  {
    _ticks.name("YUM RepoParser");
    _ticks.sendTo(progress);
  }

  // -------------------------------------------------------------------------

  bool RepoParser::Impl::repomd_CB(
    const OnMediaLocation & loc, const ResourceType & dtype)
  {
    DBG << "Adding " << dtype
        << " (" << loc.filename() << ") to RepoParser jobs " << endl;

    _jobs.push_back(RepoParserJob(loc.filename(), dtype));

    return true;
  }

  // -------------------------------------------------------------------------

  bool RepoParser::Impl::primary_CB(const data::Packagebase_Ptr & package_r)
  {
    data::Package_Ptr pkg = dynamic_pointer_cast<data::Package>(package_r);
    if (pkg)
      _consumer.consumePackage(_repository_id, pkg);
    else
      _consumer.consumeSourcePackage(_repository_id, dynamic_pointer_cast<data::SrcPackage>(package_r));

/*    MIL << "got package "
      << package.name << package.edition << " "
      << package.arch
      << endl;
    MIL << "checksum: " << package.checksum << endl;
    MIL << "summary: " << package.summary << endl;*/

    return true;
  }

  // -------------------------------------------------------------------------

  bool RepoParser::Impl::patches_CB(
    const OnMediaLocation & loc, const string & patch_id)
  {
    DBG << "Adding patch " << loc.filename() << " to RepoParser jobs " << endl;

    _jobs.push_back(RepoParserJob(loc.filename(), ResourceType::PATCH));

    return true;
  }

  // -------------------------------------------------------------------------

  bool RepoParser::Impl::patch_CB(const data::Patch_Ptr & patch)
  {
    _consumer.consumePatch( _repository_id, patch );

    MIL << "got patch "
      << patch->name << patch->edition << " "
      << patch->arch
      << endl;

    return true;
  }

  // -------------------------------------------------------------------------

  bool RepoParser::Impl::other_CB(
    const data::Resolvable_Ptr & res_ptr, const Changelog & changelog)
  {
    //_consumer.consumeChangelog(_repository_id, res_ptr, changelog);
/*
    DBG << "got changelog for "
      << res_ptr->name << res_ptr->edition << " "
      << res_ptr->arch
      << endl;

    DBG << "last entry: " << changelog.front() << endl;
*/
    return true;
  }

  // -------------------------------------------------------------------------

  bool RepoParser::Impl::filelist_CB(
    const data::Resolvable_Ptr & res_ptr, const data::Filenames & filenames)
  {
    //_consumer.consumeFilelist(_repository_id, res_ptr, filenames);
/*
    DBG << "got filelist for "
      << res_ptr->name << res_ptr->edition << " "
      << res_ptr->arch
      << endl;

    DBG << "last entry: " << filenames.front() << endl;
*/
    return true;
  }

  // --------------------------------------------------------------------------

  bool RepoParser::Impl::pattern_CB(const data::Pattern_Ptr & product_ptr)
  {
    _consumer.consumePattern(_repository_id, product_ptr);

    MIL << "got pattern " << product_ptr->name << endl;

    return true;
  }

  // --------------------------------------------------------------------------

  bool RepoParser::Impl::product_CB(const data::Product_Ptr & product_ptr)
  {
    _consumer.consumeProduct(_repository_id, product_ptr);

    MIL << "got product " << product_ptr->name
        << "-" << product_ptr->edition << endl;

    return true;
  }

  // --------------------------------------------------------------------------

  void RepoParser::Impl::parse(const Pathname &cache_dir)
  {
    zypp::parser::yum::RepomdFileReader(
        cache_dir + "/repodata/repomd.xml",
        bind(&RepoParser::Impl::repomd_CB, this, _1, _2));

     long long totalsize = 0;
     for(list<RepoParserJob>::const_iterator it = _jobs.begin();
         it != _jobs.end(); ++it)
     {
       RepoParserJob job = *it;
       totalsize += PathInfo(cache_dir + job.filename()).size();
     }

     MIL << "Total files size: " << totalsize << endl;
    _ticks.range(totalsize);
    _ticks.toMin();

    doJobs(cache_dir);

    _ticks.toMax();
  }

  // --------------------------------------------------------------------------

  void RepoParser::Impl::doJobs(const Pathname &cache_dir)
  {
    for(list<RepoParserJob>::const_iterator it = _jobs.begin();
        it != _jobs.end(); ++it)
    {
      RepoParserJob job = *it;
      // FIXME better way to do progress here?
      int jobsize = PathInfo(cache_dir + job.filename()).size();

      MIL << "going to parse " << job.type() << " file "
          << job.filename() << " (" << jobsize << " bytes)" << endl;

      switch(job.type().toEnum())
      {
        // parse primary.xml.gz
        case ResourceType::PRIMARY_e:
        {
          CombinedProgressData jobrcv( _ticks, jobsize );
          PrimaryFileReader(
            cache_dir + job.filename(),
            bind(&RepoParser::Impl::primary_CB, this, _1),
            jobrcv);
          break;
        }

        case ResourceType::PATCHES_e:
        {
          PatchesFileReader(
            cache_dir + job.filename(),
            bind(&RepoParser::Impl::patches_CB, this, _1, _2));
          // reset progress reporter max value (number of jobs changed if
          // there are patches to parse)
          _ticks.range( _ticks.max() + jobsize );
          // increase in the total bytes of the file
          if (!_ticks.incr( jobsize ))
            ZYPP_THROW(AbortRequestException());
          break;
        }

        case ResourceType::PATCH_e:
        {
          PatchFileReader(
            cache_dir + job.filename(),
            bind(&RepoParser::Impl::patch_CB, this, _1));
          // increase in the total bytes of the file
          if (!_ticks.incr( jobsize ))
            ZYPP_THROW(AbortRequestException());
          break;
        }

        case ResourceType::OTHER_e:
        {
          if (!_options.skipOther)
          {
            CombinedProgressData jobrcv( _ticks, jobsize );
            OtherFileReader(
              cache_dir + job.filename(),
              bind(&RepoParser::Impl::other_CB, this, _1, _2),
              jobrcv);
          }
          else
          {
            MIL << "skipping other.xml.gz" << endl;
            // increase in the total bytes of the file
            if (!_ticks.incr( jobsize ))
              ZYPP_THROW(AbortRequestException());
          }
          break;
        }

        case ResourceType::FILELISTS_e:
        {
          if (!_options.skipFilelists)
          {
            CombinedProgressData jobrcv( _ticks, jobsize );
            FilelistsFileReader(
              cache_dir + job.filename(),
              bind(&RepoParser::Impl::filelist_CB, this, _1, _2),
              jobrcv);
          }
          else
          {
            MIL << "skipping filelists.xml.gz";
            // increase in the total bytes of the file
          if (!_ticks.incr( jobsize ))
            ZYPP_THROW(AbortRequestException());
          }
          break;
        }

        case ResourceType::PATTERNS_e:
        {
          PatternFileReader(
            cache_dir + job.filename(),
            bind(&RepoParser::Impl::pattern_CB, this, _1));
          // increase in the total bytes of the file
          if (!_ticks.incr( jobsize ))
            ZYPP_THROW(AbortRequestException());
          break;
        }

        case ResourceType::PRODUCT_e:
        {
          ProductFileReader(
            cache_dir + job.filename(),
            bind(&RepoParser::Impl::product_CB, this, _1));
          // increase in the total bytes of the file
          if (!_ticks.incr( jobsize ))
            ZYPP_THROW(AbortRequestException());
          break;
        }

        default:
        {
          WAR << "Don't know how to read "
              << job.type() << " file "
              << job.filename() << endl;
        }
      }
    }
  }


  ///////////////////////////////////////////////////////////////////
  //
  //  CLASS : RepoParser
  //
  ///////////////////////////////////////////////////////////////////

  RepoParser::RepoParser(
      const std::string & repository_id,
      data::ResolvableDataConsumer & consumer,
      const RepoParserOpts & options,
      const ProgressData::ReceiverFnc & progress)
    :
      _pimpl(new Impl(repository_id, consumer, options, progress))
  {}


  RepoParser::~RepoParser()
  {}


  void RepoParser::parse(const Pathname & cache_dir)
  {
    _pimpl->parse(cache_dir);
  }


    } // ns yum
  } // ns parser
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
