/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/PrimaryFileReader.h
 * Interface definition of primary.xml.gz file reader.
 */
#ifndef ZYPP_PARSER_YUM_PRIMARYFILEPARSER_H
#define ZYPP_PARSER_YUM_PRIMARYFILEPARSER_H

#include "zypp/ProgressData.h"

#include "zypp/parser/yum/FileReaderBase.h"

namespace zypp
{

  namespace data
  {
    class Package;
    DEFINE_PTR_TYPE(Package);
  } // ns data


  namespace parser
  {
    namespace yum
    {

  /**
   * Reads through a primary.xml.gz file and collects package data including
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
     * CTOR. Creates also \ref xml::Reader and starts reading.
     * 
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
     * DTOR.
     */
    ~PrimaryFileReader();

  private:
    class Impl;
    RW_pointer<Impl,rw_pointer::Scoped<Impl> > _pimpl;
  };


    } // ns zypp
  } // ns parser
} // ns yum

#endif /* ZYPP_PARSER_YUM_PRIMARYFILEPARSER_H */

// vim: set ts=2 sts=2 sw=2 et ai:
