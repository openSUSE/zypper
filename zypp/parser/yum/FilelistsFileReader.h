/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/FilelistsFileReader.h
 * Interface of filelists.xml.gz file reader.
 */
#ifndef ZYPP_PARSER_YUM_FILELISTFILEREADER_H_
#define ZYPP_PARSER_YUM_FILELISTFILEREADER_H_

#include "zypp/base/PtrTypes.h"
#include "zypp/base/NonCopyable.h"
#include "zypp/base/Function.h"
#include "zypp/data/ResolvableData.h"

#include "zypp/ProgressData.h"

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  /**
   * Reads through a filelists.xml.gz file and collects list of file contained
   * in packages.
   *
   * After each package is read, a \ref data::Resolvable
   * and \ref data::Filenames is prepared and \ref _callback
   * is called with these two objects passed in.
   *
   * The \ref _callback is provided on construction.
   *
   * \code
   * FilelistsFileReader reader(filelists_file,
   *                        bind(&SomeClass::callbackfunc, &SomeClassInstance, _1, _2));
   * \endcode
   */
  class FilelistsFileReader : private base::NonCopyable
  {
  public:
    /**
     * Callback definition.
     */
    typedef function<bool(const data::Resolvable_Ptr &, const data::Filenames &)> ProcessPackage;

    /**
     * CTOR. Creates also \ref xml::Reader and starts reading.
     * 
     * \param filelists_file the filelists.xml.gz file you want to read
     * \param callback function to process \ref _resolvable data.
     * \param progress progress reporting object
     *
     * \see FilelistsFileReader::ProcessPackage
     */
    FilelistsFileReader(
      const Pathname & filelists_file,
      const ProcessPackage & callback,
      const ProgressData::ReceiverFnc & progress = ProgressData::ReceiverFnc());

    /**
     * DTOR.
     */
    ~FilelistsFileReader();

  private:
    class Impl;
    RW_pointer<Impl,rw_pointer::Scoped<Impl> > _pimpl;
  };


    } // ns zypp
  } // ns parser
} // ns yum

#endif /*ZYPP_PARSER_YUM_FILELISTFILEREADER_H_*/

// vim: set ts=2 sts=2 sw=2 et ai:
