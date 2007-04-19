#ifndef ZYPP_PARSER_YUM_PRIMARYFILEPARSER_H
#define ZYPP_PARSER_YUM_PRIMARYFILEPARSER_H

#include "zypp/Date.h"
#include "zypp/base/Function.h"
#include "zypp/base/Logger.h"
#include "zypp/parser/xml/Reader.h"
#include "zypp/data/ResolvableData.h"

#undef ZYPP_BASE_LOGGER_LOGGROUP
#define ZYPP_BASE_LOGGER_LOGGROUP "parser"

namespace zypp
{
  namespace parser
  {
    namespace yum
    {

      enum Tag
      {
        tag_NONE,
        tag_package,
        tag_format
      };

/**
 * Iterates through a primary.xml file giving on each iteration
 * a \ref OnMediaLocation object with the resource and its
 * type ( primary, patches, filelists, etc ).
 * The iteration is done via a callback provided on
 * construction.
 *
 * \code
 * RepomdFileReader reader(repomd_file, 
 *                  bind( &SomeClass::callbackfunc, &object, _1, _2 ) );
 * \endcode
 */
class PrimaryFileReader
{
public:
  /**
   * Callback definition.
   */
  typedef function<bool(const zypp::data::Package&)> ProcessPackage;

  /**
   * Constructor
   * \param primary_file the primary.xml.gz file you want to read
   * \param function to process \ref _package data.
   * 
   * \see PrimaryFileReader::ProcessPackage
   */
  PrimaryFileReader(const Pathname &primary_file, ProcessPackage callback);

  /**
   * Callback provided to the XML parser.
   */
  bool consumeNode(zypp::xml::Reader & reader_r);

private:
  bool consumeFormatChildNodes(zypp::xml::Reader & reader_r);

private:
  Tag _tag;
  unsigned _count;
  zypp::data::Package *_package;
  ProcessPackage _callback;
/*  CheckSum _checksum;
  std::string _checksum_type;
  Date _timestamp;*/
};


    } // ns zypp
  } // ns parser
} // ns yum

#endif /* ZYPP_PARSER_YUM_PRIMARYFILEPARSER_H */

// vim: set ts=2 sts=2 sw=2 et ai:
