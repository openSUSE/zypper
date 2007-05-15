/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <fstream>

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
    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      if (reader_r->name() == "patch")
      {
        _patch = new data::Patch;

        _patch->id = reader_r->getAttribute("patchid").asString();
        _patch->timestamp = Date(reader_r->getAttribute("timestamp").asString());
        // reader_r->getAttribute("timestamp").asString() ?
        return true;
      }

      if (reader_r->name() == "yum:name")
      {
        _patch->name = reader_r.nodeText().asString();
        return true;
      }

      if (reader_r->name() == "summary")
      {
        Locale locale(reader_r->getAttribute("lang").asString());
        _patch->summary.setText(reader_r.nodeText().asString(), locale);
        return true;
      }

      if (reader_r->name() == "description")
      {
        Locale locale(reader_r->getAttribute("lang").asString());
        _patch->description.setText(reader_r.nodeText().asString(), locale);
        return true;
      }
      
      if (reader_r->name() == "license-to-confirm")
      {
        Locale locale(reader_r->getAttribute("lang").asString());
        _patch->licenseToConfirm.setText(reader_r.nodeText().asString(), locale);
        return true;
      }

      if (reader_r->name() == "yum:version")
      {
        _patch->edition = Edition(reader_r->getAttribute("ver").asString(),
				  reader_r->getAttribute("rel").asString(),
				  reader_r->getAttribute("epoch").asString());
        return true;
      }

      if (consumeDependency(reader_r, _patch->deps))
        return true; 

      if (reader_r->name() == "category")
      {
        _patch->category = reader_r.nodeText().asString();
        return true;
      }

      if (reader_r->name() == "reboot-needed")
      {
        _patch->rebootNeeded = true;
        return true;
      }
      
      if (reader_r->name() == "package-manager")
      {
        _patch->affectsPkgManager = true;
        return true;
      }

      // TODO update-script

      // TODO atoms ((package|script|message)+)+
/*      if (reader_r->name() == "atoms")
      {
        tag(tag_atoms);
        return consumeAtomsNode();
      }
*/
    }
    else if (reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT)
    {
      if (reader_r->name() == "patch")
      {
      	if (_callback)
      	  _callback(handoutPatch());
      
      	return true;
      }
/*      if (reader_r->name() == "atoms")
      {
        toParent(tag_NONE);
        return true;
      }
*/
    }
    return true;
  }

  // --------------------------------------------------------------------------
/*
  bool PatchFileReader::consumeAtomsNode(Reader & reader_r)
  {
    return true;
  }
*/

  // --------------------------------------------------------------------------

  data::Patch_Ptr PatchFileReader::handoutPatch()
  {
    data::Patch_Ptr ret;
    ret.swap(_patch);
    return ret;
  }


    } // ns yum
  } // ns parser
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:

