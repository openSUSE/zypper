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
   * After each package is read, a \ref OnMediaLocation
   * and \ref repo::yum::ResourceType is prepared and \ref _callback
   * is called with these two objects passed in.
   *
   * The \ref _callback is provided on construction.
   *
   *
   * \code
   * RepomdFileReader reader(repomd_file, 
   *                  bind( &SomeClass::callbackfunc, &SomeClassInstance, _1, _2 ) );
   * \endcode
   */
  class RepomdFileReader : private base::NonCopyable
  {
  public:
   /**
    * Callback definition.
    * First parameter is a \ref OnMediaLocation object with the resource
    * second parameter is the resource type.
    */
    typedef function< bool(
        const OnMediaLocation &,
        const repo::yum::ResourceType &)>
      ProcessResource;

   /**
    * CTOR. Creates also \ref xml::Reader and starts reading.
    * 
    * \param repomd_file is the repomd.xml file you want to read
    * \param callback is a function.
    *
    * \see RepomdFileReader::ProcessResource
    */
    RepomdFileReader(
      const Pathname & repomd_file, const ProcessResource & callback);

    /**
     * DTOR
     */
    ~RepomdFileReader();

  private:
    class Impl;
    RW_pointer<Impl,rw_pointer::Scoped<Impl> > _pimpl;
  };


    } // ns yum
  } // ns parser
} // ns zypp

#endif /*zypp_source_yum_RepomdFileReader_H*/

// vim: set ts=2 sts=2 sw=2 et ai:
