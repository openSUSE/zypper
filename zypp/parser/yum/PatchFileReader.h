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
#include "zypp/base/Logger.h"
#include "zypp/parser/xml/Reader.h"
#include "zypp/data/ResolvableData.h"

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "parser"

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  /**
   * 
   */
  class PatchFileReader
  {
  public:

    /**
     * Callback definition.
     */
    typedef function<bool(const zypp::data::Patch &)> ProcessPatch;

    /**
     * 
     */
    PatchFileReader(const Pathname & patch_file, ProcessPatch callback);

    /**
     * Callback provided to the XML parser.
     */
    bool consumeNode(zypp::xml::Reader & reader_r);

  private:
    /**
     * Callback for processing patch metadata passed in through constructor.
     */
    ProcessPatch _callback;

    /**
     * Pointer to the \ref zypp::data::Patch object for storing the patch
     * metada (except of depencencies).
     */
    zypp::data::Patch *_patch;

    /**
     * A map of lists of strings for storing dependencies.
     * 
     * \see zypp::data::Dependencies
     */
    zypp::data::Dependencies _deps;
  };


    }
  }
}

#endif /*PATCHFILEREADER_H_*/
