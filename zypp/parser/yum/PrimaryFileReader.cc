/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#include "zypp/base/Logger.h"

#include "zypp/parser/yum/PrimaryFileReader.h"

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


  PrimaryFileReader::PrimaryFileReader(
      const Pathname & primary_file,
      const ProcessPackage & callback,
      const ProgressData::ReceiverFnc & progress)
    :
      _callback(callback)
  {
    _ticks.sendTo(progress);

    Reader reader(primary_file);
    MIL << "Reading " << primary_file << endl;
    reader.foreachNode(bind( &PrimaryFileReader::consumeNode, this, _1 ));
  }

  // --------------------------------------------------------------------------

  bool PrimaryFileReader::consumeNode(Reader & reader_r)
  {
//    DBG << "**node: " << reader_r->name() << " (" << reader_r->nodeType() << ")" << endl;
    if (isBeingProcessed(tag_package) && consumePackageNode(reader_r, _package))
      return true;


    if (reader_r->nodeType() == XML_READER_TYPE_ELEMENT)
    {
      if (reader_r->name() == "metadata")
      {
        unsigned total_packages;
        zypp::str::strtonum(reader_r->getAttribute("packages").asString(), total_packages);
        _ticks.range(total_packages);
        _ticks.toMin();
        return true;
      }

      if (reader_r->name() == "package")
      {
        tag(tag_package);
  //      DBG << "got " << reader_r->getAttribute("type") << " package" << endl;
        _package = new data::Package;

        return consumePackageNode(reader_r, _package);
      }
    }

    else if ( reader_r->nodeType() == XML_READER_TYPE_END_ELEMENT )
    {
      if (reader_r->name() == "package")
      {
        if (_package && _callback)
          _callback(handoutPackage());

        _ticks.incr();

        toParentTag();
        return true;
      }

      if (reader_r->name() == "metadata")
      {
        _ticks.toMax();
        return true;
      }
    }

    return true;
  }

  // --------------------------------------------------------------------------

  data::Package_Ptr PrimaryFileReader::handoutPackage()
  {
    data::Package_Ptr ret;
    ret.swap(_package);
    return ret;
  }


    } // ns yum
  } // ns parser
} //ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
