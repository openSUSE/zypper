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

  private:

    /**
     * Callback provided to the XML reader.
     * 
     * \param  the xml reader object reading the file  
     * \return true to tell the reader to continue, false to tell it to stop
     *
     * \see PrimaryFileReader::consumeNode(xml::Reader)
     */
    bool consumeNode(xml::Reader & reader_r);

    /**
     * Process atoms node and all of its children.
     * 
     * \return true if current node has been completely processed, false
     *         if additional processing is required outside of the method. 
     */
    bool consumeAtomsNode(xml::Reader & reader_r);

    bool consumeMessageNode(xml::Reader & reader_r);

    bool consumeScriptNode(xml::Reader & reader_r);

    /**
     * Creates a new \ref data::Patch_Ptr, swaps its contents with \ref _patch
     * and returns it. Used to hand-out the data object to its consumer
     * (a \ref ProcessPatch function) after it has been read.
     */
    data::Patch_Ptr handoutPatch();

    void saveAtomInPatch();
    
    void copyAtomFromTmpObj(data::Atom_Ptr & atom_ptr) const;

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

    /** Pointer to an atom currently being read. */
    data::ResObject_Ptr _tmpResObj;
  };


    } // ns yum
  } // ns parser
} // ns zypp

#endif /*PATCHFILEREADER_H_*/
