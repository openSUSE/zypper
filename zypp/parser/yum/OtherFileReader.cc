/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/OtherFileReader.cc
 * Implementation of other.xml.gz file reader.
 */
#include "zypp/base/Logger.h"
#include "zypp/parser/xml/Reader.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/Changelog.h"
#include "zypp/base/UserRequestException.h"

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


  ///////////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : OtherFileReader::Impl
  //
  class OtherFileReader::Impl : private base::NonCopyable
  {
  public:
    Impl(
    const Pathname & other_file,
    const ProcessPackage & callback,
    const ProgressData::ReceiverFnc & progress);

  public:

    /**
     * Callback provided to the XML parser.
     */
    bool consumeNode(xml::Reader & reader_r);

    /**
     * Creates a new \ref data::Resolvable_Ptr, swaps its contents with
     * \ref _resolvable and returns it. Used to hand-out the data object to its consumer
     * (a \ref ProcessPackage function) after it has been read.
     */
    data::Resolvable_Ptr handoutResolvable();

  private:

    /**
     * Pointer to the \ref zypp::data::Resolvable object for storing the NVRA
     * data.
     */
    zypp::data::Resolvable_Ptr _resolvable;

    /**
     * Changelog of \ref _resolvable.
     */
    Changelog _changelog;

    /**
     * Callback for processing package metadata passed in through constructor.
     */
    ProcessPackage _callback;

    /**
     * Progress reporting object.
     */
    ProgressData _ticks;
  };
  ///////////////////////////////////////////////////////////////////////

  OtherFileReader::Impl::Impl(
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
    reader.foreachNode(bind(&OtherFileReader::Impl::consumeNode, this, _1));
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

  bool OtherFileReader::Impl::consumeNode(Reader & reader_r)
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

        if (!_ticks.incr())
          ZYPP_THROW(AbortRequestException());

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

  data::Resolvable_Ptr OtherFileReader::Impl::handoutResolvable()
  {
    data::Resolvable_Ptr ret;
    ret.swap(_resolvable);
    return ret;
  }


  ///////////////////////////////////////////////////////////////////
  //
  //  CLASS NAME : OtherFileReader
  //
  ///////////////////////////////////////////////////////////////////

  OtherFileReader::OtherFileReader(
      const Pathname & other_file,
      const ProcessPackage & callback,
      const ProgressData::ReceiverFnc & progress)
    :
      _pimpl(new Impl(other_file, callback, progress))
  {}

  OtherFileReader::~OtherFileReader()
  {}


    } // ns yum
  } // ns parser
} // ns zypp

// vim: set ts=2 sts=2 sw=2 et ai:
