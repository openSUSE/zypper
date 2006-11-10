/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/YUMPatchesParser.cc
 *
*/

#include <zypp/parser/yum/YUMPatchesParser.h>
#include <zypp/parser/yum/YUMPrimaryParser.h>
#include <istream>
#include <string>
#include "zypp/parser/xml_parser_assert.h"
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include <zypp/parser/LibXMLHelper.h>
#include <zypp/base/Logger.h>
#include <zypp/parser/yum/schemanames.h>

using namespace std;
namespace zypp
{
namespace parser
{
namespace yum
{

YUMPatchesParser::~YUMPatchesParser()
{ }

YUMPatchesParser::YUMPatchesParser(std::istream &is, const std::string& baseUrl, parser::ParserProgress::Ptr progress )
    : XMLNodeIterator<YUMPatchesData_Ptr>(is, baseUrl,PATCHESSCHEMA, progress)
{
  fetchNext();
}

YUMPatchesParser::YUMPatchesParser()
{ }

YUMPatchesParser::YUMPatchesParser(YUMPatchesData_Ptr& entry)
    : XMLNodeIterator<YUMPatchesData_Ptr>(entry)
{ }


// select for which elements process() will be called
bool
YUMPatchesParser::isInterested(const xmlNodePtr nodePtr)
{
  return _helper.isElement(nodePtr) && _helper.name(nodePtr) == "patch";
}

// do the actual processing
YUMPatchesData_Ptr
YUMPatchesParser::process(const xmlTextReaderPtr reader)
{
  xml_assert(reader);
  YUMPatchesData_Ptr patchPtr = new YUMPatchesData;
  xmlNodePtr dataNode = xmlTextReaderExpand(reader);
  xml_assert(dataNode);

  patchPtr->id = _helper.attribute(dataNode,"id");

  for (xmlNodePtr child = dataNode->children;
       child && child != dataNode;
       child = child->next)
  {
    if (_helper.isElement(child))
    {
      string name = _helper.name(child);
      if (name == "location")
      {
        patchPtr->location = _helper.attribute(child,"href");
      }
      else if (name == "checksum")
      {
        patchPtr->checksumType = _helper.attribute(child,"type");
        patchPtr->checksum = _helper.content(child);
      }
      else
      {
        WAR << "YUM <data> contains the unknown element <" << name << "> "
        << _helper.positionInfo(child) << ", skipping" << endl;
      }
    }
  }
  return patchPtr;
} /* end process */


} // namespace yum
} // namespace parser
} // namespace zypp
