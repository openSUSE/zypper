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
#include "zypp2/cache/CacheStore.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/source/yum/YUMResourceType.h"
#include "zypp/ProgressData.h"


using zypp::source::yum::YUMResourceType;

namespace zypp
{
  namespace parser
  {
    namespace yum
    {

  /**
   * 
   */
  struct YUMParserJob
  {
    YUMParserJob(const Pathname & filename, const YUMResourceType & type)
      : _filename(filename), _type(type) {}

    const Pathname & filename() const { return _filename; } 
    const YUMResourceType & type() const { return _type; } 

  private:
    Pathname _filename;
    YUMResourceType _type;
  };


  /**
   * 
   */
  class YUMParser
  {
  public:
    YUMParser(
      const zypp::data::RecordId & catalog_id,
      zypp::cache::CacheStore & consumer,
      const ProgressData::ReceiverFnc & progress = ProgressData::ReceiverFnc()
    );

    void start(const zypp::Pathname & path);

  private:
    void doJobs(const zypp::Pathname & path);

    bool repomd_CB(const OnMediaLocation & loc, const YUMResourceType & dtype);
    bool primary_CB(const data::Package_Ptr & package_r); 
    bool patches_CB(const OnMediaLocation &loc, const std::string & patch_id);
    bool patch_CB(const data::Patch_Ptr & patch);

  private:
    zypp::cache::CacheStore & _consumer;
    
    /** ID of the repository record in the DB (catalogs.id) */
    zypp::data::RecordId _catalog_id;

    /** List of parser jobs read from repomd.xml and patches.xml files. */
    std::list<YUMParserJob> _jobs;

    /** Progress reporting object. */
    ProgressData _ticks;
  };


    } // ns yum
  } // ns parser
} // ns zypp

#endif /*YUMPARSER_H_*/

// vim: set ts=2 sts=2 sw=2 et ai:
