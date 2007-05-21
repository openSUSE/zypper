/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include <fstream>

#include "zypp/parser/yum/FilelistsFileReader.h"

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

  FilelistsFileReader::FilelistsFileReader(
      const Pathname & filelist_file,
      const ProcessPackage & callback,
      const ProgressData::ReceiverFnc & progress)
    :
      _callback(callback)
  {
    _ticks.sendTo(progress);
    _ticks.name("filelist.xml.gz");

    Reader reader(filelist_file);
    MIL << "Reading " << filelist_file << endl;
    reader.foreachNode(bind(&FilelistsFileReader::consumeNode, this, _1));
  }

  // --------------------------------------------------------------------------

  bool FilelistsFileReader::consumeNode(Reader & reader_r)
  {
    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      // xpath: /filelists
      if (reader_r->name() == "filelists")
      {
        unsigned total_packages;
        zypp::str::strtonum(reader_r->getAttribute("packages").asString(), total_packages);
        _ticks.range(total_packages);
        _ticks.toMin();
        return true;
      }

      // xpath: /filelists/package (+)
      if (reader_r->name() == "package")
      {
        _resolvable = new data::Resolvable;
        _filenames.clear();

        _resolvable->name = reader_r->getAttribute("name").asString();
        _resolvable->arch = Arch(reader_r->getAttribute("arch").asString());

        return true;
      }

      // xpath: /filelists/package/version
      if (reader_r->name() == "version")
      {
        _resolvable->edition = Edition(reader_r->getAttribute("ver").asString(),
                                    reader_r->getAttribute("rel").asString(),
                                    reader_r->getAttribute("epoch").asString());
        return true;
      }

      // xpath: /filelists/package/file (*)
      if (reader_r->name() == "file")
      {
        // ignoring type dir/ghost  reader_r->getAttribute("type").asString();
        _filenames.push_back(reader_r.nodeText().asString());
        return true;
      }
    }

    else if (reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT)
    {
      // xpath: /filelists/package
      if (reader_r->name() == "package")
      {
        if (_callback && !_filenames.empty())
          _callback(handoutResolvable(), _filenames);

        _ticks.incr();

        return true;
      }

      // xpath: /filelists
      if (reader_r->name() == "filelists")
      {
        _ticks.toMax();
        return true;
      }
    }

    return true;
  }

  // --------------------------------------------------------------------------

  data::Resolvable_Ptr FilelistsFileReader::handoutResolvable()
  {
    data::Resolvable_Ptr ret;
    ret.swap(_resolvable);
    return ret;
  }


    } // ns yum
  } // ns parser
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:

