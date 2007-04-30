/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <fstream>

#include "zypp/parser/yum/PatchFileReader.h"

using namespace std;
using namespace zypp::xml;

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  PatchFileReader::PatchFileReader(const Pathname & patch_file, ProcessPatch callback)
      : _callback(callback), _patch(NULL)
  {
    Reader reader(patch_file);
    MIL << "Reading " << patch_file << endl;
    reader.foreachNode(bind(&PatchFileReader::consumeNode, this, _1));
  }


  bool PatchFileReader::consumeNode(Reader & reader_r)
  {
    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      if (reader_r->name() == "patch")
      {
        if (_patch) delete _patch;
        _patch = new zypp::data::Patch();

        _patch->id = reader_r->getAttribute("patchid").asString();
        _patch->timestamp = Date(reader_r->getAttribute("timestamp").asString());
        return true;
      }
      if (reader_r->name() == "yum:name")
      {
        _patch->name = reader_r.nodeText().asString();
        return true;
      }
    }
    else if (reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT)
    {
      if (reader_r->name() == "patch")
      {
        _callback(*_patch);
        if (_patch)
        {
          delete _patch;
          _patch = NULL;
        }
        return true;
      }
    }
    return true;
  }


    } // ns yum
  } // ns parser
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
