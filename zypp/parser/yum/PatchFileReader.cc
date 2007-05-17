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
      
      // xpath: /patch/licence-to-confirm
      if (reader_r->name() == "license-to-confirm")
      {
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

      // xpath: /patch/reboot-needed
      if (reader_r->name() == "reboot-needed")
      {
        _patch->rebootNeeded = true;
        return true;
      }

      // xpath: /patch/package-manager
      if (reader_r->name() == "package-manager")
      {
        _patch->affectsPkgManager = true;
        return true;
      }

      // TODO xpath: /patch/update-script

      // xpath: /patch/atoms
      if (reader_r->name() == "atoms")
      {
        // remember that we are processing atoms from now on
        // xpath: /patch/atoms/*
        tag(tag_atoms);
        // no need to further process this node so not calling consumeAtomsNode();
        return true;
      }
    }
    else if (reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT)
    {
      // xpath: /patch
      if (reader_r->name() == "patch")
      {
        // TODO some validation? e.g. _patch->atoms.size() > 0 

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
    if (isBeingProcessed(tag_package))
    {
       data::Package_Ptr package_ptr = zypp::dynamic_pointer_cast<data::Package>(_tmpResObj); 
       if (package_ptr && consumePackageNode(reader_r, package_ptr))
        return true;
    }
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
        // this object will be used by FileReaderBase::consumePackageNodes()
        // which needs Package object, not Atom. But it will be saved as Atom
        // after it's been filled.
        data::Package_Ptr package = new data::Package;
        consumePackageNode(reader_r, package);
        _tmpResObj = package;
        return true;
      }

      // xpath: /patch/atoms/message
      if (reader_r->name() == "message")
      {
        tag(tag_message);
        _tmpResObj = new data::Message;
        return true;
      }

      // xpath: /patch/atoms/script
      if (reader_r->name() == "script")
      {
        tag(tag_script);
        _tmpResObj = new data::Script;
        return true;
      }
    }
    else if (reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT)
    {
      // xpath: /patch/atoms/package
      if (reader_r->name() == "package")
      {
        data::Atom_Ptr atom_ptr = new data::Atom;
        copyAtomFromTmpObj(atom_ptr); 
        _patch->atoms.insert(atom_ptr);

        DBG << "Atom " << atom_ptr->name << " " << atom_ptr->edition << " successfully read." << endl;  

        // get rid of the old package object
        data::ResObject_Ptr tmp;
        tmp.swap(_tmpResObj);

        toParentTag(); // back to processing of previous tag (atoms)
        return true;
      }

      // xpath: /patch/atoms/message
      if (reader_r->name() == "message")
      {
        DBG << "Message " << _tmpResObj->name << " " << _tmpResObj->edition << " successfully read." << endl;

        saveAtomInPatch();
        toParentTag(); // back to processing of previous tag (atoms)
        return true;
      }

      // xpath: /patch/atoms/script
      if (reader_r->name() == "script")
      {
        DBG << "Script " << _tmpResObj->name << " " << _tmpResObj->edition << " successfully read." << endl;

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

/*  bool PatchFileReader::consumePackageNode(Reader & reader_r)
  {
     data::Package_Ptr package_ptr = zypp::dynamic_pointer_cast<data::Package>(_tmpResObj); 
     if (package_ptr && consumePackageNode(reader_r, package_ptr))
      return true;
    
    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      // TODO package extensions -> pkg-files, license-to-confirm
      if (reader_r->name() == "")
      {
        return true;
      }
    }

    return true;
  }
*/
  // --------------------------------------------------------------------------

  bool PatchFileReader::consumeMessageNode(Reader & reader_r)
  {
    if (FileReaderBase::consumeDependency(reader_r, _tmpResObj->deps))
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
    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
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

  // --------------------------------------------------------------------------

  void PatchFileReader::copyAtomFromTmpObj(data::Atom_Ptr & atom_ptr) const
  {
    atom_ptr->name = _tmpResObj->name;
    atom_ptr->edition = _tmpResObj->edition;
    atom_ptr->arch = _tmpResObj->arch;
    atom_ptr->deps = _tmpResObj->deps;
    atom_ptr->vendor = _tmpResObj->vendor;
    atom_ptr->installedSize = _tmpResObj->installedSize;
    atom_ptr->buildTime = _tmpResObj->buildTime;
    atom_ptr->installOnly = _tmpResObj->installOnly;
    atom_ptr->summary = _tmpResObj->summary;
    atom_ptr->description = _tmpResObj->description;
    atom_ptr->licenseToConfirm = _tmpResObj->licenseToConfirm;
    atom_ptr->insnotify = _tmpResObj->insnotify;
    atom_ptr->delnotify = _tmpResObj->delnotify;
  }


    } // ns yum
  } // ns parser
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
