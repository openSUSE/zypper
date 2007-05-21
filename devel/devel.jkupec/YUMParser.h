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
   * 
   */
  class YUMParser
  {
  public:
    YUMParser(
      const data::RecordId & catalog_id,
      data::ResolvableDataConsumer & consumer,
      const ProgressData::ReceiverFnc & progress = ProgressData::ReceiverFnc()
    );

    void start(const zypp::Pathname & path);

  private:
    void doJobs(const zypp::Pathname & path);

    bool repomd_CB(const OnMediaLocation & loc, const YUMResourceType & dtype);
    bool primary_CB(const data::Package_Ptr & package_r); 
    bool patches_CB(const OnMediaLocation &loc, const std::string & patch_id);
    bool patch_CB(const data::Patch_Ptr & patch);
    bool other_CB(const data::Resolvable_Ptr & res_ptr, const Changelog & changelog);
    bool filelist_CB(const data::Resolvable_Ptr & res_ptr, const data::Filenames & filenames);

  private:
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

