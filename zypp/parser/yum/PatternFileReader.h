/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/

#ifndef PATTERNFILEREADER_H_
#define PATTERNFILEREADER_H_

#include "zypp/base/Function.h"
#include "zypp/parser/xml/Reader.h"
#include "zypp/parser/yum/FileReaderBase.h"
#include "zypp/data/ResolvableData.h"

namespace zypp
{
  namespace parser
  {
    namespace yum
    {


  /**
   * Reader of pattern.xml files conforming to RNC/RNG definition located
   * in zypp/parser/yum/schema/pattern.rn(c|g).
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

  private:

    /**
     * Callback provided to the XML reader.
     * 
     * \param  the xml reader object reading the file  
     * \return true to tell the reader to continue, false to tell it to stop
     *
     * \see PrimaryFileReader::consumeNode(xml::Reader)
     */
    bool consumeNode(xml::Reader & reader_r);

    /**
     * Creates a new \ref data::Pattern_Ptr, swaps its contents with \ref _pattern
     * and returns it. Used to hand-out the data object to its consumer
     * (a \ref ProcessPattern function) after it has been read.
     */
    data::Pattern_Ptr handoutPattern();

  private:
    /**
     * Callback for processing pattern metadata.
     */
    ProcessPattern _callback;

    /**
     * Pointer to the \ref zypp::data::Pattern object for storing the pattern
     * metada.
     */
    data::Pattern_Ptr _pattern;
  };


    } // ns yum
  } // ns parser
} // ns zypp

#endif /*PATTERNFILEREADER_H_*/
