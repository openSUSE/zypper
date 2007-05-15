/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef ZYPP_PARSER_YUM_PRIMARYFILEPARSER_H
#define ZYPP_PARSER_YUM_PRIMARYFILEPARSER_H

#include "zypp/base/Function.h"
#include "zypp/parser/xml/Reader.h"
#include "zypp/parser/yum/FileReaderBase.h"
#include "zypp/data/ResolvableData.h"
#include "zypp/ProgressData.h"


namespace zypp
{
  namespace parser
  {
    namespace yum
    {

  /**
   * Reads through a primary.xml file and collects package data including
   * dependencies.
   * 
   * After a package is read, a \ref zypp::data::Package object is prepared
   * and \ref _callback is called with this object passed in.
   *
   * The \ref _callback is provided on construction.
   *
   * \code
   * PrimaryFileReader reader(repomd_file, 
   *                          bind(&SomeClass::callbackfunc, &object, _1));
   * \endcode
   */
  class PrimaryFileReader : public FileReaderBase
  {
  public:
    /**
     * Callback definition.
     */
    typedef function<bool(const data::Package_Ptr &)> ProcessPackage;

    /**
     * Constructor
     * \param primary_file the primary.xml.gz file you want to read
     * \param callback function to process \ref _package data.
     * \param progress progress reporting function
     * 
     * \see PrimaryFileReader::ProcessPackage
     */
    PrimaryFileReader(
      const Pathname & primary_file,
      const ProcessPackage & callback,
      const ProgressData::ReceiverFnc & progress = ProgressData::ReceiverFnc());

    /**
     * Callback provided to the XML parser.
     */
    bool consumeNode(xml::Reader & reader_r);

  private:
    /**
     * Creates a new Package_Ptr swaps its contents with \ref _package and
     * returns it.
     */
    data::Package_Ptr handoutPackage();

  private:
    /**
     * Callback for processing package metadata passed in through constructor.
     */
    ProcessPackage _callback;

    /**
     * \ref zypp::data::Package object for storing the package metada
     */
    data::Package_Ptr _package;

    /**
     * Progress reporting object.
     */
    ProgressData _ticks;
  };


    } // ns zypp
  } // ns parser
} // ns yum

#endif /* ZYPP_PARSER_YUM_PRIMARYFILEPARSER_H */

// vim: set ts=2 sts=2 sw=2 et ai:
