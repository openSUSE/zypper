/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp2/parser/yum/YUMParser.h
 *
 * YUM parser public API definition. 
 */
#ifndef YUMPARSER_H_
#define YUMPARSER_H_

#include "zypp/base/PtrTypes.h"
#include "zypp/base/NonCopyable.h"

#include "zypp/data/RecordId.h"
#include "zypp/data/ResolvableDataConsumer.h"

#include "zypp/ProgressData.h"


namespace zypp
{
  namespace parser
  {
    namespace yum
    {


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
   * data::RecordId repository_id = store.lookupOrAppendRepository(sourceurl, "/");
   *
   * YUMParser parser(repository_id, store, &progress_function);
   * parser.parse(source_cache_dir);
   *
   * store.commit();
   *
   * \code
   *
   * \todo make the parser configurable, e.g. exclude(FILELISTS_e)?
   *
   * \see RepomdFileReader, PrimaryFileReader, OtherFileReader
   * \see FilelistsFileReader, PatchesFileReader, PatchFileReader
   * \see PatternFileReader, ProductFileReader
   */
  class YUMParser : private base::NonCopyable
  {
  public:

    /**
     * CTOR
     *
     * \param repository_id repository identifier
     * \param consumer consumer of parsed data
     * \param progress progress reporting function
     */
    YUMParser(
      const data::RecordId & repository_id,
      data::ResolvableDataConsumer & consumer,
      const ProgressData::ReceiverFnc & progress = ProgressData::ReceiverFnc()
    );
    
    /**
     * DTOR 
     */
    ~YUMParser();

    /**
     * Starts parsing of repository cache dir located at \a path.
     *
     * This method uses RepomdFileReader to get a list of parser jobs
     * and calls \ref Impl::doJobs(Pathname) to do them.
     *
     * \param path location of the raw repository cache
     */
    void parse(const Pathname & path);

  private:
    class Impl;
    RW_pointer<Impl,rw_pointer::Scoped<Impl> > _pimpl;
  };


    } // ns yum
  } // ns parser
} // ns zypp

#endif /*YUMPARSER_H_*/

// vim: set ts=2 sts=2 sw=2 et ai:
