/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef PATCHFILEREADER_H_
#define PATCHFILEREADER_H_

#include "zypp/base/Function.h"
#include "zypp/parser/xml/Reader.h"
#include "zypp/parser/yum/FileReaderBase.h"
#include "zypp/data/ResolvableData.h"

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  /**
   * 
   */
  class PatchFileReader : FileReaderBase
  {
  public:

    /**
     * Callback definition.
     */
    typedef function<bool(const data::Patch_Ptr &)> ProcessPatch;

    /**
     * 
     */
    PatchFileReader(const Pathname & patch_file, ProcessPatch callback);

    /**
     * Callback provided to the XML parser.
     */
    bool consumeNode(xml::Reader & reader_r);


  private:
    data::Patch_Ptr handoutPatch();


  private:
    /**
     * Callback for processing patch metadata passed in through constructor.
     */
    ProcessPatch _callback;

    /**
     * Pointer to the \ref zypp::data::Patch object for storing the patch
     * metada (except of depencencies).
     */
    data::Patch_Ptr _patch;
  };


    } // ns yum
  } // ns parser
} // ns zypp

#endif /*PATCHFILEREADER_H_*/
