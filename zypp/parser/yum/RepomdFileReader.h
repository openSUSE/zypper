/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/RepomdFileReader.h
 * Interface of repomd.xml file reader.
 */
#ifndef zypp_source_yum_RepomdFileReader_H
#define zypp_source_yum_RepomdFileReader_H

#include "zypp/base/PtrTypes.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/Function.h"

#include "zypp/OnMediaLocation.h"
#include "zypp/repo/yum/ResourceType.h"

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  /**
   * Reads through a repomd.xml file and collects type, location, checksum and
   * other data about metadata files to be processed.
   *
   * After each file entry is read, a \ref OnMediaLocation
   * and \ref repo::yum::ResourceType are prepared and passed to the \ref _callback.
   * ResourceType is additionally be passed as a plain string. This form allows
   * handling custom resource types (e.g. ones with embedded locale tag).
   */
  class RepomdFileReader : private base::NonCopyable
  {
  public:
    /** Callback taking \ref OnMediaLocation and \ref repo::yum::ResourceType (also as String)*/
    typedef function< bool( OnMediaLocation &&, const repo::yum::ResourceType &, const std::string & )> ProcessResource;

   /**
    * CTOR. Creates also \ref xml::Reader and starts reading.
    * 
    * \param repomd_file is the repomd.xml file you want to read
    * \param callback is a function.
    *
    * \see RepomdFileReader::ProcessResource
    */
    RepomdFileReader( const Pathname & repomd_file, const ProcessResource & callback );

    /** DTOR */
    ~RepomdFileReader();

  private:
    class Impl;
    RW_pointer<Impl,rw_pointer::Scoped<Impl> > _pimpl;
  };


    } // namespace yum
  } // namespace parser
} // namespace zypp

#endif // zypp_source_yum_RepomdFileReader_H
