/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/PatchFileReader.h
 * Interface definition of patch XML file reader.
 */
#ifndef PATCHFILEREADER_H_
#define PATCHFILEREADER_H_

#include "zypp/base/Function.h"
#include "zypp/parser/yum/FileReaderBase.h"

namespace zypp
{

  namespace data
  {
    class Patch;
    DEFINE_PTR_TYPE(Patch);
  } // ns data


  namespace parser
  {
    namespace yum
    {


  /**
   * Reader of patch.xml files conforming to schema definition located
   * at zypp/parser/yum/schema/patch.rnc.
   * 
   * \see zypp::data::Patch
   * \see zypp::parser::xml::Reader
   */
  class PatchFileReader : FileReaderBase
  {
  public:

    /**
     * Consumer callback definition. Function which will process the read
     * data must be of this type.
     */
    typedef function<bool(const data::Patch_Ptr &)> ProcessPatch;

    /**
     * CTOR. Creates also \ref xml::Reader and starts reading.
     *
     * \param patch_file patch.xml file to read.
     * \param callback Function which will process read data.
     */
    PatchFileReader(const Pathname & patch_file, ProcessPatch callback);

    /**
     * DTOR.
     */
    ~PatchFileReader();

  private:
    class Impl;
    RW_pointer<Impl,rw_pointer::Scoped<Impl> > _pimpl;
  };


    } // ns yum
  } // ns parser
} // ns zypp

#endif /*PATCHFILEREADER_H_*/
