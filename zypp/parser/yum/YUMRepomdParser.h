/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/YUMRepomdParser.h
 *
*/



#ifndef YUMRepomdParser_h
#define YUMRepomdParser_h

#include <zypp/parser/yum/YUMParserData.h>
#include <zypp/parser/XMLNodeIterator.h>
#include <zypp/parser/LibXMLHelper.h>

namespace zypp
{
namespace parser
{
namespace yum
{

/**
* @short Parser for YUM repomd.xml files (describing the repository)
* Use this class as an iterator that produces, one after one,
* YUMRepomdData_Ptr(s) for the XML package elements.
* Here's an example:
*
* for (YUMRepomdParser iter(anIstream, baseUrl),
*      iter != YUMOtherParser.end(),     // or: iter() != 0, or ! iter.atEnd()
*      ++iter) {
*    doSomething(*iter)
* }
*
* The iterator owns the pointer (i.e., caller must not delete it)
* until the next ++ operator is called. At this time, it will be
* destroyed (and a new ENTRYTYPE is created.)
*
* If the input is fundamentally flawed so that it makes no sense to
* continue parsing, XMLNodeIterator will log it and consider the input as finished.
* You can query the exit status with errorStatus().
*/
class YUMRepomdParser : public XMLNodeIterator<YUMRepomdData_Ptr>
{
public:
  YUMRepomdParser(std::istream &is, const std::string &baseUrl, parser::ParserProgress::Ptr progress = parser::ParserProgress::Ptr() );
  YUMRepomdParser();
  YUMRepomdParser(YUMRepomdData_Ptr& entry);
  virtual ~YUMRepomdParser();

private:
  virtual bool isInterested(const xmlNodePtr nodePtr);
  virtual YUMRepomdData_Ptr process(const xmlTextReaderPtr reader);

  LibXMLHelper _helper;
};
} // namespace yum
} // namespace parser
} // namespace zypp

#endif
