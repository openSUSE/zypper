/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/RepoindexFileReader.h
 * Interface of repoindex.xml file reader.
 */
#ifndef zypp_source_yum_RepoindexFileReader_H
#define zypp_source_yum_RepoindexFileReader_H

#include "zypp/base/PtrTypes.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/Function.h"
#include "zypp/base/InputStream.h"
#include "zypp/Pathname.h"

namespace zypp
{
  class RepoInfo;

  namespace parser
  {

  /**
   * Reads through a repoindex.xml file and collects repositories.
   *
   * After each repository is read, a \ref RepoInfo
   * is prepared and \ref _callback
   * is called with this object passed in.
   *
   * The \ref _callback is provided on construction.
   *
   *
   * \code
   * RepoindexFileReader reader(repoindex_file,
   *                  bind( &SomeClass::callbackfunc, &SomeClassInstance, _1) );
   * \endcode
   */
  class RepoindexFileReader : private base::NonCopyable
  {
  public:
   /**
    * Callback definition.
    * First parameter is a \ref RepoInfo object with the resource
    * FIXME return value is ignored
    */
    typedef function< bool( const RepoInfo & )> ProcessResource;

   /**
    * CTOR. Creates also \ref xml::Reader and starts reading.
    *
    * \param repoindexFile is the repoindex.xml file you want to read
    * \param callback is a function.
    *
    * \see RepoindexFileReader::ProcessResource
    */
    RepoindexFileReader( const zypp::Pathname & repoindexFile,
                         const ProcessResource & callback);

    /**
     * \short Constructor. Creates the reader and start reading.
     *
     * \param is a valid input stream
     * \param callback Callback that will be called for each repository.
     *
     * \see RepoindexFileReader::ProcessResource
     */
     RepoindexFileReader( const InputStream &is,
                          const ProcessResource & callback );

    /**
     * DTOR
     */
    ~RepoindexFileReader();

  private:
    class Impl;
    RW_pointer<Impl,rw_pointer::Scoped<Impl> > _pimpl;
  };


  } // ns parser
} // ns zypp

#endif /*zypp_source_yum_RepoindexFileReader_H*/

// vim: set ts=2 sts=2 sw=2 et ai:
