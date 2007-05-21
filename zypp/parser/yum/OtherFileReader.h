/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef OTHERFILEREADER_H_
#define OTHERFILEREADER_H_

#include "zypp/base/Function.h"
#include "zypp/base/Logger.h"
#include "zypp/parser/xml/Reader.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/ProgressData.h"
#include "zypp/Changelog.h"

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "parser"

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  /**
   * Reads through a other.xml file and collects additional package data
   * (currently only changelogs).
   *
   * After a package is read, a \ref data::Resolvable
   * and \ref Changelog is prepared and \ref _callback
   * is called with these two objects passed in.
   *
   * The \ref _callback is provided on construction.
   *
   * \code
   * OtherFileReader reader(other_file,
   *                        bind(&SomeClass::callbackfunc, &object, _1));
   * \endcode
   */
  class OtherFileReader
  {
  public:
    /**
     * Callback definition.
     */
    typedef function<bool(const data::Resolvable_Ptr &, const Changelog)> ProcessPackage;

    /**
     * Constructor
     * \param other_file the other.xml.gz file you want to read
     * \param callback function to process \ref _resolvable data.
     * \param progress progress reporting object
     *
     * \see OtherFileReader::ProcessPackage
     */
    OtherFileReader(
      const Pathname & other_file,
      const ProcessPackage & callback,
      const ProgressData::ReceiverFnc & progress = ProgressData::ReceiverFnc());

  private:

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


    } // ns zypp
  } // ns parser
} // ns yum

#endif /*OTHERFILEREADER_H_*/

// vim: set ts=2 sts=2 sw=2 et ai:
