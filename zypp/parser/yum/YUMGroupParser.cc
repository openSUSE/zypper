/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/YUMGroupParser.cc
 *
*/

#include <zypp/parser/yum/YUMGroupParser.h>
#include <zypp/parser/LibXMLHelper.h>
#include <istream>
#include <string>
#include "zypp/parser/xml_parser_assert.h"
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include <zypp/base/Logger.h>
#include <zypp/parser/yum/schemanames.h>

using namespace std;
namespace zypp
{
namespace parser
{
namespace yum
{

YUMGroupParser::YUMGroupParser()
{ }

YUMGroupParser::YUMGroupParser(YUMGroupData_Ptr& entry)
    : XMLNodeIterator<YUMGroupData_Ptr>(entry)
{ }


YUMGroupParser::~YUMGroupParser()
{ }


// select for which elements process() will be called
bool
YUMGroupParser::isInterested(const xmlNodePtr nodePtr)
{
  return _helper.isElement(nodePtr) && _helper.name(nodePtr) == "group";
}

// do the actual processing
YUMGroupData_Ptr
YUMGroupParser::process(const xmlTextReaderPtr reader)
{
  xml_assert(reader);
  YUMGroupData_Ptr dataPtr = new YUMGroupData;
  xmlNodePtr dataNode = xmlTextReaderExpand(reader);
  xml_assert(dataNode);

  for (xmlNodePtr child = dataNode->children;
       child && child != dataNode;
       child = child->next)
  {
    if (_helper.isElement(child))
    {
      string name = _helper.name(child);
      if (name == "groupid")
      {
        dataPtr->groupId = _helper.content(child);
      }
      else if (name == "name")
      {
        dataPtr->name.setText(_helper.content(child), Locale(_helper.attribute(child,"lang")));
      }
      else if (name == "default")
      {
        dataPtr->default_ = _helper.content(child);
      }
      else if (name == "uservisible")
      {
        dataPtr->userVisible = _helper.content(child);
      }
      else if (name == "description")
      {
        dataPtr->description.setText(_helper.content(child), Locale(_helper.attribute(child,"lang")));
        ERR << "description has " << dataPtr->description.locales().size() << std::endl;
      }
      else if (name == "grouplist")
      {
        parseGrouplist(dataPtr, child);
      }
      else if (name == "packagelist")
      {
        parsePackageList(dataPtr, child);
      }
      else
      {
        WAR << "YUM <group> contains the unknown element <" << name << "> "
        << _helper.positionInfo(child) << ", skipping" << endl;
      }
    }
  }
  return dataPtr;
} /* end process */

void YUMGroupParser::parseGrouplist(YUMGroupData_Ptr dataPtr,
                                    xmlNodePtr node)
{
  xml_assert(dataPtr);
  xml_assert(node);

  for (xmlNodePtr child = node->children;
       child != 0;
       child = child ->next)
  {
    if (_helper.isElement(child))
    {
      string name = _helper.name(child);
      if (name == "metapkg" || name == "groupreq")
      {
        dataPtr->grouplist.push_back
        (MetaPkg(_helper.attribute(child,"type"),
                 _helper.content(child)));
      }
      else
      {
        WAR << "YUM <grouplist> contains the unknown element <" << name << "> "
        << _helper.positionInfo(child) << ", skipping" << endl;
      }
    }
  }
}


void YUMGroupParser::parsePackageList(YUMGroupData_Ptr dataPtr,
                                      xmlNodePtr node)
{
  xml_assert(dataPtr);
  xml_assert(node);

  for (xmlNodePtr child = node->children;
       child != 0;
       child = child ->next)
  {
    if (_helper.isElement(child))
    {
      string name = _helper.name(child);
      if (name == "packagereq")
      {
        dataPtr->packageList.push_back
        (PackageReq(_helper.attribute(child,"type"),
                    _helper.attribute(child,"epoch"),
                    _helper.attribute(child,"ver"),
                    _helper.attribute(child,"rel"),
                    _helper.content(child)));
      }
      else
      {
        WAR << "YUM <packagelist> contains the unknown element <" << name << "> "
        << _helper.positionInfo(child) << ", skipping" << endl;
      }
    }
  }
}



YUMGroupParser::YUMGroupParser(istream &is, const string &baseUrl, parser::ParserProgress::Ptr progress )
    : XMLNodeIterator<YUMGroupData_Ptr>(is, baseUrl,GROUPSCHEMA, progress)
{
  fetchNext();
}

} // namespace yum
} // namespace parser
} // namespace zypp
