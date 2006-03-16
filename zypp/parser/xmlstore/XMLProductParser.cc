/*---------------------------------------------------------------------\
|                          ____ _   __ __ ___                          |
|                         |__  / \ / / . \ . \                         |
|                           / / \ V /|  _/  _/                         |
|                          / /__ | | | | | |                           |
|                         /_____||_| |_| |_|                           |
|                                                                      |
\---------------------------------------------------------------------*/
/** \file zypp/parser/yum/XMLProductParser.cc
 *
*/

#include <istream>
#include <string>
#include "zypp/parser/xml_parser_assert.h"
#include <libxml/xmlreader.h>
#include <libxml/tree.h>
#include "zypp/parser/xmlstore/XMLProductParser.h"
#include "zypp/parser/xmlstore/XMLResObjectParser.h"
#include "zypp/parser/LibXMLHelper.h"
#include "zypp/base/Logger.h"
#include "zypp/parser/xmlstore/schemanames.h"

using namespace std;
namespace zypp {
  namespace parser {
    namespace xmlstore {

      XMLProductParser::~XMLProductParser()
      { }
      
      XMLProductParser::XMLProductParser(istream &is, const string& baseUrl)
        : XMLNodeIterator<XMLProductData_Ptr>(is, baseUrl /*,PRODUCTSCHEMA*/)
      {
        fetchNext();
      }
      
      XMLProductParser::XMLProductParser()
      { }
      
      XMLProductParser::XMLProductParser(XMLProductData_Ptr& entry)
      : XMLNodeIterator<XMLProductData_Ptr>(entry)
      { }
      
      
      // select for which elements process() will be called
      bool 
      XMLProductParser::isInterested(const xmlNodePtr nodePtr)
      {
        return _helper.isElement(nodePtr) && _helper.name(nodePtr) == "product";
      }
      
      // do the actual processing
      XMLProductData_Ptr
      XMLProductParser::process(const xmlTextReaderPtr reader)
      {
        xml_assert(reader);
        XMLProductData_Ptr productPtr = new XMLProductData;
        xmlNodePtr dataNode = xmlTextReaderExpand(reader);
        xml_assert(dataNode);
        productPtr->type = _helper.attribute(dataNode,"type");
      
        for (xmlNodePtr child = dataNode->children; child && child != dataNode; child = child->next)
        {
          if (_helper.isElement(child))
          {
            string name = _helper.name(child);
            if (name == "name") {
	      productPtr->name = _helper.content(child);
            }
            else if (name == "vendor") {
	      productPtr->vendor = _helper.content(child);
            }
            else if (name == "release-notes-url") {
              productPtr->releasenotesurl = _helper.content(child);
            }
            else if (name == "displayname") {
              productPtr->displayname.setText(_helper.content(child), Locale(_helper.attribute(child,"lang")));
            }
            else if (name == "description") {
              productPtr->description.setText(_helper.content(child), Locale(_helper.attribute(child,"lang")));
            }
            else if (name == "version") {
              productPtr->epoch = _helper.attribute(child,"epoch");
              productPtr->ver = _helper.attribute(child,"ver");
              productPtr->rel = _helper.attribute(child,"rel");
            }
            else if (name == "provides") {
              parseDependencyEntries(& productPtr->provides, child);
            }
            else if (name == "conflicts") {
              parseDependencyEntries(& productPtr->conflicts, child);
            }
            else if (name == "obsoletes") {
              parseDependencyEntries(& productPtr->obsoletes, child);
            }
            else if (name == "prerequires") {
              parseDependencyEntries(& productPtr->prerequires, child);
            }
            else if (name == "requires") {
              parseDependencyEntries(& productPtr->requires, child);
            }
            else if (name == "recommends") {
              parseDependencyEntries(& productPtr->recommends, child);
            }
            else if (name == "suggests") {
              parseDependencyEntries(& productPtr->suggests, child);
            }
            else if (name == "supplements") {
              parseDependencyEntries(& productPtr->supplements, child);
            }
            else if (name == "enhances") {
              parseDependencyEntries(& productPtr->enhances, child);
            }
            else if (name == "freshens") {
              parseDependencyEntries(& productPtr->freshens, child);
            }
            else {
              WAR << "XML <data> contains the unknown element <" << name << "> "
                << _helper.positionInfo(child) << ", skipping" << endl;
            }
          }
        }
        return productPtr;
      } /* end process */
      
      
      
    } // namespace yum
  } // namespace parser
} // namespace zypp
