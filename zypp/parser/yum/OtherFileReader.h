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
#include "zypp/parser/ParserProgress.h"

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
   * like changelogs.
   *
   * After a package is read, a \ref zypp::data::Resolvable
   * and \ref changelog TODO is prepared and \ref _callback
   * is called with these two objects passed in. 
   *
   * The \ref _callback is provided on construction.
   *
   * \code
   * PrimaryFileReader reader(repomd_file, 
   *                          bind(&SomeClass::callbackfunc, &object, _1));
   * \endcode
   */
  class OtherFileReader
  {
  public:
    /**
     * Callback definition.
     */
    typedef function<bool(const zypp::data::Resolvable &)> ProcessPackage;


    /**
     * Constructor
     * \param other_file the other.xml.gz file you want to read
     * \param function to process \ref _resolvable data.
     * \param progress progress reporting function TODO better progress reporting
     * 
     * \see OtherFileReader::ProcessPackage
     */
    PrimaryFileReader(const Pathname & other_file,
        ProcessPackage callback, ParserProgress::Ptr progress);

    /**
     * Callback provided to the XML parser.
     */
    bool consumeNode(zypp::xml::Reader & reader_r);

  private:

    /**
     * Number of packages read so far.
     */
    unsigned _count;

    /**
     * Total number of packages to be read. This information is acquired from
     * the <code>packages</code> attribute of <code>otherdata<code> tag.
     */
    unsigned _total_packages;

    /**
     * Pointer to the \ref zypp::data::Resolvable object for storing the NVRA
     * data.
     */
    zypp::data::Resolvable *_resolvable;

    /**
     * Callback for processing package metadata passed in through constructor.
     */
    ProcessPackage _callback;

    /**
     * Progress reporting object.
     */
    ParserProgress::Ptr _progress;

    long int _old_progress;
  };


    } // ns zypp
  } // ns parser
} // ns yum

#endif /*OTHERFILEREADER_H_*/

// vim: set ts=2 sts=2 sw=2 et ai:
