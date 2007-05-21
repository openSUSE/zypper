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

  OtherFileReader::OtherFileReader(
      const Pathname & other_file,
      const ProcessPackage & callback,
      const ProgressData::ReceiverFnc & progress)
    :
      _callback(callback)
  {
    _ticks.sendTo(progress);
    _ticks.name("other.xml.gz");

    Reader reader(other_file);
    MIL << "Reading " << other_file << endl;
    reader.foreachNode(bind(&OtherFileReader::consumeNode, this, _1));
  }

  // --------------------------------------------------------------------------

  bool OtherFileReader::consumeNode(Reader & reader_r)
  {
    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      // xpath: /otherdata
      if (reader_r->name() == "otherdata")
      {
        unsigned total_packages;
        zypp::str::strtonum(reader_r->getAttribute("packages").asString(), total_packages);
        _ticks.range(total_packages);
        _ticks.toMin();
        return true;
      }

      // xpath: /otherdata/package (+)
      if (reader_r->name() == "package")
      {
        _resolvable = new data::Resolvable;
        _changelog.clear();

        _resolvable->name = reader_r->getAttribute("name").asString();
        _resolvable->arch = Arch(reader_r->getAttribute("arch").asString());

        return true;
      }

      // xpath: /otherdata/package/version
      if (reader_r->name() == "version")
      {
        _resolvable->edition = Edition(reader_r->getAttribute("ver").asString(),
                                    reader_r->getAttribute("rel").asString(),
                                    reader_r->getAttribute("epoch").asString());
        return true;
      }

      // xpath: /otherdata/package/changelog (*)
      if (reader_r->name() == "changelog")
      {
        ChangelogEntry entry(
              Date(reader_r->getAttribute("date").asString()),
              reader_r->getAttribute("author").asString(),
              reader_r.nodeText().asString());
        _changelog.push_back(entry);
        return true;
      }
    }

    else if (reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT)
    {
      // xpath: /otherdata/package
      if (reader_r->name() == "package")
      {
        if (_callback && !_changelog.empty())
          _callback(handoutResolvable(), _changelog);

        _ticks.incr();

        return true;
      }

      // xpath: /otherdata
      if (reader_r->name() == "otherdata")
      {
        _ticks.toMax();
        return true;
      }
    }

    return true;
  }

  // --------------------------------------------------------------------------

  data::Resolvable_Ptr OtherFileReader::handoutResolvable()
  {
    data::Resolvable_Ptr ret;
    ret.swap(_resolvable);
    return ret;
  }


    } // ns yum
  } // ns parser
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
