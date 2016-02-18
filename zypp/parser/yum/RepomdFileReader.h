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
   *
   * Depending on the \ref _callback type provided on construction, ResourceType may
   * additionally be passed as a plain string. This form allows handling custom
   * resource types (e.g. ones with embedded locale tag).
   *
   * \code
   * RepomdFileReader reader(repomd_file, 
   *                  bind( &SomeClass::callbackfunc, &SomeClassInstance, _1, _2 ) );
   * \endcode
   */
  class RepomdFileReader : private base::NonCopyable
  {
  public:
   /** Callbacl taking \ref OnMediaLocation and \ref repo::yum::ResourceType */
    typedef function< bool( const OnMediaLocation &, const repo::yum::ResourceType & )> ProcessResource;

    /** Alternate callback also receiving the ResourceType as string. */
    typedef function< bool( const OnMediaLocation &, const repo::yum::ResourceType &, const std::string & )> ProcessResource2;

   /**
    * CTOR. Creates also \ref xml::Reader and starts reading.
    * 
    * \param repomd_file is the repomd.xml file you want to read
    * \param callback is a function.
    *
    * \see RepomdFileReader::ProcessResource
    */
    RepomdFileReader( const Pathname & repomd_file, const ProcessResource & callback );
    /** \overload taking ProcessResource2 callback */
    RepomdFileReader( const Pathname & repomd_file, const ProcessResource2 & callback );

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
