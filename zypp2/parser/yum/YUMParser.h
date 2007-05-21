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
#include "zypp/data/ResolvableDataConsumer.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/source/yum/YUMResourceType.h"
#include "zypp/ProgressData.h"
#include "zypp/Changelog.h"


using zypp::source::yum::YUMResourceType;

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  /**
   * Structure encapsulating YUM parser data type and filename.
   */
  struct YUMParserJob
  {
    YUMParserJob(const Pathname & filename, const YUMResourceType & type)
      : _filename(filename), _type(type) {}

    const Pathname & filename() const { return _filename; } 
    const YUMResourceType & type() const { return _type; } 

  private:
    /** File to be processed */
    Pathname _filename;
    /** Type of YUM file */
    YUMResourceType _type;
  };


  /**
   * YUM metada parser.
   *
   * Reads repomd.xml file to get the list of files to parse and enques them
   * as YUMParserJobs. Then uses *FileReader classes to parse the files and
   * a \ref ResolvableDataConsumer to process the read data (typically to
   * store them in a database).
   *
   *
   * \code
   * 
   * cache::CacheStore store(dbdir);
   * data::RecordId catalog_id = store.lookupOrAppendCatalog(sourceurl, "/");
   *
   * YUMParser parser(catalog_id, store, &progress_function);
   * parser.start(source_cache_dir);
   *
   * store.commit();
   *
   * \code
   *
   * TODO make the parser configurable, e.g. exclude(FILELISTS_e)?
   *
   * \see RepomdFileReader, PrimaryFileReader, OtherFileReader
   * \see FilelistsFileReader, PatchesFileReader, PatchFileReader
   */
  class YUMParser
  {
  public:

    /**
     * CTOR
     *
     * \param catalog_id repository identifier
     * \param consumer consumer of parsed data
     * \param progress progress reporting function
     */
    YUMParser(
      const data::RecordId & catalog_id,
      data::ResolvableDataConsumer & consumer,
      const ProgressData::ReceiverFnc & progress = ProgressData::ReceiverFnc()
    );

    /**
     * Starts parsing of repository cache dir located at \a path.
     *
     * This method uses RepomdFileReader to get a list of parser jobs
     * and calls \ref doJobs(Pathname) to do them.
     *
     * \param path location of the raw repository cache
     */
    void start(const Pathname & path);

  private:
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
    bool repomd_CB(const OnMediaLocation & loc, const YUMResourceType & dtype);

    /**
     * Callback for processing packages returned from \ref PrimaryFileReader.
     * Uses \ref _consumer to process read package data.
     *
     * \param package_r pointer to package data
     */
    bool primary_CB(const data::Package_Ptr & package_r); 

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

  private:
    /** Object for processing the read data */
    data::ResolvableDataConsumer & _consumer;

    /** ID of the repository record in the DB (catalogs.id) */
    data::RecordId _catalog_id;

    /** List of parser jobs read from repomd.xml and patches.xml files. */
    std::list<YUMParserJob> _jobs;

    /** Progress reporting object for overall YUM parser progress. */
    ProgressData _ticks;
  };


    } // ns yum
  } // ns parser
} // ns zypp

#endif /*YUMPARSER_H_*/

// vim: set ts=2 sts=2 sw=2 et ai:

