/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/xmlstore/XMLResObjectParser.cc
 *
*/

#include <zypp/parser/xmlstore/XMLResObjectParser.h>
#include <istream>
#include <string>
#include "zypp/parser/xml_parser_assert.h"
#include <libxml/xmlstring.h>
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
namespace xmlstore
{
      

XMLResObjectParser::XMLResObjectParser()
{ }

XMLResObjectParser::~XMLResObjectParser()
{
}


void
XMLResObjectParser::parseResObjectCommonData( XMLResObjectData_Ptr dataPtr, xmlNodePtr node)
{
  xml_assert(node);

  for (xmlNodePtr child = node->children; child != 0; child = child ->next)
  {
    if (_helper.isElement(child))
    {
      string name = _helper.name(child);

      if (name == "name") {
        dataPtr->name = _helper.content(child);
      }
      else if (name == "arch") {
        dataPtr->arch = _helper.content(child);
      }
      else if (name == "version") {
        dataPtr->epoch = _helper.attribute(child,"epoch");
        dataPtr->ver = _helper.attribute(child,"ver");
        dataPtr->rel = _helper.attribute(child,"rel");
      }
    }
  }
} 

void
    XMLResObjectParser::parseDependencies( XMLResObjectData_Ptr dataPtr, xmlNodePtr node)
{
  xml_assert(node);

  for (xmlNodePtr child = node->children; child != 0; child = child ->next)
  {
    if (_helper.isElement(child))
    {
      string name = _helper.name(child);
      if (name == "provides") {
        parseDependencyEntries(& dataPtr->provides, child);
      }
      else if (name == "conflicts") {
        parseDependencyEntries(& dataPtr->conflicts, child);
      }
      else if (name == "obsoletes") {
        parseDependencyEntries(& dataPtr->obsoletes, child);
      }
      else if (name == "prerequires") {
        parseDependencyEntries(& dataPtr->prerequires, child);
      }
      else if (name == "requires") {
        parseDependencyEntries(& dataPtr->requires, child);
      }
      else if (name == "recommends") {
        parseDependencyEntries(& dataPtr->recommends, child);
      }
      else if (name == "suggests") {
        parseDependencyEntries(& dataPtr->suggests, child);
      }
      else if (name == "supplements") {
        parseDependencyEntries(& dataPtr->supplements, child);
      }
      else if (name == "enhances") {
        parseDependencyEntries(& dataPtr->enhances, child);
      }
      else if (name == "freshens") {
        parseDependencyEntries(& dataPtr->freshens, child);
      }
    }
  } 
  
}

void
XMLResObjectParser::parseDependencyEntries(list<XMLDependency> *depList,
                                              xmlNodePtr depNode)
{
  xml_assert(depNode);

  for (xmlNodePtr child = depNode->children; child != 0; child = child ->next)
  {
    if (_helper.isElement(child))
    {
      string name = _helper.name(child);

      if ( name == "capability" )
      {
        depList->push_back
            (XMLDependency(_helper.attribute(child,"kind"),
            _helper.content(child)));
      }
      else
      {
        WAR << "XML dependency contains the unknown element <" << name << "> "
          << _helper.positionInfo(child) << ", skipping" << endl;
      }
    }
  }
}
      
} // namespace xmlstore
} // namespace parser
} // namespace zypp
