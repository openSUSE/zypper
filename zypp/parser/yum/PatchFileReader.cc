/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "zypp/base/Logger.h"
#include "zypp/parser/yum/PatchFileReader.h"

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "parser"

using namespace std;
using namespace zypp::xml;

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  PatchFileReader::PatchFileReader(const Pathname & patch_file, ProcessPatch callback)
      : _callback(callback)
  {
    Reader reader(patch_file);
    MIL << "Reading " << patch_file << endl;
    reader.foreachNode(bind(&PatchFileReader::consumeNode, this, _1));
  }

  // --------------------------------------------------------------------------

  /*
   * xpath and multiplicity of processed nodes are included in the code
   * for convenience:
   * 
   * // xpath: <xpath> (?|*|+)
   * 
   * if multiplicity is ommited, then the node has multiplicity 'one'. 
   */

  // --------------------------------------------------------------------------

  bool PatchFileReader::consumeNode(Reader & reader_r)
  {
    if (isBeingProcessed(tag_atoms) && consumeAtomsNode(reader_r))
      return true;

    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      // xpath: /patch
      if (reader_r->name() == "patch")
      {
        tag(tag_patch);

        _patch = new data::Patch;

        _patch->id = reader_r->getAttribute("patchid").asString();
        _patch->timestamp = Date(reader_r->getAttribute("timestamp").asString());
        // reader_r->getAttribute("timestamp").asString() ?
        return true;
      }

      // xpath: /patch/yum:name
      if (reader_r->name() == "yum:name")
      {
        _patch->name = reader_r.nodeText().asString();
        return true;
      }

      // xpath: /patch/summary
      if (reader_r->name() == "summary")
      {
        Locale locale(reader_r->getAttribute("lang").asString());
        _patch->summary.setText(reader_r.nodeText().asString(), locale);
        return true;
      }

      // xpath: /patch/description
      if (reader_r->name() == "description")
      {
        Locale locale(reader_r->getAttribute("lang").asString());
        _patch->description.setText(reader_r.nodeText().asString(), locale);
        return true;
      }

      // xpath: /patch/licence-to-confirm (*)
      if (reader_r->name() == "license-to-confirm")
      {
        // TODO support several licenses in several languages?
        Locale locale(reader_r->getAttribute("lang").asString());
        _patch->licenseToConfirm.setText(reader_r.nodeText().asString(), locale);
        return true;
      }

      // xpath: /patch/yum:version
      if (reader_r->name() == "yum:version")
      {
        _patch->edition = Edition(reader_r->getAttribute("ver").asString(),
				  reader_r->getAttribute("rel").asString(),
				  reader_r->getAttribute("epoch").asString());
        return true;
      }

      // dependency block nodes
      if (consumeDependency(reader_r, _patch->deps))
        return true; 

      // xpath: /patch/category
      if (reader_r->name() == "category")
      {
        _patch->category = reader_r.nodeText().asString();
        return true;
      }

      // xpath: /patch/reboot-needed (?)
      if (reader_r->name() == "reboot-needed")
      {
        _patch->rebootNeeded = true;
        return true;
      }

      // xpath: /patch/package-manager (?)
      if (reader_r->name() == "package-manager")
      {
        _patch->affectsPkgManager = true;
        return true;
      }

      // xpath: /patch/update-script (?)
      if (reader_r->name() == "update-script")
      {
        _patch->updateScript = reader_r.nodeText().asString();
      }

      // xpath: /patch/atoms (+)
      if (reader_r->name() == "atoms")
      {
        // remember that we are processing atoms from now on
        // xpath: /patch/atoms/*
        tag(tag_atoms);
        // no need to further process this node so not calling
        // consumeAtomsNode() here
        return true;
      }
    }
    else if (reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT)
    {
      // xpath: /patch
      if (reader_r->name() == "patch")
      {
        if (!_patch->atoms.size())
          WAR << "No atoms found for patch " << _patch->name << " " << _patch->edition << endl; 

      	if (_callback)
      	  _callback(handoutPatch());

        toParentTag(); // just for the case of reuse somewhere/sometimes

      	return true;
      }
    }

    return true;
  }

  // --------------------------------------------------------------------------

  bool PatchFileReader::consumeAtomsNode(Reader & reader_r)
  {
    // consumePackageNode
    if (isBeingProcessed(tag_package) && consumePackageNode(reader_r))
      return true;
    // consumeMessageNode
    else if (isBeingProcessed(tag_message) && consumeMessageNode(reader_r))
      return true;
    // consumeScriptNode
    else if (isBeingProcessed(tag_script) && consumeScriptNode(reader_r))
      return true;


    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      // xpath: /patch/atoms/package
      if (reader_r->name() == "package")
      {
        // remember that we are processing package from now on
        // xpath: /patch/atoms/package/*
        tag(tag_package);
        // DBG << "Atom node, tagpath: " << tagPath() << endl;
        _tmpResObj = new data::PackageAtom;
        // process also the package node attributes
        consumePackageNode(reader_r);
        return true;
      }

      // xpath: /patch/atoms/message
      if (reader_r->name() == "message")
      {
        // remember that we are processing message from now on
        // xpath: /patch/atoms/message/*
        tag(tag_message);
        // DBG << "Message node, tagpath: " << tagPath() << endl;
        _tmpResObj = new data::Message;
        return true;
      }

      // xpath: /patch/atoms/script
      if (reader_r->name() == "script")
      {
        // remember that we are processing script from now on
        // xpath: /patch/atoms/script/*
        tag(tag_script);
        // DBG << "Script node, tagpath: " << tagPath() << endl;
        _tmpResObj = new data::Script;
        return true;
      }
    }
    else if (reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT)
    {
      // xpath: /patch/atoms/package
      if (reader_r->name() == "package")
      {
        // DBG << "Atom " << _tmpResObj->name << " " << _tmpResObj->edition << " successfully read." << endl;  

        saveAtomInPatch();
        toParentTag(); // back to processing of previous tag (atoms)
        return true;
      }

      // xpath: /patch/atoms/message
      if (reader_r->name() == "message")
      {
        // DBG << "Message " << _tmpResObj->name << " " << _tmpResObj->edition << " successfully read." << endl;

        saveAtomInPatch();
        toParentTag(); // back to processing of previous tag (atoms)
        return true;
      }

      // xpath: /patch/atoms/script
      if (reader_r->name() == "script")
      {
        // DBG << "Script " << _tmpResObj->name << " " << _tmpResObj->edition << " successfully read." << endl;

        saveAtomInPatch();
        toParentTag(); // back to processing of previous tag (atoms)
        return true;
      }

      // xpath: /patch/atoms
      if (reader_r->name() == "atoms")
      {
        toParentTag(); // back to processing of previous tag (patch)
        return true;
      }
    }

    return true;
  }

  // --------------------------------------------------------------------------

  bool PatchFileReader::consumePackageNode(Reader & reader_r)
  {
    if (isBeingProcessed(tag_patchrpm) && consumePatchrpmNode(reader_r))
      return true;
    else if (isBeingProcessed(tag_deltarpm) && consumeDeltarpmNode(reader_r))
      return true;

    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      // xpath: /patch/atoms/package/pkgfiles
      if (reader_r->name() == "pkgfiles")
      {
        tag(tag_pkgfiles);
        return true;
      }

      // xpath: /patch/atoms/package/pkgfiles/patchrpm (*)
      if (reader_r->name() == "patchrpm")
      {
        tag(tag_patchrpm);
        _patchrpm = new data::PatchRpm;
        return true;
      }

      // xpath: /patch/atoms/package/pkgfiles/deltarpm (*)
      if (reader_r->name() == "deltarpm")
      {
        tag(tag_deltarpm);
        _deltarpm = new data::DeltaRpm;
        return true;
      }

      // xpath: /patch/atoms/package/license-to-confirm (*)
      if (reader_r->name() == "license-to-confirm")
      {
        DBG << "got license-to-confirm, lang: " << reader_r->getAttribute("lang").asString();

        // no way to determine which translation is associated
        // with another, all previous will be overwritten with
        // the last one
        // TODO introduce an identifier in YUM data
        // TODO make this rely on tag order as a temporary solution?

        _tmpResObj->licenseToConfirm.setText(
            reader_r.nodeText().asString(),
            Locale(reader_r->getAttribute("lang").asString()));

        return true;
      }
    }

    else if (reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT)
    {
      // xpath: /patch/atoms/package/pkgfiles
      if (reader_r->name() == "pkgfiles")
      {
        toParentTag();
        return true;
      }

      // xpath: /patch/atoms/package/pkgfiles/patchrpm (*)
      if (reader_r->name() == "patchrpm")
      {
        data::PatchRpm_Ptr tmp;
        tmp.swap(_patchrpm);
        data::PackageAtom_Ptr patom_ptr = zypp::dynamic_pointer_cast<data::PackageAtom>(_tmpResObj);
        if (patom_ptr)
          patom_ptr->patchRpms.insert(tmp);
        toParentTag();
        return true;
      }

      // xpath: /patch/atoms/package/pkgfiles/deltarpm (*)
      if (reader_r->name() == "deltarpm")
      {
        data::DeltaRpm_Ptr tmp;
        tmp.swap(_deltarpm);
        data::PackageAtom_Ptr patom_ptr = zypp::dynamic_pointer_cast<data::PackageAtom>(_tmpResObj);
        if (patom_ptr)
          patom_ptr->deltaRpms.insert(tmp);
        toParentTag();
        return true;
      }
    }

    // FileReaderBase::consumePackageNode() call here, otherwise the pkgfiles
    // would not be read.
    data::Package_Ptr package_ptr = zypp::dynamic_pointer_cast<data::Package>(_tmpResObj);
    if (package_ptr)
    {
      // xpath: /patch/atoms/package/* (except pkgfiles/* and license-to-confirm) (*)
      if (isBeingProcessed(tag_package))
        return FileReaderBase::consumePackageNode(reader_r, package_ptr);
    }
    else
    {
      ZYPP_THROW(Exception("Error in parser code. Package atom object not found."));
    }

    return true;
  }

  // --------------------------------------------------------------------------

  bool PatchFileReader::consumePatchrpmNode(Reader & reader_r)
  {
    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      // xpath: /patch/atoms/package/patchrpm/location
      if (reader_r->name() == "location")
      {
        _patchrpm->location.filePath = reader_r->getAttribute("href").asString();
        // ignoring attribute 'base'
        return true;
      }

      // xpath: /patch/atoms/package/patchrpm/checksum
      if (reader_r->name() == "checksum")
      {
        _patchrpm->location.fileChecksum = CheckSum(
                  reader_r->getAttribute("type").asString(),
                  reader_r.nodeText().asString());
        return true;
      }

      // xpath: /patch/atoms/package/patchrpm/time
      if (reader_r->name() == "time")
      {
        _patchrpm->buildTime =
            Date(reader_r->getAttribute("build").asString());

        _patchrpm->fileTime =
            Date(reader_r->getAttribute("file").asString());

        return true;
      }

      // xpath: /patch/atoms/package/patchrpm/size
      if (reader_r->name() == "size")
      {
        // size of the rpm file
        _patchrpm->location.fileSize = str::strtonum<ByteCount::SizeType>(
            reader_r->getAttribute("package").asString());

        // size of ??
        _patchrpm->archiveSize = str::strtonum<ByteCount::SizeType>(
            reader_r->getAttribute("archive").asString());

        return true;
      }

      // xpath: /patch/atoms/package/patchrpm/baseversion (+)
      if (reader_r->name() == "baseversion")
      {
        data::BaseVersion_Ptr base_ptr = new data::BaseVersion;

        // size of the rpm file
        base_ptr->edition = Edition(reader_r->getAttribute("ver").asString(),
                                    reader_r->getAttribute("rel").asString(),
                                    reader_r->getAttribute("epoch").asString());

        _patchrpm->baseVersions.insert(base_ptr);
        return true;
      }
    }

    else if (reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT)
    {
      // xpath: /patch/atoms/package/pkgfiles/patchrpm
      if (reader_r->name() == "patchrpm")
      {
        return false;
      }
    }

    return true;
  }

  // --------------------------------------------------------------------------

  bool PatchFileReader::consumeDeltarpmNode(Reader & reader_r)
  {
    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      // xpath: /patch/atoms/package/deltarpm/location
      if (reader_r->name() == "location")
      {
        _deltarpm->location.filePath = reader_r->getAttribute("href").asString();
        // ignoring attribute 'base'
        return true;
      }

      // xpath: /patch/atoms/package/deltarpm/checksum
      if (reader_r->name() == "checksum")
      {
        _deltarpm->location.fileChecksum = CheckSum(
                  reader_r->getAttribute("type").asString(),
                  reader_r.nodeText().asString());
        return true;
      }

      // xpath: /patch/atoms/package/deltarpm/time
      if (reader_r->name() == "time")
      {
        _deltarpm->buildTime =
            Date(reader_r->getAttribute("build").asString());

        _deltarpm->fileTime =
            Date(reader_r->getAttribute("file").asString());

        return true;
      }

      // xpath: /patch/atoms/package/deltarpm/size
      if (reader_r->name() == "size")
      {
        // size of the rpm file
        _deltarpm->location.fileSize = str::strtonum<ByteCount::SizeType>(
            reader_r->getAttribute("package").asString());

        // size of ??
        _deltarpm->archiveSize = str::strtonum<ByteCount::SizeType>(
            reader_r->getAttribute("archive").asString());

        return true;
      }

      // xpath: /patch/atoms/package/deltarpm/baseversion
      if (reader_r->name() == "baseversion")
      {
        // size of the rpm file
        _deltarpm->baseVersion.edition = Edition(reader_r->getAttribute("ver").asString(),
                                    reader_r->getAttribute("rel").asString(),
                                    reader_r->getAttribute("epoch").asString());
        // checksum
        _deltarpm->baseVersion.checkSum =
            CheckSum("md5", reader_r->getAttribute("md5sum").asString());

        // build time
        _deltarpm->baseVersion.buildTime =
            Date(reader_r->getAttribute("buildtime").asString());

        // sequence info
        _deltarpm->baseVersion.sequenceInfo =
            reader_r->getAttribute("sequence_info").asString();

        return true;
      }
    }

    else if (reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT)
    {
      // xpath: /patch/atoms/package/pkgfiles/deltarpm
      if (reader_r->name() == "deltarpm")
      {
        return false;
      }
    }

    return true;
  }

  // --------------------------------------------------------------------------

  bool PatchFileReader::consumeMessageNode(Reader & reader_r)
  {
    if (consumeDependency(reader_r, _tmpResObj->deps))
      return true;

    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      // xpath: /patch/atoms/message/yum:name
      if (reader_r->name() == "yum:name")
      {
        _tmpResObj->name = reader_r.nodeText().asString();
        return true;
      }

      // xpath: /patch/atoms/message/yum:name
      if (reader_r->name() == "yum:version")
      {
        _tmpResObj->edition = Edition(reader_r->getAttribute("ver").asString(),
                                    reader_r->getAttribute("rel").asString(),
                                    reader_r->getAttribute("epoch").asString());
        return true;
      }

      // xpath: /patch/atoms/message//text
      if (reader_r->name() == "text")
      {
        data::Message_Ptr message = dynamic_pointer_cast<data::Message>(_tmpResObj);
        if (message)
          message->text.setText(
              reader_r.nodeText().asString(),
              Locale(reader_r->getAttribute("lang").asString()));
        return true;
      }
    }

    else if (reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT)
    {
      // xpath: /patch/atoms/message
      if (reader_r->name() == "message")
      {
        return false;
      }
    }

    return true;
  }

  // --------------------------------------------------------------------------

  bool PatchFileReader::consumeScriptNode(Reader & reader_r)
  {
    if (consumeDependency(reader_r, _tmpResObj->deps))
      return true;

    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      data::Script_Ptr script = dynamic_pointer_cast<data::Script>(_tmpResObj);
      if (!script)
      {
        WAR << "data::Script object expected, but not found" << endl;
        return true;
      }

      // xpath: /patch/atoms/script/yum:name
      if (reader_r->name() == "yum:name")
      {
        _tmpResObj->name = reader_r.nodeText().asString();
        return true;
      }

      // xpath: /patch/atoms/script/yum:version
      if (reader_r->name() == "yum:version")
      {
        _tmpResObj->edition = Edition(reader_r->getAttribute("ver").asString(),
                                    reader_r->getAttribute("rel").asString(),
                                    reader_r->getAttribute("epoch").asString());
        return true;
      }

      // xpath: /patch/atoms/script/do
      if (reader_r->name() == "do")
      {
        script->doScript = reader_r.nodeText().asString();
        return true;
      }

      // xpath: /patch/atoms/script/undo
      if (reader_r->name() == "undo")
      {
        script->undoScript = reader_r.nodeText().asString();
        return true;
      }

      // xpath: /patch/atoms/script/do-location
      if (reader_r->name() == "do-location")
      {
        // xsd:anyURI do script file path base (not used)
        // ignoring reader_r->getAttribute("xml:base").asString();

        // xsd:anyURI do script file path
        script->doScriptLocation.filePath = reader_r->getAttribute("href").asString();
        return true;
      }

      // xpath: /patch/atoms/script/do-checksum
      if (reader_r->name() == "do-checksum")
      {
        script->doScriptLocation.fileChecksum = CheckSum(
                            reader_r->getAttribute("type").asString(),
                            reader_r.nodeText().asString());
        return true;
      }

      // xpath: /patch/atoms/script/undo-location
      if (reader_r->name() == "undo-location")
      {
        // xsd:anyURI undo script file path base (not used)
        // ignoring reader_r->getAttribute("xml:base").asString();

        // xsd:anyURI undo script file path
        script->undoScriptLocation.filePath = reader_r->getAttribute("href").asString();
        return true;
      }

      // xpath: /patch/atoms/script/undo-checksum
      if (reader_r->name() == "undo-checksum")
      {
        script->undoScriptLocation.fileChecksum = CheckSum(
                            reader_r->getAttribute("type").asString(),
                            reader_r.nodeText().asString());
        return true;
      }
    }

    else if (reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT)
    {
      // xpath: /patch/atoms/script
      if (reader_r->name() == "script")
      {
        return false;
      }
    }

    return true;
  }

  // --------------------------------------------------------------------------

  data::Patch_Ptr PatchFileReader::handoutPatch()
  {
    data::Patch_Ptr ret;
    ret.swap(_patch);
    return ret;
  }

  // --------------------------------------------------------------------------

  void PatchFileReader::saveAtomInPatch()
  {
    data::ResObject_Ptr tmp;
    tmp.swap(_tmpResObj);
    _patch->atoms.insert(tmp);
  }


    } // ns yum
  } // ns parser
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
