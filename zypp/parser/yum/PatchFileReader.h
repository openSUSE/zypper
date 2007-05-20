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
   * Reader of patch.xml files conforming to RNC/RNG definition located
   * in zypp/parser/yum/schema/patch.rn(c|g).
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
     * Process <tt>atoms</tt> node and all of its children.
     * 
     * \param reader_r XML file reader reading the patch file.
     * \return true if current node has been completely processed, false
     *         if additional processing is required outside of the method. 
     */
    bool consumeAtomsNode(xml::Reader & reader_r);

    /**
     * Process <tt>package</tt> node and all of its children. This method
     * uses \ref FileReaderBase::consumePackageNode(xml::Reader,data::Package_Ptr)
     * method and adds <tt>pkgfiles</tt> element processing.
     * 
     * \param reader_r XML file reader reading the patch file.
     * \return true if current node has been completely processed, false
     *         if additional processing is required outside of the method. 
     */
    bool consumePackageNode(xml::Reader & reader_r);

    /**
     * Process <tt>patchrpm</tt> node and all of its children.
     * 
     * \param reader_r XML file reader reading the patch file.
     * \return true if current node has been completely processed, false
     *         if additional processing is required outside of the method. 
     */
    bool consumePatchrpmNode(xml::Reader & reader_r);

    /**
     * Process <tt>deltarpm</tt> node and all of its children.
     * 
     * \param reader_r XML file reader reading the patch file.
     * \return true if current node has been completely processed, false
     *         if additional processing is required outside of the method. 
     */
    bool consumeDeltarpmNode(xml::Reader & reader_r);

    /**
     * Process <tt>message</tt> node and all of its children.
     * 
     * \param reader_r XML file reader reading the patch file.
     * \return true if current node has been completely processed, false
     *         if additional processing is required outside of the method. 
     */
    bool consumeMessageNode(xml::Reader & reader_r);

    /**
     * Process <tt>script</tt> node and all of its children.
     * 
     * \param reader_r XML file reader reading the patch file.
     * \return true if current node has been completely processed, false
     *         if additional processing is required outside of the method. 
     */
    bool consumeScriptNode(xml::Reader & reader_r);

    /**
     * Creates a new \ref data::Patch_Ptr, swaps its contents with \ref _patch
     * and returns it. Used to hand-out the data object to its consumer
     * (a \ref ProcessPatch function) after it has been read.
     */
    data::Patch_Ptr handoutPatch();

    /**
     * Creates a new \ref data::ResObject_Ptr, swap its contents with
     * \ref _tmpResObj and inserts it into <tt>_patch.atoms</tt>. Used
     * after an atom is read.
     */
    void saveAtomInPatch();

    /**
     * 
     */
    void copyPackageAtomFromTmpObj(data::Atom_Ptr & atom_ptr) const;

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

    /**
     * Pointer to an atom currently being read. This can be either
     * a \ref data::PackageAtom, \ref data::Message, or \ref data::Script.
     *
     * \see data::Patch::atoms
     */
    data::ResObject_Ptr _tmpResObj;

    /** Data object for storing patchrpm data */
    data::PatchRpm_Ptr _patchrpm;

    /** Data object for storing deltarpm data */
    data::DeltaRpm_Ptr _deltarpm;
  };


    } // ns yum
  } // ns parser
} // ns zypp

#endif /*PATCHFILEREADER_H_*/
