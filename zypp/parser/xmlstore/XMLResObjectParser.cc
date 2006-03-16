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

void parseResObjectEntries( XMLResObjectData *data,  xmlNodePtr depNode)
{
  xml_assert(data);
  xml_assert(depNode); 
  
  
}

void
XMLResObjectParser::parseDependencyEntries(list<XMLDependency> *depList,
                                              xmlNodePtr depNode)
{
  xml_assert(depList);
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
        WAR << "XML dependency within <format> contains the unknown element <" << name << "> "
          << _helper.positionInfo(child) << ", skipping" << endl;
      }
    }
  }
}
      
} // namespace xmlstore
} // namespace parser
} // namespace zypp
