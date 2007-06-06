/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/PatternFileReader.h
 * Interface of patterns.xml file reader.
 */
#ifndef ZYPP_PARSER_YUM_PATTERNFILEREADER_H_
#define ZYPP_PARSER_YUM_PATTERNFILEREADER_H_

#include "zypp/parser/yum/FileReaderBase.h"

namespace zypp
{

  namespace data
  {
    class Pattern;
    DEFINE_PTR_TYPE(Pattern);
  } // ns data


  namespace parser
  {
    namespace yum
    {


  /**
   * Reader of patterns.xml file conforming to schema definition located
   * at zypp/parser/yum/schema/patterns.rnc.
   * 
   * \see zypp::data::Pattern
   * \see zypp::parser::xml::Reader
   */
  class PatternFileReader : FileReaderBase
  {
  public:

    /**
     * Consumer callback definition. Function which will process the read
     * data must be of this type.
     */
    typedef function<bool(const data::Pattern_Ptr &)> ProcessPattern;

    /**
     * CTOR. Creates also \ref xml::Reader and starts reading.
     *
     * \param pattern_file pattern.xml file to read.
     * \param callback Function which will process read data.
     */
    PatternFileReader(const Pathname & pattern_file, ProcessPattern callback);
    
    /**
     * DTOR.
     */
    ~PatternFileReader();

  private:
    class Impl;
    RW_pointer<Impl,rw_pointer::Scoped<Impl> > _pimpl;
  };


    } // ns yum
  } // ns parser
} // ns zypp

#endif /*ZYPP_PARSER_YUM_PATTERNFILEREADER_H_*/

// vim: set ts=2 sts=2 sw=2 et ai:
