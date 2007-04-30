/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <fstream>

#include "zypp/parser/yum/OtherFileReader.h"

using namespace std;
using namespace zypp::xml;

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  OtherFileReader::OtherFileReader(const Pathname & other_file, ProcessOther callback)
      : _callback(callback), _resolvable(NULL)
  {
    Reader reader(other_file);
    MIL << "Reading " << other_file << endl;
    reader.foreachNode(bind(&OtherFileReader::consumeNode, this, _1));
  }


  bool OtherFileReader::consumeNode(Reader & reader_r)
  {
    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      if (reader_r->name() == "package")
      {
        if (_resolvable) delete _resolvable;
        _resolvable = new zypp::data::Resolvable();

        _resolvable->name = reader_r->getAttribute("name").asString();
        _resolvable->arch = Arch(reader_r->getAttribute("arch").asString());
        return true;
      }
      if (reader_r->name() == "version")
      {
        _package->edition = Edition(reader_r->getAttribute("ver").asString(),
                                    reader_r->getAttribute("rel").asString(),
                                    reader_r->getAttribute("epoch").asString());
      }
    }
    else if (reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT)
    {
      if (reader_r->name() == "package")
      {
        // _callback(*_resolvable, changelog, checksum - package id);
        if (_resolvable)
        {
          delete _resolvable;
          _resolvable = NULL;
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
